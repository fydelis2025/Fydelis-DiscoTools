#ifndef USBCREATOR_H
#define USBCREATOR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include "utils/diskmanager.h"

class UsbCreator : public QWidget
{
    Q_OBJECT
public:
    explicit UsbCreator(DiskManager *mgr, QWidget *parent = nullptr);
private slots:
    void refreshDisks();
    void onBrowseISO();
    void onCreateUSB();
    void onOperationFinished(bool ok, const QString &msg);
    void onOutputReceived(const QString &line);
private:
    void setupUI();
    DiskManager *m_mgr;
    QList<DiskDeviceInfo> m_disks;
    QComboBox *m_devCb;
    QLineEdit *m_isoEd;
    QPushButton *m_goBtn;
    QProgressBar *m_prog;
    QTextEdit *m_log;
};

#endif
