#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <QObject>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QDebug>
#include <QDir>

struct PartitionInfo {
    QString name;
    QString size;
    QString type;
    QString mountPoint;
    QString fsType;
    QString uuid;
};

struct DiskDeviceInfo {
    QString name;
    QString size;
    QString type;
    QString model;
    QString transport;
    QList<PartitionInfo> partitions;
    bool isSystem = false;
};

class DiskManager : public QObject
{
    Q_OBJECT

public:
    explicit DiskManager(QObject *parent = nullptr);

    // ─── Discovery ──────────────────────────────────────────
    QList<DiskDeviceInfo> listDisks();
    DiskDeviceInfo getDiskInfo(const QString &device);
    QList<PartitionInfo> getPartitions(const QString &disk);

    // ─── Operations ─────────────────────────────────────────
    void formatPartition(const QString &device, const QString &fsType,
                         const QString &label = "", bool quick = true);
    void wipeDisk(const QString &device);
    void cloneDisk(const QString &source, const QString &target);
    void createBootableUSB(const QString &isoPath, const QString &device);
    void recoverPartitionTable(const QString &device);
    void recoverDeletedFiles(const QString &device, const QString &outputDir,
                             const QString &type = "all");
    void cancelCurrentOperation();

    // ─── Utilities ──────────────────────────────────────────
    static QString humanReadableSize(qint64 bytes);
    static bool isRoot();

signals:
    void operationStarted(const QString &message);
    void progressChanged(int percent);
    void operationFinished(bool success, const QString &message);
    void outputReceived(const QString &line);
    void errorOccurred(const QString &error);
    void disksUpdated(const QList<DiskDeviceInfo> &disks);   // ← CORRIGIDO

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessOutput();
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process;
    void runCommand(const QString &program, const QStringList &args);
};

#endif
