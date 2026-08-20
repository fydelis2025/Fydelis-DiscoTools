#include "dashboard.h"
#include "utils/styles.h"
#include <QHBoxLayout>
#include <QDateTime>
#include <QProcess>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QResizeEvent>

// ─── StatCard ───────────────────────────────────────────────
StatCard::StatCard(const QString &icon, const QString &title,
                   const QString &value, const QColor &accentColor, QWidget *parent)
    : QFrame(parent), m_accent(accentColor)
{
    setObjectName("Card");
    setMinimumSize(200,120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet(QString("QFrame#Card{background:#1e1e2f;border-radius:14px;border:1px solid #2a2a3f;padding:16px;}"
                          "QFrame#Card:hover{border:1px solid %1;}").arg(accentColor.name()));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(16,12,16,12);
    m_iconLabel = new QLabel(icon); m_iconLabel->setStyleSheet("font-size:28px;");
    m_valueLabel = new QLabel(value);
    m_valueLabel->setStyleSheet(QString("font-size:32px;font-weight:bold;color:%1;").arg(accentColor.name()));
    m_titleLabel = new QLabel(title);
    m_titleLabel->setStyleSheet(QString("color:%1;font-size:13px;").arg(FydelisTheme::TEXT_SECONDARY.name()));
    m_subLabel = new QLabel(""); m_subLabel->setStyleSheet("color:#5a5a6a;font-size:11px;");
    l->addWidget(m_iconLabel); l->addWidget(m_valueLabel); l->addWidget(m_titleLabel); l->addWidget(m_subLabel);
}

void StatCard::setValue(const QString &v) { m_valueLabel->setText(v); }
void StatCard::setSubText(const QString &t) { m_subLabel->setText(t); m_subLabel->setVisible(!t.isEmpty()); }

// ─── StorageBar ──────────────────────────────────────────────
StorageBar::StorageBar(const QString &name, qint64 used, qint64 total, const QColor &color, QWidget *parent)
    : QFrame(parent), m_color(color)
{
    setObjectName("Card");
    setStyleSheet("QFrame#Card{background:#1a1a2e;border-radius:10px;border:1px solid #2a2a3f;padding:8px 12px;}");
    QVBoxLayout *l = new QVBoxLayout(this); l->setContentsMargins(12,6,12,6); l->setSpacing(4);
    QHBoxLayout *hr = new QHBoxLayout();
    m_nameLabel = new QLabel(name); m_nameLabel->setStyleSheet("font-size:13px;font-weight:bold;color:#e0e0e0;");
    m_infoLabel = new QLabel(); m_infoLabel->setStyleSheet("font-size:11px;color:#a0a0b0;");
    hr->addWidget(m_nameLabel); hr->addStretch(); hr->addWidget(m_infoLabel);
    l->addLayout(hr);
    m_bar = new QProgressBar(); m_bar->setRange(0,100); m_bar->setFixedHeight(16);
    m_bar->setStyleSheet(QString("QProgressBar{background:#2a2a3f;border:none;border-radius:8px;text-align:center;font-size:10px;color:#e0e0e0;}"
                                 "QProgressBar::chunk{background:%1;border-radius:8px;}").arg(color.name()));
    l->addWidget(m_bar);
    updateValues(used, total);
}

void StorageBar::updateValues(qint64 used, qint64 total)
{
    if(total<=0){m_bar->setValue(0);m_infoLabel->setText("N/A");return;}
    int p=(int)(used*100/total); m_bar->setValue(p);
    m_bar->setFormat(QString("%1%").arg(p));
    m_infoLabel->setText(DiskManager::humanReadableSize(used)+" / "+DiskManager::humanReadableSize(total));
}

// ─── Dashboard ──────────────────────────────────────────────
Dashboard::Dashboard(DiskManager *mgr, QWidget *parent)
    : QWidget(parent), m_mgr(mgr)
{
    setupUI();
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &Dashboard::onTimerTick);
    m_refreshTimer->start(10000);
    QTimer::singleShot(300, this, &Dashboard::refreshAll);
}

