#ifndef DISKINFO_H
#define DISKINFO_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTableWidget>
#include "utils/diskmanager.h"

class DiskInfo : public QWidget
{
    Q_OBJECT
public:
    explicit DiskInfo(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDisks();
    void onDiskSelected(int idx);
private:
    void setupUI();
    void showDiskDetails(const DiskInfo &d);
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_cb;
    QTextEdit *m_details;
    QTableWidget *m_partTbl;
};

#endif
