#include "mainwindow.h"
#include "utils/styles.h"
#include "modules/dashboard.h"
#include "modules/diskformatter.h"
#include "modules/partitionrecovery.h"
#include "modules/filerecovery.h"
#include "modules/diskcloner.h"
#include "modules/diskinfo.h"
#include "modules/usbcreator.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_mgr = new DiskManager(this);
    setWindowTitle("FydelisDisk Suite v1.0");
    setMinimumSize(900,600);
    resize(1200,750);

    QWidget *central = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    setupSidebar();
    setupContent();
    setupStatusBar();

    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(m_stack, 1);
    setCentralWidget(central);
    setStyleSheet(FydelisTheme::globalStyleSheet());

    connect(m_mgr, &DiskManager::operationStarted, this, &MainWindow::onOperationStarted);
    connect(m_mgr, &DiskManager::operationFinished, this, &MainWindow::onOperationFinished);
    connect(m_mgr, &DiskManager::progressChanged, this, &MainWindow::onProgressChanged);

    if (!DiskManager::isRoot())
        QMessageBox::warning(this, "Permissão", "Execute como root para acesso completo.");

    navigateTo(0);
}

MainWindow::~MainWindow() {}

void MainWindow::setupSidebar()
{
    m_sidebar = new QWidget();
    m_sidebar->setFixedWidth(240);
    m_sidebar->setStyleSheet("background-color: #1a1a2e;");

    QVBoxLayout *sl = new QVBoxLayout(m_sidebar);
    sl->setContentsMargins(0,0,0,0);
    sl->setSpacing(0);

    // Brand
    QWidget *brand = new QWidget();
    brand->setStyleSheet("background-color: #0f3460;");
    brand->setFixedHeight(100);
    QVBoxLayout *bl = new QVBoxLayout(brand);
    bl->setAlignment(Qt::AlignCenter);
    QLabel *icon = new QLabel("💿"); icon->setStyleSheet("font-size:32px;"); icon->setAlignment(Qt::AlignCenter);
    QLabel *title = new QLabel("FydelisDisk"); title->setStyleSheet("color:#00b4d8; font-size:20px; font-weight:bold;"); title->setAlignment(Qt::AlignCenter);
    QLabel *sub = new QLabel("Suite de Utilitários"); sub->setStyleSheet("color:#a0a0b0; font-size:11px;"); sub->setAlignment(Qt::AlignCenter);
    bl->addWidget(icon); bl->addWidget(title); bl->addWidget(sub);
    sl->addWidget(brand);

    struct Item { QString icon; QString text; };
    QList<Item> items = {
        {"🏠","Dashboard"},       // 0
        {"🗂","Informações"},     // 1
        {"💾","Formatar"},        // 2
        {"🔧","Recuperar Partição"}, // 3
        {"📂","Recuperar Arquivos"}, // 4
        {"🔄","Clonar Disco"},    // 5
        {"💿","USB Bootável"},    // 6
        {"⚙","Configurações"},    // 7
    };

    QScrollArea *sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setStyleSheet("QScrollArea{border:none;background:transparent;}");
    QWidget *nc = new QWidget(); nc->setStyleSheet("background:transparent;");
    QVBoxLayout *nl = new QVBoxLayout(nc); nl->setContentsMargins(10,15,10,15); nl->setSpacing(4);

    for (int i=0; i<items.size(); i++) {
        QPushButton *btn = new QPushButton(items[i].icon + "  " + items[i].text);
        btn->setFixedHeight(48); btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(FydelisTheme::sidebarButtonStyle(i==0));
        connect(btn, &QPushButton::clicked, this, [this,i](){ navigateTo(i); });
        nl->addWidget(btn);
        m_navButtons.append(btn);
    }
    nl->addStretch();
    QLabel *ver = new QLabel("v1.0.0 · FydelisTechOS");
    ver->setStyleSheet("color:#a0a0b0; font-size:10px; padding:10px;");
    ver->setAlignment(Qt::AlignCenter);
    nl->addWidget(ver);

    sa->setWidget(nc);
    sl->addWidget(sa, 1);
}

void MainWindow::setupContent()
{
    m_stack = new QStackedWidget();

    m_dashboard    = new Dashboard(m_mgr);
    m_diskInfo     = new DiskInfo(m_mgr);
    m_formatter    = new DiskFormatter(m_mgr);
    m_partRecovery = new PartitionRecovery(m_mgr);
    m_fileRecovery = new FileRecovery(m_mgr);
    m_cloner       = new DiskCloner(m_mgr);
    m_usbCreator   = new UsbCreator(m_mgr);

    QWidget *settings = new QWidget();
    settings->setStyleSheet("background-color:#161623;");
    QLabel *sl = new QLabel("⚙ Configurações\n\nEm desenvolvimento...", settings);
    sl->setStyleSheet("color:#a0a0b0; font-size:18px;");
    sl->setAlignment(Qt::AlignCenter);

    m_stack->addWidget(m_dashboard);     // 0
    m_stack->addWidget(m_diskInfo);       // 1
    m_stack->addWidget(m_formatter);      // 2
    m_stack->addWidget(m_partRecovery);   // 3
    m_stack->addWidget(m_fileRecovery);   // 4
    m_stack->addWidget(m_cloner);         // 5
    m_stack->addWidget(m_usbCreator);     // 6
    m_stack->addWidget(settings);         // 7
}

void MainWindow::setupStatusBar()
{
    m_opLabel = new QLabel("✅ Pronto");
    m_opLabel->setStyleSheet("color:#06d6a0; font-size:12px; padding:4px 10px;");
    m_statusLabel = new QLabel("Nenhuma operação");
    m_statusLabel->setStyleSheet("color:#a0a0b0; font-size:12px;");

    statusBar()->addPermanentWidget(m_opLabel);
    statusBar()->addPermanentWidget(m_statusLabel, 1);
}

void MainWindow::navigateTo(int index)
{
    if (index<0 || index>=m_navButtons.size()) return;
    for (int i=0; i<m_navButtons.size(); i++)
        m_navButtons[i]->setStyleSheet(FydelisTheme::sidebarButtonStyle(i==index));
    m_stack->setCurrentIndex(index);
}

void MainWindow::onOperationStarted(const QString &msg)
{
    m_opLabel->setText("⏳ " + msg);
    m_opLabel->setStyleSheet("color:#ffd166; font-size:12px; padding:4px 10px;");
    m_statusLabel->setText(msg);
}

void MainWindow::onOperationFinished(bool success, const QString &msg)
{
    m_opLabel->setText(success ? "✅ "+msg : "❌ "+msg);
    m_opLabel->setStyleSheet(QString("color:%1; font-size:12px; padding:4px 10px;")
                                 .arg(success ? "#06d6a0" : "#ef476f"));
    m_statusLabel->setText(msg);
}

void MainWindow::onProgressChanged(int percent)
{
    m_statusLabel->setText(QString("Progresso: %1%").arg(percent));
}
