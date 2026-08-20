#include "partitionrecovery.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>
#include <QProcess>

PartitionRecovery::PartitionRecovery(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_mgr, &DiskManager::operationFinished, this, &PartitionRecovery::onOperationFinished);
    connect(m_mgr, &DiskManager::outputReceived, this, &PartitionRecovery::onOutputReceived);
    QTimer::singleShot(500, this, &PartitionRecovery::refreshDisks);
}

void PartitionRecovery::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);
    QLabel *hdr = new QLabel("🔧  Recuperar Partição");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);
    QLabel *desc = new QLabel("Recupere tabelas de partições perdidas.");
    desc->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    ml->addWidget(desc);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->addWidget(new QLabel("Disco:"));
    QHBoxLayout *dr = new QHBoxLayout();
    m_cb = new QComboBox(); m_cb->setMinimumHeight(40); m_cb->setStyleSheet(FydelisTheme::comboBoxStyle());
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &PartitionRecovery::refreshDisks);
    dr->addWidget(m_cb,1); dr->addWidget(ref);
    cl->addLayout(dr);
    m_status = new QLabel("Pronto"); m_status->setStyleSheet("color:#a0a0b0;");
    cl->addWidget(m_status);
    ml->addWidget(card);

    QHBoxLayout *ab = new QHBoxLayout();
    m_scanBtn = new QPushButton("🔍 ESCANEAR"); m_scanBtn->setStyleSheet(FydelisTheme::primaryButtonStyle());
    m_scanBtn->setMinimumHeight(48);
    connect(m_scanBtn, &QPushButton::clicked, this, &PartitionRecovery::onQuickScan);
    m_restoreBtn = new QPushButton("♻ RESTAURAR"); m_restoreBtn->setStyleSheet(FydelisTheme::dangerButtonStyle());
    m_restoreBtn->setMinimumHeight(48);
    connect(m_restoreBtn, &QPushButton::clicked, this, &PartitionRecovery::onRestore);
    ab->addWidget(m_scanBtn); ab->addWidget(m_restoreBtn);
    ml->addLayout(ab);

    m_prog = new QProgressBar(); m_prog->setRange(0,0);
    m_prog->setStyleSheet(FydelisTheme::progressBarStyle()); m_prog->setVisible(false);
    ml->addWidget(m_prog);
    ml->addWidget(new QLabel("Log:"));
    m_log = new QTextEdit(); m_log->setReadOnly(true);
    m_log->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_log,1);
}

void PartitionRecovery::refreshDisks()
{
    m_disks = m_mgr->listDisks();
    m_cb->clear();
    for (const auto &d : m_disks) m_cb->addItem(d.name + " (" + d.size + ")");
    m_log->append("✅ " + QString::number(m_disks.size()) + " disco(s).");
}

void PartitionRecovery::onQuickScan()
{
    int idx = m_cb->currentIndex();
    if (idx<0 || idx>=m_disks.size()) { QMessageBox::warning(this,"","Selecione um disco."); return; }
    QString dev = "/dev/" + m_disks[idx].name;
    m_log->append("🔍 Escaneando " + dev + "...");
    m_prog->setVisible(true);
    QProcess *p = new QProcess(this);
    connect(p, &QProcess::finished, this, [this,p](int){
        m_log->append(QString::fromUtf8(p->readAllStandardOutput()));
        m_prog->setVisible(false); p->deleteLater();
    });
    p->start("gdisk", {"-l", dev});
}

void PartitionRecovery::onRestore()
{
    int idx = m_cb->currentIndex();
    if (idx<0 || idx>=m_disks.size()) return;
    QString dev = "/dev/" + m_disks[idx].name;
    if (QMessageBox::warning(this,"Restaurar",QString("Sobrescrever tabela de %1?").arg(dev),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    m_mgr->recoverPartitionTable(dev);
}

void PartitionRecovery::onOperationFinished(bool ok, const QString &msg)
{
    m_prog->setVisible(false);
    m_log->append((ok?"✅ ":"❌ ")+msg);
}

void PartitionRecovery::onOutputReceived(const QString &line)
{
    m_log->append(line.trimmed());
}
