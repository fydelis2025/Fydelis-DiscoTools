#include "diskinfo.h"
#include "utils/styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QTimer>
#include <QProcess>

DiskInfo::DiskInfo(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    connect(m_cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DiskInfo::onDiskSelected);
    QTimer::singleShot(500, this, &DiskInfo::refreshDisks);
}

void DiskInfo::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(24,20,24,20); ml->setSpacing(16);
    QLabel *hdr = new QLabel("🗂  Informações do Disco");
    hdr->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    ml->addWidget(hdr);

    QFrame *card = new QFrame(); card->setObjectName("Card"); card->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *cl = new QVBoxLayout(card);
    QHBoxLayout *dr = new QHBoxLayout();
    m_cb = new QComboBox(); m_cb->setStyleSheet(FydelisTheme::comboBoxStyle());
    QPushButton *ref = new QPushButton("🔄"); ref->setStyleSheet(FydelisTheme::primaryButtonStyle());
    connect(ref, &QPushButton::clicked, this, &DiskInfo::refreshDisks);
    dr->addWidget(m_cb,1); dr->addWidget(ref);
    cl->addLayout(dr);
    ml->addWidget(card);

    m_details = new QTextEdit(); m_details->setReadOnly(true); m_details->setMaximumHeight(200);
    m_details->setStyleSheet(FydelisTheme::textEditStyle());
    ml->addWidget(m_details);

    QLabel *pl = new QLabel("Partições:");
    pl->setStyleSheet("font-size:14px;font-weight:bold;color:"+FydelisTheme::TEXT_PRIMARY.name()+";");
    ml->addWidget(pl);

    m_partTbl = new QTableWidget(0,5);
    m_partTbl->setHorizontalHeaderLabels({"Partição","Tamanho","FS","Montagem","UUID"});
    m_partTbl->setStyleSheet("QTableWidget{background:#1a1a2e;color:#e0e0e0;border:1px solid #2a2a3f;}"
                             "QHeaderView::section{background:#0f3460;color:#e0e0e0;padding:8px;font-weight:bold;}");
    m_partTbl->horizontalHeader()->setStretchLastSection(true);
    m_partTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ml->addWidget(m_partTbl,1);
}

void DiskInfo::refreshDisks()
{
    m_disks = m_mgr->listDisks();
    m_cb->clear();
    for (const auto &d : m_disks)
        m_cb->addItem(QString("%1 %2 [%3] (%4)").arg(d.isSystem?"🖥":"💽",d.name,d.transport,d.size));
    if (m_cb->count()>0) onDiskSelected(0);
}

void DiskInfo::onDiskSelected(int idx)
{
    if (idx<0 || idx>=m_disks.size()) return;
    const DiskDeviceInfo &d = m_disks[idx];

    QString info;
    info += "📌 Disco: " + d.name + "\n";
    info += "📏 Tamanho: " + d.size + "\n";
    info += "🔌 Transporte: " + (d.transport.isEmpty()?"N/A":d.transport) + "\n";
    info += "🏭 Modelo: " + (d.model.isEmpty()?"N/A":d.model) + "\n";
    info += "🖥 Sistema: " + QString(d.isSystem?"Sim":"Não") + "\n";
    info += "📦 Partições: " + QString::number(d.partitions.size()) + "\n";
    m_details->setText(info);

    m_partTbl->setRowCount(0);
    for (const auto &p : d.partitions) {
        int r = m_partTbl->rowCount();
        m_partTbl->insertRow(r);
        m_partTbl->setItem(r,0,new QTableWidgetItem(p.name));
        m_partTbl->setItem(r,1,new QTableWidgetItem(p.size));
        m_partTbl->setItem(r,2,new QTableWidgetItem(p.fsType.isEmpty()?"?":p.fsType));
        m_partTbl->setItem(r,3,new QTableWidgetItem(p.mountPoint.isEmpty()?"não montado":p.mountPoint));
        m_partTbl->setItem(r,4,new QTableWidgetItem(p.uuid.isEmpty()?"-":p.uuid));
    }
}