void Dashboard::setupUI()
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);

    // Header
    QWidget *hdr = new QWidget(); hdr->setFixedHeight(90);
    hdr->setStyleSheet("background:#161623;padding:16px 24px;");
    QVBoxLayout *hl = new QVBoxLayout(hdr); hl->setContentsMargins(28,12,28,12);
    QHBoxLayout *wr = new QHBoxLayout();
    m_welcomeLabel = new QLabel("FydelisDisk Suite");
    m_welcomeLabel->setStyleSheet(QString("font-size:24px;font-weight:bold;color:%1;").arg(FydelisTheme::ACCENT.name()));
    m_dateLabel = new QLabel(); m_dateLabel->setStyleSheet("color:#a0a0b0;font-size:13px;");
    wr->addWidget(m_welcomeLabel); wr->addStretch(); wr->addWidget(m_dateLabel);
    hl->addLayout(wr);
    QLabel *sub = new QLabel("Utilitários de disco · FydelisTechOS");
    sub->setStyleSheet("color:#a0a0b0;font-size:13px;"); hl->addWidget(sub);
    ml->addWidget(hdr);

    // Scroll content
    QScrollArea *sa = new QScrollArea(); sa->setWidgetResizable(true); sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet("QScrollArea{border:none;background:#161623;}");
    QWidget *sc = new QWidget(); sc->setStyleSheet("background:transparent;");
    QVBoxLayout *cl = new QVBoxLayout(sc); cl->setContentsMargins(24,20,24,20); cl->setSpacing(16);

    // Stat cards grid
    m_gridLayout = new QGridLayout(); m_gridLayout->setSpacing(16);
    m_totalDisksCard = new StatCard("💽","Discos","0",FydelisTheme::ACCENT);
    m_totalPartitionsCard = new StatCard("📦","Partições","0",FydelisTheme::SUCCESS);
    m_systemDiskCard = new StatCard("🖥","Sistema","—",FydelisTheme::WARNING);
    m_usbDevicesCard = new StatCard("🔌","USB","0",FydelisTheme::ACCENT_LIGHT);
    m_healthCard = new StatCard("❤️","Saúde","OK",FydelisTheme::SUCCESS);
    m_cloneTargetCard = new StatCard("🔄","Clone","—",FydelisTheme::ACCENT);
    m_gridLayout->addWidget(m_totalDisksCard,0,0);
    m_gridLayout->addWidget(m_totalPartitionsCard,0,1);
    m_gridLayout->addWidget(m_systemDiskCard,0,2);
    m_gridLayout->addWidget(m_usbDevicesCard,0,3);
    m_gridLayout->addWidget(m_healthCard,1,0);
    m_gridLayout->addWidget(m_cloneTargetCard,1,1);
    cl->addLayout(m_gridLayout);

    // Storage
    cl->addWidget(new QLabel("📊 Armazenamento"));
    QFrame *scard = new QFrame(); scard->setObjectName("Card"); scard->setStyleSheet(FydelisTheme::cardStyle());
    m_storageLayout = new QVBoxLayout(scard); m_storageLayout->setContentsMargins(16,12,16,12);
    m_storageLayout->addWidget(new QLabel("Carregando..."));
    cl->addWidget(scard);

    // Quick actions
    cl->addWidget(new QLabel("⚡ Ações Rápidas"));
    QWidget *ac = new QWidget(); ac->setObjectName("Card"); ac->setStyleSheet(FydelisTheme::cardStyle());
    QHBoxLayout *al = new QHBoxLayout(ac); al->setSpacing(12);
    QString qs = "QPushButton{background:#1a1a2e;color:#e0e0e0;border:2px solid #2a2a3f;border-radius:12px;padding:16px;font-size:12px;font-weight:bold;min-width:120px;}"
                 "QPushButton:hover{border-color:#00b4d8;}";
    m_quickFormatBtn = new QPushButton("💾 Formatar"); m_quickFormatBtn->setStyleSheet(qs);
    m_quickCloneBtn = new QPushButton("🔄 Clonar"); m_quickCloneBtn->setStyleSheet(qs);
    m_quickRecoverBtn = new QPushButton("🔧 Recuperar"); m_quickRecoverBtn->setStyleSheet(qs);
    m_quickUSBBtn = new QPushButton("💿 USB"); m_quickUSBBtn->setStyleSheet(qs);
    connect(m_quickFormatBtn,&QPushButton::clicked,this,&Dashboard::onQuickFormat);
    connect(m_quickCloneBtn,&QPushButton::clicked,this,&Dashboard::onQuickClone);
    connect(m_quickRecoverBtn,&QPushButton::clicked,this,&Dashboard::onQuickRecover);
    connect(m_quickUSBBtn,&QPushButton::clicked,this,&Dashboard::onQuickUSB);
    al->addWidget(m_quickFormatBtn); al->addWidget(m_quickCloneBtn);
    al->addWidget(m_quickRecoverBtn); al->addWidget(m_quickUSBBtn); al->addStretch();
    cl->addWidget(ac);

    // System info
    QHBoxLayout *br = new QHBoxLayout(); br->setSpacing(16);
    QFrame *sys = new QFrame(); sys->setObjectName("Card"); sys->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *sl2 = new QVBoxLayout(sys);
    sl2->addWidget(new QLabel("🖥 Sistema"));
    auto addInfo = [&](const QString &lbl, QLabel *&vl){
        QHBoxLayout *r = new QHBoxLayout();
        QLabel *lb = new QLabel(lbl); lb->setStyleSheet("color:#a0a0b0;font-size:12px;min-width:90px;");
        vl = new QLabel("--"); vl->setStyleSheet("color:#e0e0e0;font-size:12px;font-weight:bold;");
        r->addWidget(lb); r->addWidget(vl,1); sl2->addLayout(r);
    };
    addInfo("Hostname:",m_hostnameLabel);
    addInfo("Kernel:",m_kernelLabel);
    addInfo("Arquitetura:",m_archLabel);
    addInfo("Uptime:",m_uptimeLabel);
    br->addWidget(sys,1);

    QFrame *rc = new QFrame(); rc->setObjectName("Card"); rc->setStyleSheet(FydelisTheme::cardStyle());
    QVBoxLayout *rl = new QVBoxLayout(rc);
    rl->addWidget(new QLabel("📋 Operações Recentes"));
    m_recentOpsLabel = new QLabel("Nenhuma operação ainda."); m_recentOpsLabel->setStyleSheet("color:#a0a0b0;font-size:12px;");
    m_recentOpsLabel->setWordWrap(true);
    rl->addWidget(m_recentOpsLabel);
    br->addWidget(rc,1);
    cl->addLayout(br);
    cl->addStretch();

    sa->setWidget(sc);
    ml->addWidget(sa,1);
}

