#include "diskformatter.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QInputDialog>

DiskFormatter::DiskFormatter(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_mgr, &DiskManager::operationFinished, this, &DiskFormatter::onOperationFinished);
    connect(m_mgr, &DiskManager::outputReceived, this, &DiskFormatter::onOutputReceived);
    connect(m_diskCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DiskFormatter::onDiskSelected);
    QTimer::singleShot(500, this, &DiskFormatter::refreshDisks);
}

void DiskFormatter::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);

    QLabel *hdr = new QLabel("💾  Formatar Disco");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);
    QLabel *desc = new QLabel("Formate partições. Operação destrutiva.");
    desc->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    ml->addWidget(desc);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->addWidget(new QLabel("Disco:"));
    QHBoxLayout *dr = new QHBoxLayout();
    m_diskCb = new QComboBox(); m_diskCb->setMinimumHeight(40); m_diskCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &DiskFormatter::refreshDisks);
    dr->addWidget(m_diskCb,1); dr->addWidget(ref);
    cl->addLayout(dr);
    m_partTbl = new QTableWidget(0,5);
    m_partTbl->setHorizontalHeaderLabels({"Partição","Tamanho","FS","Montagem","UUID"});
    m_partTbl->setStyleSheet("QTableWidget{background:#1a1a2e;color:#e0e0e0;border:1px solid #2a2a3f;border-radius:6px;}"
                             "QHeaderView::section{background:#0f3460;color:#e0e0e0;padding:8px;font-weight:bold;}");
    m_partTbl->horizontalHeader()->setStretchLastSection(true);
    m_partTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_partTbl->setMaximumHeight(150);
    cl->addWidget(m_partTbl);
    ml->addWidget(card);

    QFrame *oc = new QFrame(); oc->setObjectName("Card"); oc->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *ol = new QVBoxLayout(oc);
    ol->addWidget(new QLabel("Opções:"));
    QHBoxLayout *r1 = new QHBoxLayout();
    m_fsCb = new QComboBox(); m_fsCb->addItems({"ext4","ext3","ext2","btrfs","xfs","ntfs","fat32","exfat"});
    m_fsCb->setCurrentText("ext4"); m_fsCb->setStyleSheet(FydelisTheme::comboBoxStyle());
    m_labelEd = new QLineEdit(); m_labelEd->setPlaceholderText("Rótulo");
    m_labelEd->setStyleSheet("QLineEdit{background:#1a1a2e;color:#e0e0e0;border:1px solid #2a2a3f;border-radius:6px;padding:8px;}");
    r1->addWidget(new QLabel("FS:")); r1->addWidget(m_fsCb);
    r1->addWidget(new QLabel("Rótulo:")); r1->addWidget(m_labelEd,1);
    ol->addLayout(r1);
    m_quickCk = new QCheckBox("Rápida"); m_quickCk->setChecked(true);
    m_quickCk->setStyleSheet("color:#e0e0e0;");
    ol->addWidget(m_quickCk);
    ml->addWidget(oc);

    QHBoxLayout *ab = new QHBoxLayout();
    m_fmtBtn = new QPushButton("💾 FORMATAR"); m_fmtBtn->setStyleSheet(FydelisTheme::dangerButtonStyle());
    m_fmtBtn->setMinimumHeight(48);
    connect(m_fmtBtn, &QPushButton::clicked, this, &DiskFormatter::onFormatClicked);
    m_wipeBtn = new QPushButton("⚠ LIMPAR"); m_wipeBtn->setStyleSheet(FydelisTheme::primaryButtonStyle());
    m_wipeBtn->setMinimumHeight(48);
    connect(m_wipeBtn, &QPushButton::clicked, this, &DiskFormatter::onWipeClicked);
    ab->addWidget(m_fmtBtn); ab->addWidget(m_wipeBtn);
    ml->addLayout(ab);

    m_prog = new QProgressBar(); m_prog->setRange(0,100);
    m_prog->setStyleSheet(FydelisTheme::progressBarStyle()); m_prog->setVisible(false);
    ml->addWidget(m_prog);
    ml->addWidget(new QLabel("Log:"));
    m_log = new QTextEdit(); m_log->setReadOnly(true); m_log->setMaximumHeight(200);
    m_log->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_log,1);
}

void DiskFormatter::refreshDisks()
{
    m_disks = m_mgr->listDisks();
    m_diskCb->clear();
    for (const auto &d : m_disks)
        m_diskCb->addItem(QString("%1 %2 [%3] (%4)").arg(d.isSystem?"🖥":"💽",d.name,d.transport,d.size));
    m_log->append("✅ Discos: " + QString::number(m_disks.size()));
}

void DiskFormatter::onDiskSelected(int idx)
{
    if (idx<0 || idx>=m_disks.size()) return;
    const DiskDeviceInfo &d = m_disks[idx];
    m_partTbl->setRowCount(0);
    for (const auto &p : d.partitions) {
        int r = m_partTbl->rowCount();
        m_partTbl->insertRow(r);
        m_partTbl->setItem(r,0,new QTableWidgetItem(p.name));
        m_partTbl->setItem(r,1,new QTableWidgetItem(p.size));
        m_partTbl->setItem(r,2,new QTableWidgetItem(p.fsType.isEmpty()?"?":p.fsType));
        m_partTbl->setItem(r,3,new QTableWidgetItem(p.mountPoint.isEmpty()?"não montado":p.mountPoint));
        m_partTbl->setItem(r,4,new QTableWidgetItem(p.uuid.isEmpty()?"-":p.uuid.left(8)+"..."));
    }
}

void DiskFormatter::onFormatClicked()
{
    int idx = m_diskCb->currentIndex();
    if (idx<0 || idx>=m_disks.size()) { QMessageBox::warning(this,"","Selecione um disco."); return; }
    QStringList parts;
    for (const auto &p : m_disks[idx].partitions) parts << p.name;
    if (parts.isEmpty()) { QMessageBox::warning(this,"","Sem partições."); return; }
    bool ok;
    QString sel = QInputDialog::getItem(this,"Partição","Qual?",parts,0,false,&ok);
    if (!ok) return;
    if (QMessageBox::warning(this,"Confirmar",QString("Formatar %1?").arg(sel),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    m_prog->setVisible(true);
    m_mgr->formatPartition(sel,m_fsCb->currentText(),m_labelEd->text().trimmed(),m_quickCk->isChecked());
}

void DiskFormatter::onWipeClicked()
{
    int idx = m_diskCb->currentIndex();
    if (idx<0 || idx>=m_disks.size()) return;
    if (QMessageBox::critical(this,"ATENÇÃO",QString("LIMPAR %1?").arg(m_disks[idx].name),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
    m_prog->setVisible(true);
    m_mgr->wipeDisk(m_disks[idx].name);
}

void DiskFormatter::onOperationFinished(bool ok, const QString &msg)
{
    m_prog->setVisible(false);
    m_log->append((ok?"✅ ":"❌ ")+msg);
    if (ok) refreshDisks();
}

void DiskFormatter::onOutputReceived(const QString &line)
{
    m_log->append(line.trimmed());
}
