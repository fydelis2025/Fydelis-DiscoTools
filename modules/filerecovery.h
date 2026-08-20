#ifndef FILERECOVERY_H
#define FILERECOVERY_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "utils/diskmanager.h"

class FileRecovery : public QWidget
{
    Q_OBJECT
public:
    explicit FileRecovery(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDevices();
    void onRecover();
    void onBrowse();
    void onOperationFinished(bool ok, const QString &msg);
    void onOutputReceived(const QString &line);
private:
    void setupUI();
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_devCb;
    QLineEdit *m_outEd;
    QComboBox *m_typeCb;
    QPushButton *m_goBtn;
    QProgressBar *m_prog;
    QTextEdit *m_log;
};

#endif
