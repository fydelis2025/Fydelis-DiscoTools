#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "utils/diskmanager.h"
#include "modules/dashboard.h"

class DiskFormatter;
class PartitionRecovery;
class FileRecovery;
class DiskCloner;
class DiskInfo;
class UsbCreator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateTo(int index);
    void onOperationStarted(const QString &msg);
    void onOperationFinished(bool success, const QString &msg);
    void onProgressChanged(int percent);

private:
    void setupUI();
    void setupSidebar();
    void setupContent();
    void setupStatusBar();

    QWidget *m_sidebar;
    QList<QPushButton*> m_navButtons;
    QStackedWidget *m_stack;
    DiskFormatter *m_formatter;
    PartitionRecovery *m_partRecovery;
    FileRecovery *m_fileRecovery;
    DiskCloner *m_cloner;
    DiskInfo *m_diskInfo;
    UsbCreator *m_usbCreator;
    QLabel *m_statusLabel;
    QLabel *m_opLabel;
    DiskManager *m_mgr;
    Dashboard *m_dashboard;
};

#endif
