#include "diskmanager.h"
#include <QFileInfo>
#include <unistd.h>

DiskManager::DiskManager(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DiskManager::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &DiskManager::onProcessOutput);
    connect(m_process, &QProcess::errorOccurred,
            this, &DiskManager::onProcessError);
}

QList<DiskDeviceInfo> DiskManager::listDisks()
{
    QList<DiskDeviceInfo> disks;
    QProcess p;
    p.start("lsblk", {"-J","-o","NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL,ROTA,TRAN"});
    p.waitForFinished(10000);
    QByteArray data = p.readAllStandardOutput();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return disks;

    QJsonArray devices = doc.object()["blockdevices"].toArray();
    for (const QJsonValue &val : devices) {
        QJsonObject obj = val.toObject();
        DiskDeviceInfo d;
        d.name = obj["name"].toString();
        d.size = obj["size"].toString();
        d.type = obj["type"].toString();
        d.model = obj["model"].toString();
        d.transport = obj["tran"].toString();
        if (d.type != "disk") continue;

        QJsonArray children = obj["children"].toArray();
        for (const QJsonValue &cv : children) {
            QJsonObject co = cv.toObject();
            PartitionInfo p;
            p.name = co["name"].toString();
            p.size = co["size"].toString();
            p.type = co["type"].toString();
            p.mountPoint = co["mountpoint"].toString();
            p.fsType = co["fstype"].toString();
            p.uuid = co["uuid"].toString();
            d.partitions.append(p);
            if (p.mountPoint == "/") d.isSystem = true;
        }
        disks.append(d);
    }
    emit disksUpdated(disks);
    return disks;
}

DiskDeviceInfo DiskManager::getDiskInfo(const QString &device)
{
    QString dev = device.startsWith("/dev/") ? device.mid(5) : device;
    for (const auto &d : listDisks())
        if (d.name == dev) return d;
    return DiskDeviceInfo();
}

QList<PartitionInfo> DiskManager::getPartitions(const QString &disk)
{
    return getDiskInfo(disk).partitions;
}

void DiskManager::formatPartition(const QString &device, const QString &fsType,
                                   const QString &label, bool quick)
{
    QString devPath = device.startsWith("/dev/") ? device : "/dev/" + device;
    QProcess u; u.start("umount", {devPath}); u.waitForFinished(3000);

    QString prog;
    QStringList args;
    if (fsType.startsWith("ext")) {
        prog = "mkfs." + fsType; args << "-F";
        if (!label.isEmpty()) args << "-L" << label;
        args << devPath;
    } else if (fsType == "btrfs") {
        prog = "mkfs.btrfs"; args << "-f";
        if (!label.isEmpty()) args << "-L" << label;
        args << devPath;
    } else if (fsType == "xfs") {
        prog = "mkfs.xfs"; args << "-f";
        if (!label.isEmpty()) args << "-L" << label;
        args << devPath;
    } else if (fsType == "ntfs") {
        prog = "mkfs.ntfs"; args << "-Q";
        if (!label.isEmpty()) args << "-L" << label;
        if (!quick) args << "-f";
        args << devPath;
    } else if (fsType == "fat32" || fsType == "vfat") {
        prog = "mkfs.vfat"; args << "-F32";
        if (!label.isEmpty()) args << "-n" << label;
        args << devPath;
    } else {
        emit errorOccurred("Sistema de arquivos não suportado: " + fsType);
        return;
    }
    emit operationStarted("Formatando " + devPath + " como " + fsType + "...");
    runCommand(prog, args);
}

void DiskManager::wipeDisk(const QString &device)
{
    QString devPath = device.startsWith("/dev/") ? device : "/dev/" + device;
    emit operationStarted("Limpando " + devPath + "...");
    runCommand("wipefs", {"-a", devPath});
}

void DiskManager::cloneDisk(const QString &source, const QString &target)
{
    QString src = source.startsWith("/dev/") ? source : "/dev/" + source;
    QString tgt = target.startsWith("/dev/") ? target : "/dev/" + target;
    emit operationStarted("Clonando " + src + " → " + tgt + "...");
    runCommand("dd", {"if="+src, "of="+tgt, "bs=4M", "status=progress"});
}

void DiskManager::createBootableUSB(const QString &isoPath, const QString &device)
{
    QString dev = device.startsWith("/dev/") ? device : "/dev/" + device;
    if (!QFileInfo::exists(isoPath)) {
        emit errorOccurred("ISO não encontrado: " + isoPath);
        return;
    }
    emit operationStarted("Criando USB bootável: " + isoPath + " → " + dev);
    runCommand("dd", {"if="+isoPath, "of="+dev, "bs=4M", "status=progress", "oflag=sync"});
}

void DiskManager::recoverPartitionTable(const QString &device)
{
    QString dev = device.startsWith("/dev/") ? device : "/dev/" + device;
    emit operationStarted("Recuperando tabela de partições em " + dev + "...");
    runCommand("testdisk", {"/cmd", dev, "analyse,write"});
}

void DiskManager::recoverDeletedFiles(const QString &device, const QString &outputDir,
                                       const QString &type)
{
    QString dev = device.startsWith("/dev/") ? device : "/dev/" + device;
    QDir().mkpath(outputDir);
    emit operationStarted("Recuperando arquivos de " + dev + " para " + outputDir + "...");
    runCommand("foremost", {"-i", dev, "-o", outputDir, "-t", type});
}

void DiskManager::cancelCurrentOperation()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        emit operationFinished(false, "Operação cancelada pelo usuário.");
    }
}

void DiskManager::runCommand(const QString &program, const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning) {
        emit errorOccurred("Já existe uma operação em andamento.");
        return;
    }
    m_process->start(program, args);
}

void DiskManager::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    bool success = (exitCode == 0 && status == QProcess::NormalExit);
    QString msg = success ? "Operação concluída com sucesso!"
                          : "Falha (código " + QString::number(exitCode) + ")";
    emit operationFinished(success, msg);
    emit progressChanged(100);
}

void DiskManager::onProcessOutput()
{
    QString out = QString::fromUtf8(m_process->readAllStandardOutput());
    emit outputReceived(out);
}

void DiskManager::onProcessError(QProcess::ProcessError err)
{
    QString msg;
    switch (err) {
        case QProcess::FailedToStart: msg = "Falha ao iniciar. Programa não encontrado?"; break;
        case QProcess::Crashed: msg = "Processo encerrado inesperadamente."; break;
        default: msg = "Erro no processo.";
    }
    emit errorOccurred(msg);
}

QString DiskManager::humanReadableSize(qint64 bytes)
{
    const char *u[] = {"B","KB","MB","GB","TB"};
    int i = 0; double s = bytes;
    while (s >= 1024 && i < 4) { s /= 1024; i++; }
    return QString::number(s,'f',2) + " " + u[i];
}

bool DiskManager::isRoot() { return getuid() == 0; }
