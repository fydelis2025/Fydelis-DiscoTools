#ifndef DISKFORMATTER_H
#define DISKFORMATTER_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QInputDialog>
#include <QTimer>
#include "utils/diskmanager.h"

class DiskFormatter : public QWidget
{
    Q_OBJECT
public:
    explicit DiskFormatter(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDisks();
    void onDiskSelected(int idx);
    void onFormatClicked();
    void onWipeClicked();
    void onOperationFinished(bool ok, const QString &msg);
    void onOutputReceived(const QString &line);
private:
    void setupUI();
    void updateInfo(const DiskDeviceInfo &d);
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_diskCb;
    QTextEdit *m_infoTxt;
    QComboBox *m_fsCb;
    QLineEdit *m_labelEd;
    QCheckBox *m_quickCk;
    QPushButton *m_fmtBtn, *m_wipeBtn;
    QProgressBar *m_prog;
    QTextEdit *m_log;
    QTableWidget *m_partTbl;
};

#endif
