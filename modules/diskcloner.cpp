#include "diskcloner.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>

DiskCloner::DiskCloner(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_mgr, &DiskManager::operationFinished, this, &DiskCloner::onOperationFinished);
    connect(m_mgr, &DiskManager::outputReceived, this, &DiskCloner::onOutputReceived);
    QTimer::singleShot(500, this, &DiskCloner::refreshDisks);
}

void DiskCloner::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);
    QLabel *hdr = new QLabel("🔄  Clonar Disco");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);
    QLabel *desc = new QLabel("Clone bit-a-bit com dd.");
    desc->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    ml->addWidget(desc);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->addWidget(new QLabel("Origem:"));
    m_srcCb = new QComboBox(); m_srcCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    cl->addWidget(m_srcCb);

    QHBoxLayout *mr = new QHBoxLayout();
    m_swapBtn = new QPushButton("⇅ INVERTER"); m_swapBtn->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(m_swapBtn, &QPushButton::clicked, this, &DiskCloner::onSwapSourceTarget);
    mr->addStretch(); mr->addWidget(m_swapBtn); mr->addStretch();
    cl->addLayout(mr);

    cl->addWidget(new QLabel("Destino:"));
    m_tgtCb = new QComboBox(); m_tgtCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    cl->addWidget(m_tgtCb);
    ml->addWidget(card);

    QHBoxLayout *ab = new QHBoxLayout();
    m_goBtn = new QPushButton("⚠ CLONAR"); m_goBtn->setStyleSheet(FydelisTheme::dangerButtonStyle());
    m_goBtn->setMinimumHeight(48);
    connect(m_goBtn, &QPushButton::clicked, this, &DiskCloner::onClone);
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &DiskCloner::refreshDisks);
    ab->addWidget(m_goBtn); ab->addWidget(ref);
    ml->addLayout(ab);

    m_prog = new QProgressBar(); m_prog->setRange(0,100);
    m_prog->setStyleSheet(FydelisTheme::progressBarStyle()); m_prog->setVisible(false);
    ml->addWidget(m_prog);
    ml->addWidget(new QLabel("Log:"));
    m_log = new QTextEdit(); m_log->setReadOnly(true);
    m_log->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_log,1);
}

void DiskCloner::refreshDisks()
{
    m_disks = m_mgr->listDisks();
    m_srcCb->clear(); m_tgtCb->clear();
    for (const auto &d : m_disks) {
        QString s = d.name + " (" + d.size + ")" + (d.isSystem?" 🖥":"");
        m_srcCb->addItem(s); m_tgtCb->addItem(s);
    }
}

void DiskCloner::onSwapSourceTarget()
{
    int si = m_srcCb->currentIndex(), ti = m_tgtCb->currentIndex();
    m_srcCb->setCurrentIndex(ti); m_tgtCb->setCurrentIndex(si);
}

void DiskCloner::onClone()
{
    int si = m_srcCb->currentIndex(), ti = m_tgtCb->currentIndex();
    if (si<0||ti<0||si==ti) { QMessageBox::warning(this,"","Selecione origem e destino diferentes."); return; }
    if (QMessageBox::critical(this,"ATENÇÃO",QString("CLONAR %1 → %2?").arg(m_disks[si].name,m_disks[ti].name),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    m_mgr->cloneDisk(m_disks[si].name, m_disks[ti].name);
}

void DiskCloner::onOperationFinished(bool ok, const QString &msg)
{
    m_prog->setVisible(false);
    m_log->append((ok?"✅ ":"❌ ") + msg);
}

void DiskCloner::onOutputReceived(const QString &line)
{
    m_log->append(line.trimmed());
}
