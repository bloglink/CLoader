/*******************************************************************************
 * Copyright [2017]   <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       CAN自动升级程序
*******************************************************************************/
#ifndef CWINLOADER_H
#define CWINLOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <QTimer>
#include <QDebug>
#include <QWidget>
#include <QMimeData>
#include <QSettings>
#include <QDropEvent>
#include <QFileDialog>
#include <QCloseEvent>
#include <QStringList>
#include <QDataStream>
#include <QButtonGroup>
#include <QElapsedTimer>
#include <QDragEnterEvent>
#include <QTranslator>

#define NET "/mnt/network/"
#define TMP "./temp/"
#define CON "./config/"
#define USB "/mnt/usb1/aip/"

#define FREE 0x00
#define SYNC 0x01
#define READ 0x02
#define SEND 0x03
#define FAIL 0x04
#define OVER 0x05
#define QUIT 0x06
#define PROT 0x07

namespace Ui {
class CWinLoader;
}

class CWinLoader : public QWidget
{
    Q_OBJECT

public:
    explicit CWinLoader(QWidget *parent = 0);
    ~CWinLoader();

private:
    Ui::CWinLoader *ui;

private slots:
    void WinInit(void);
    void KeyInit(void);
    void KeyJudge(int id);
    void DatInit(void);
    void IDInit(void);
    void CanInit(void);
    void CanRead(void);
    bool CanReadFrame(void);
    bool CanWriteFrame(can_frame TxMsg);
    bool CanWrite(QByteArray msg);
    void CanQuit(void);

    void FileRead(void);

    void SendMsg(QByteArray msg);
    void Delay(int ms);
    void on_tabSoftware_cellClicked(int row, int column);

    void on_tabDevice_clicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    QButtonGroup *btnGroup;
    QSettings *setting;
    can_frame  RxMsg;
    QByteArray text;
    QTimer *timer;
    QFile *file;
    quint8 step;
    quint8 page;
    int  s;
    int id;

    QTranslator *translator;
};

#endif // CWINLOADER_H
