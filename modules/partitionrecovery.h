#ifndef PARTITIONRECOVERY_H
#define PARTITIONRECOVERY_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include "utils/diskmanager.h"

class PartitionRecovery : public QWidget
{
    Q_OBJECT
public:
    explicit PartitionRecovery(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDisks();
    void onQuickScan();
    void onRestore();
    void onOperationFinished(bool ok, const QString &msg);
    void onOutputReceived(const QString &line);
private:
    void setupUI();
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_cb;
    QPushButton *m_scanBtn, *m_restoreBtn;
    QProgressBar *m_prog;
    QTextEdit *m_log;
    QLabel *m_status;
};

#endif