void Dashboard::refreshAll()
{
    m_disks = m_mgr->listDisks();
    updateStats();
    updateStorageBars();
    updateSystemInfo();
    m_dateLabel->setText(QDateTime::currentDateTime().toString("dddd, d MMMM yyyy · HH:mm"));
}

void Dashboard::onTimerTick() { refreshAll(); }

void Dashboard::updateStats()
{
    int td=0,tp=0,usb=0; bool sys=false; QString sysName;
    for(const auto &d:m_disks){
        if(d.type=="disk"){td++;tp+=d.partitions.size();if(d.transport=="usb")usb++;if(d.isSystem){sys=true;sysName=d.name;}}
    }
    m_totalDisksCard->setValue(QString::number(td));
    m_totalPartitionsCard->setValue(QString::number(tp));
    m_systemDiskCard->setValue(sys?sysName:"—");
    m_usbDevicesCard->setValue(QString::number(usb));
    m_healthCard->setValue(td>0?"✅ OK":"⚠ Atenção");
    m_cloneTargetCard->setValue(td>=2?"✅ Pronto":"⚠ +1 disco");
}

void Dashboard::updateStorageBars()
{
    QLayoutItem *child;
    while((child=m_storageLayout->takeAt(0))!=nullptr){if(child->widget())child->widget()->deleteLater();delete child;}

    QProcess df; 
    df.start("df", QStringList() << "-B1" << "--output=source,size,used,avail,target,fstype", QIODevice::ReadOnly);
    df.waitForFinished(5000);
    QStringList lines = QString::fromUtf8(df.readAllStandardOutput()).split("\n",Qt::SkipEmptyParts);
    QList<QColor> colors = {QColor(0,180,216),QColor(6,214,160),QColor(255,209,102),QColor(239,71,111),QColor(114,9,183),QColor(247,127,0)};
    int c=0;
    for(int i=1;i<lines.size()&&c<6;i++){
        QStringList p = lines[i].split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
        if(p.size()<6||p[0]=="tmpfs"||p[0]=="devtmpfs"||p[0]=="overlay") continue;
        QString name = p[4]; if(name=="/") name="Sistema (/)";
        m_storageLayout->addWidget(new StorageBar(name,p[2].toLongLong(),p[1].toLongLong(),colors[c%colors.size()]));
        c++;
    }
    if(c==0){QLabel *nl=new QLabel("Nenhum disco montado.");nl->setStyleSheet("color:#a0a0b0;");nl->setAlignment(Qt::AlignCenter);m_storageLayout->addWidget(nl);}
    m_storageLayout->addStretch();
}

