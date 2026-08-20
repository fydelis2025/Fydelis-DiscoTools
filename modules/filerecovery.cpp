#include "filerecovery.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

FileRecovery::FileRecovery(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_mgr, &DiskManager::operationFinished, this, &FileRecovery::onOperationFinished);
    connect(m_mgr, &DiskManager::outputReceived, this, &FileRecovery::onOutputReceived);
    QTimer::singleShot(500, this, &FileRecovery::refreshDevices);
}

void FileRecovery::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);
    QLabel *hdr = new QLabel("📂  Recuperar Arquivos");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);
    QLabel *desc = new QLabel("Recupere arquivos deletados com foremost.");
    desc->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    ml->addWidget(desc);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    QHBoxLayout *dr = new QHBoxLayout();
    m_devCb = new QComboBox(); m_devCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &FileRecovery::refreshDevices);
    dr->addWidget(new QLabel("Dispositivo:")); dr->addWidget(m_devCb,1); dr->addWidget(ref);
    cl->addLayout(dr);

    // Variável renomeada de 'or' para 'outputLayout' para evitar conflito com a palavra reservada do C++
    QHBoxLayout *outputLayout = new QHBoxLayout();
    m_outEd = new QLineEdit(); m_outEd->setPlaceholderText("/home/user/recovered");
    m_outEd->setStyleSheet("QLineEdit{background:#1a1a2e;color:#e0e0e0;border:1px solid #2a2a3f;border-radius:6px;padding:8px;}");
    QPushButton *browse = new QPushButton("📁"); browse->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(browse, &QPushButton::clicked, this, &FileRecovery::onBrowse);
    outputLayout->addWidget(new QLabel("Destino:")); 
    outputLayout->addWidget(m_outEd, 1); 
    outputLayout->addWidget(browse);
    cl->addLayout(outputLayout);

    m_typeCb = new QComboBox();
    m_typeCb->addItems({"all","jpg,gif,png","doc,pdf,xls","zip,rar,7z","mp3,mp4,avi"});
    m_typeCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    cl->addWidget(new QLabel("Tipo:")); cl->addWidget(m_typeCb);
    ml->addWidget(card);

    m_goBtn = new QPushButton("🚀 RECUPERAR"); m_goBtn->setStyleSheet(FydelisTheme::dangerButtonStyle());
    m_goBtn->setMinimumHeight(48);
    connect(m_goBtn, &QPushButton::clicked, this, &FileRecovery::onRecover);
    ml->addWidget(m_goBtn);
    m_prog = new QProgressBar(); m_prog->setRange(0,0);
    m_prog->setStyleSheet(FydelisTheme::progressBarStyle()); m_prog->setVisible(false);
    ml->addWidget(m_prog);
    ml->addWidget(new QLabel("Log:"));
    m_log = new QTextEdit(); m_log->setReadOnly(true);
    m_log->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_log,1);
}

void FileRecovery::refreshDevices()
{
    m_disks = m_mgr->listDisks();
    m_devCb->clear();
    for (const auto &d : m_disks) {
        m_devCb->addItem(d.name + " (" + d.size + ")");
        for (const auto &p : d.partitions)
            m_devCb->addItem("  └ " + p.name + " [" + p.fsType + "]");
    }
}

void FileRecovery::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Pasta");
    if (!dir.isEmpty()) m_outEd->setText(dir);
}

void FileRecovery::onRecover()
{
    if (m_devCb->currentText().isEmpty()) { QMessageBox::warning(this,"","Selecione dispositivo."); return; }
    if (m_outEd->text().trimmed().isEmpty()) { QMessageBox::warning(this,"","Defina pasta destino."); return; }
    QString dev = m_devCb->currentText().section(" ",0,0).trimmed();
    if (dev.startsWith("└")) dev = dev.mid(2).trimmed();
    m_prog->setVisible(true);
    m_mgr->recoverDeletedFiles(dev, m_outEd->text().trimmed(), m_typeCb->currentText()=="all"?"all":m_typeCb->currentText());
}

void FileRecovery::onOperationFinished(bool ok, const QString &msg)
{
    m_prog->setVisible(false);
    m_log->append((ok?"✅ ":"❌ ") + msg);
}

void FileRecovery::onOutputReceived(const QString &line)
{
    m_log->append(line.trimmed());
}
