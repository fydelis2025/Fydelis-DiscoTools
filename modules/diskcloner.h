#ifndef DISKCLONER_H
#define DISKCLONER_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include "utils/diskmanager.h"

class DiskCloner : public QWidget
{
    Q_OBJECT
public:
    explicit DiskCloner(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDisks();
    void onClone();
    void onSwapSourceTarget();
    void onOperationFinished(bool ok, const QString &msg);
    void onOutputReceived(const QString &line);
private:
    void setupUI();
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_srcCb, *m_tgtCb;
    QPushButton *m_goBtn, *m_swapBtn;
    QProgressBar *m_prog;
    QTextEdit *m_log;
};

#endif
