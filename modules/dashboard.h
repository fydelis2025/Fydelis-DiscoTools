#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include "utils/diskmanager.h"

// ─── Stat Card Widget ───────────────────────────────────────
class StatCard : public QFrame
{
    Q_OBJECT
public:
    StatCard(const QString &icon, const QString &title,
             const QString &value, const QColor &accentColor,
             QWidget *parent = nullptr);

    void setValue(const QString &value);
    void setSubText(const QString &text);

private:
    void setupUI(const QString &icon, const QString &title,
                 const QColor &accentColor);
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_valueLabel;
    QLabel *m_subLabel;
    QColor m_accent;
};

// ─── Storage Bar Widget ─────────────────────────────────────
class StorageBar : public QFrame
{
    Q_OBJECT
public:
    StorageBar(const QString &name, qint64 used, qint64 total,
               const QColor &color, QWidget *parent = nullptr);
    void updateValues(qint64 used, qint64 total);

private:
    void setupUI(const QString &name, qint64 used, qint64 total,
                 const QColor &color);
    QLabel *m_nameLabel;
    QProgressBar *m_bar;
    QLabel *m_infoLabel;
    QColor m_color;
};

// ─── Main Dashboard ─────────────────────────────────────────
class Dashboard : public QWidget
{
    Q_OBJECT

public:
    explicit Dashboard(DiskManager *diskManager, QWidget *parent = nullptr);

    void refreshAll();

private slots:
    void onTimerTick();
    void onQuickFormat();
    void onQuickClone();
    void onQuickRecover();
    void onQuickUSB();

private:
    void setupUI();
    void rebuildGrid();
    void updateStats();
    void updateStorageBars();
    void updateSystemInfo();
    void updateRecentOperations();

    // Responsive layout
    void resizeEvent(QResizeEvent *event) override;
    int m_currentColumns = 4;

    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QTimer *m_refreshTimer;

    // Layout containers
    QScrollArea *m_scrollArea;
    QWidget *m_scrollContent;
    QGridLayout *m_gridLayout;

    // Stat cards
    StatCard *m_totalDisksCard;
    StatCard *m_totalPartitionsCard;
    StatCard *m_systemDiskCard;
    StatCard *m_usbDevicesCard;
    StatCard *m_healthCard;
    StatCard *m_cloneTargetCard;

    // Storage bars container
    QVBoxLayout *m_storageLayout;

    // Quick action buttons
    QPushButton *m_quickFormatBtn;
    QPushButton *m_quickCloneBtn;
    QPushButton *m_quickRecoverBtn;
    QPushButton *m_quickUSBBtn;

    // System info
    QLabel *m_hostnameLabel;
    QLabel *m_kernelLabel;
    QLabel *m_uptimeLabel;
    QLabel *m_archLabel;

    // Recent operations
    QLabel *m_recentOpsLabel;

    // Welcome
    QLabel *m_welcomeLabel;
    QLabel *m_dateLabel;
};

#endif // DASHBOARD_H
