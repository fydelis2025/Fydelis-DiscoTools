#include "usbcreator.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

UsbCreator::UsbCreator(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_mgr, &DiskManager::operationFinished, this, &UsbCreator::onOperationFinished);
    connect(m_mgr, &DiskManager::outputReceived, this, &UsbCreator::onOutputReceived);
    QTimer::singleShot(500, this, &UsbCreator::refreshDisks);
}

void UsbCreator::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);
    QLabel *hdr = new QLabel("💿  USB Bootável");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);
    QLabel *desc = new QLabel("Grave ISO em USB.");
    desc->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    ml->addWidget(desc);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    QHBoxLayout *ir = new QHBoxLayout();
    m_isoEd = new QLineEdit(); m_isoEd->setPlaceholderText("Arquivo ISO...");
    m_isoEd->setStyleSheet("QLineEdit{background:#1a1a2e;color:#e0e0e0;border:1px solid #2a2a3f;border-radius:6px;padding:8px;}");
    QPushButton *isoBtn = new QPushButton("📁"); isoBtn->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(isoBtn, &QPushButton::clicked, this, &UsbCreator::onBrowseISO);
    ir->addWidget(new QLabel("ISO:")); ir->addWidget(m_isoEd,1); ir->addWidget(isoBtn);
    cl->addLayout(ir);

    QHBoxLayout *dr = new QHBoxLayout();
    m_devCb = new QComboBox(); m_devCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &UsbCreator::refreshDisks);
    dr->addWidget(new QLabel("USB:")); dr->addWidget(m_devCb,1); dr->addWidget(ref);
    cl->addLayout(dr);
    ml->addWidget(card);

    QLabel *aviso = new QLabel("⚠ O dispositivo será APAGADO completamente.");
    aviso->setStyleSheet(QString("color:%1;").arg(FydelisTheme::WARNING.name()));
    ml->addWidget(aviso);

    m_goBtn = new QPushButton("💿 GRAVAR"); m_goBtn->setStyleSheet(FydelisTheme::dangerButtonStyle());
    m_goBtn->setMinimumHeight(48);
    connect(m_goBtn, &QPushButton::clicked, this, &UsbCreator::onCreateUSB);
    ml->addWidget(m_goBtn);

    m_prog = new QProgressBar(); m_prog->setRange(0,0);
    m_prog->setStyleSheet(FydelisTheme::progressBarStyle()); m_prog->setVisible(false);
    ml->addWidget(m_prog);
    ml->addWidget(new QLabel("Log:"));
    m_log = new QTextEdit(); m_log->setReadOnly(true);
    m_log->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_log,1);
}

void UsbCreator::refreshDisks()
{
    m_disks = m_mgr->listDisks();
    m_devCb->clear();
    for (const auto &d : m_disks)
        m_devCb->addItem(d.name + " (" + d.size + ") " + d.model);
}

void UsbCreator::onBrowseISO()
{
    QString path = QFileDialog::getOpenFileName(this,"ISO","/home","*.iso *.ISO");
    if (!path.isEmpty()) m_isoEd->setText(path);
}

void UsbCreator::onCreateUSB()
{
    QString iso = m_isoEd->text().trimmed();
    if (iso.isEmpty()||!QFileInfo::exists(iso)) { QMessageBox::warning(this,"","ISO inválido."); return; }
    int idx = m_devCb->currentIndex();
    if (idx<0||idx>=m_disks.size()) { QMessageBox::warning(this,"","Selecione o USB."); return; }
    if (QMessageBox::critical(this,"ATENÇÃO",QString("GRAVAR ISO em %1?").arg(m_disks[idx].name),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    m_mgr->createBootableUSB(iso, m_disks[idx].name);
}

void UsbCreator::onOperationFinished(bool ok, const QString &msg)
{
    m_prog->setVisible(false);
    m_log->append((ok?"✅ ":"❌ ")+msg);
}

void UsbCreator::onOutputReceived(const QString &line)
{
    m_log->append(line.trimmed());
}