void Dashboard::updateSystemInfo()
{
    QProcess h;
    h.start("hostname");
    h.waitForFinished(2000);
    m_hostnameLabel->setText(QString::fromUtf8(h.readAllStandardOutput()).trimmed());

    QProcess k;
    k.start("uname", QStringList() << "-r");
    k.waitForFinished(2000);
    m_kernelLabel->setText(QString::fromUtf8(k.readAllStandardOutput()).trimmed());

    QProcess a;
    a.start("uname", QStringList() << "-m");
    a.waitForFinished(2000);
    m_archLabel->setText(QString::fromUtf8(a.readAllStandardOutput()).trimmed());

    QFile uf("/proc/uptime");
    if(uf.open(QIODevice::ReadOnly)){
        QStringList up = QString::fromUtf8(uf.readAll()).trimmed().split(" ");
        double sec = up.first().toDouble();
        int d=(int)(sec/86400),h2=(int)((sec-d*86400)/3600),m=(int)((sec-d*86400-h2*3600)/60);
        m_uptimeLabel->setText(QString("%1d %2h %3m").arg(d).arg(h2).arg(m));
        uf.close();
    }
}

void Dashboard::onQuickFormat()
{
    QWidget *w=this; while(w){w=w->parentWidget(); if(w && w->metaObject()->className()==QString("MainWindow")){QMetaObject::invokeMethod(w,"navigateTo",Q_ARG(int,2));break;}}
}
void Dashboard::onQuickClone()
{
    QWidget *w=this; while(w){w=w->parentWidget(); if(w && w->metaObject()->className()==QString("MainWindow")){QMetaObject::invokeMethod(w,"navigateTo",Q_ARG(int,5));break;}}
}
void Dashboard::onQuickRecover()
{
    QWidget *w=this; while(w){w=w->parentWidget(); if(w && w->metaObject()->className()==QString("MainWindow")){QMetaObject::invokeMethod(w,"navigateTo",Q_ARG(int,3));break;}}
}
void Dashboard::onQuickUSB()
{
    QWidget *w=this; while(w){w=w->parentWidget(); if(w && w->metaObject()->className()==QString("MainWindow")){QMetaObject::invokeMethod(w,"navigateTo",Q_ARG(int,6));break;}}
}
void Dashboard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Caso queira ajustar colunas responsivamente no futuro, pode tratar aqui.
}
