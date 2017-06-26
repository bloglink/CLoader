/*******************************************************************************
 * Copyright (c) 2016,青岛艾普智能仪器有限公司
 * All rights reserved.
 *
 * version:     0.6
 * author:      link
 * date:        2016.09.28
 * brief:       自动调试工具
*******************************************************************************/
#include "CWinLoader.h"
#include "ui_CWinLoader.h"
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       构造
******************************************************************************/
CWinLoader::CWinLoader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWinLoader)
{
    ui->setupUi(this);
    WinInit();
    KeyInit();
    DatInit();
    CanInit();
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(CanRead()));
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       析构
******************************************************************************/
CWinLoader::~CWinLoader()
{
    delete ui;
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       界面初始化
******************************************************************************/
void CWinLoader::WinInit()
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui->tabDevice->verticalHeader()->setResizeMode(QHeaderView::Stretch);
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       按键初始化
******************************************************************************/
void CWinLoader::KeyInit()
{
    btnGroup = new QButtonGroup;
    btnGroup->addButton(ui->KeyStart, Qt::Key_A);
    //    btnGroup->addButton(ui->KeyReset, Qt::Key_B);
    btnGroup->addButton(ui->KeyExit,  Qt::Key_E);
    connect(btnGroup,SIGNAL(buttonClicked(int)),this,SLOT(KeyJudge(int)));
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       按键功能
******************************************************************************/
void CWinLoader::KeyJudge(int id)
{
    switch (id) {
    case Qt::Key_A:
        if (ui->KeyStart->text().contains(tr("开始下载"))) {
            ui->textBrowser->clear();
            IDInit();
            text.clear();
            text.append(0x0F);
            CanWrite(text);
            step = 0;
            timer->start(50);
            ui->KeyStart->setText(tr("中断下载"));
        } else {
            step = 0;
            timer->stop();
            ui->KeyStart->setText(tr("开始下载"));
        }
        break;
    case Qt::Key_B:
        CanWrite("G");
        break;
    case Qt::Key_E:
        this->close();
        break;
    default:
        break;
    }
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       初始化数据
******************************************************************************/
void CWinLoader::DatInit()
{
    QDir dir(QString(NET));
    if (!dir.exists()) {
        SendMsg("未发现网络升级文件\n");
        return;
    }
    dir.setFilter(QDir::Dirs|QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    for (int i=0; i<list.size(); i++) {
        if (list.at(i).fileName().contains(".bin")) {
            ui->tabSoftware->setRowCount(ui->tabSoftware->rowCount()+1);
            ui->tabSoftware->setItem(ui->tabSoftware->rowCount()-1,0,new QTableWidgetItem);
            ui->tabSoftware->item(ui->tabSoftware->rowCount()-1,0)->setText(list.at(i).filePath());
        }
    }
    dir.setPath(QString(USB));
    if (!dir.exists()) {
        SendMsg(QString(tr("未发现USB升级文件\n")).toUtf8());
        return;
    }
    list = dir.entryInfoList();
    for (int i=0; i<list.size(); i++) {
        if (list.at(i).fileName().contains(".bin")) {
            ui->tabSoftware->setRowCount(ui->tabSoftware->rowCount()+1);
            ui->tabSoftware->setItem(ui->tabSoftware->rowCount()-1,0,new QTableWidgetItem);
            ui->tabSoftware->item(ui->tabSoftware->rowCount()-1,0)->setText(list.at(i).filePath());
        }
    }
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       初始化数据
******************************************************************************/
void CWinLoader::IDInit()
{
    if (ui->tabDevice->currentItem()->text() == tr("电阻"))
        id = 0x02;
    else if (ui->tabDevice->currentItem()->text() == tr("耐压"))
        id = 0x03;
    else if (ui->tabDevice->currentItem()->text() == tr("匝间"))
        id = 0x04;
    else if (ui->tabDevice->currentItem()->text() == tr("功率"))
        id = 0x05;
    else if (ui->tabDevice->currentItem()->text() == tr("电感"))
        id = 0x06;
    else if (ui->tabDevice->currentItem()->text() == tr("备用07"))
        id = 0x07;
    else if (ui->tabDevice->currentItem()->text() == tr("备用08"))
        id = 0x08;
    else if (ui->tabDevice->currentItem()->text() == tr("备用09"))
        id = 0x09;
    else if (ui->tabDevice->currentItem()->text() == tr("输出1"))
        id = 0x13;
    else if (ui->tabDevice->currentItem()->text() == tr("输出2"))
        id = 0x14;
    else if (ui->tabDevice->currentItem()->text() == tr("输出3"))
        id = 0x15;
    else if (ui->tabDevice->currentItem()->text() == tr("输出4"))
        id = 0x16;
    else if (ui->tabDevice->currentItem()->text() == tr("输出5"))
        id = 0x17;
    else
        id = 0x01;
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       打开CAN口
******************************************************************************/
void CWinLoader::CanInit()
{
    struct sockaddr_can     addr;
    struct ifreq            ifr;
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  /*打开套接字*/
    if (s < 0){
        SendMsg("Error while opening socket\n");
        return;
    }
    strcpy(ifr.ifr_name, "can0" );
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0){
        SendMsg("Error while opening can0\n");
        return;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        SendMsg("Error in socket bind\n");
        return;
    }
    SendMsg("CAN0打开成功\n");
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       CAN读取一帧
******************************************************************************/
bool CWinLoader::CanReadFrame()
{
    int nbytes;
    int ret;
    fd_set rdfds;
    struct timeval tv;

    FD_ZERO(&rdfds);
    FD_SET(s, &rdfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    ret = select(s+1, &rdfds, NULL, NULL, &tv);
    if (ret <= 0){
        return false;
    }
    if (FD_ISSET(s, &rdfds)){
        nbytes = read(s, &RxMsg, sizeof(struct can_frame));
        return true;
    }
    return false;
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       CAN写入一帧
******************************************************************************/
bool CWinLoader::CanWriteFrame(can_frame TxMsg)
{
    int nbytes;
    nbytes = write(s, &TxMsg, sizeof(struct can_frame));
    if (nbytes != sizeof(struct can_frame)){
        perror("can raw socket write");
        SendMsg("can raw socket write Error");
        return false;
    }
    return true;
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       CAN写入
******************************************************************************/
bool CWinLoader::CanWrite(QByteArray msg)
{
    QByteArray frame;
    can_frame  TxMsg;
    TxMsg.can_id = id;
    for (int i=0; i<msg.size(); i+=8) {
        frame = msg.mid(i,8);
        TxMsg.can_dlc = frame.size();
        for (int t=0; t<frame.size(); t++) {
            TxMsg.data[t] = frame.at(t);
        }
        if(CanWriteFrame(TxMsg))
            Delay(1);
        else
            return false;
    }
    return true;
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       CAN退出
******************************************************************************/
void CWinLoader::CanQuit()
{
    step = 0;
    timer->stop();
    ui->KeyStart->setText("开始下载");
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       读取文件
******************************************************************************/
void CWinLoader::FileRead()
{
    text = file->read(1024);
    if (file->atEnd()) {
        for (int i=text.size(); i<1024; i++)
            text.append(0xff);
    }
    quint8 crc = 0;
    for (int i=0; i<1024; i++)
        crc+= text[i];
    text.insert(0,'S');
    text.insert(1,page);
    text.insert(2,!page);
    text.insert(3,crc);
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       CAN读取
******************************************************************************/
void CWinLoader::CanRead()
{
    QByteArray ack;
    if (CanReadFrame()) {
        for (int i=0; i<RxMsg.can_dlc; i++) {
            ack.append(RxMsg.data[i]);
        }
        switch (ack.at(0)) {
        case 'R':
            file = new QFile(ui->textFile->text());
            if (!file->open(QFile::ReadOnly)) {
                SendMsg(QString(tr("打开文件失败\n")).toUtf8());
                step = QUIT;
                break;
            }
            SendMsg(QString(tr("正在同步设备......   ")).toUtf8());
            page = 0;
            step = SYNC;
            break;
        case 'C':
            SendMsg(QString(tr("同步成功\n")).toUtf8());
            step = SEND;
            break;
        case 'A':
            SendMsg(QString(tr("写入成功\n")).toUtf8());
            page++;
            if (file->atEnd())
                step = OVER;
            else
                step = SEND;
            break;
        case 'N':
            SendMsg(QString(tr("写入失败\n")).toUtf8());
            step = FAIL;
            break;
        case 'E':
            SendMsg(QString(tr("写入完成\n")).toUtf8());
            step = PROT;
            page = 0;
            break;
        case 'P':
            SendMsg(QString(tr("加密成功\n")).toUtf8());
            step = QUIT;
            break;
        case 'V':
            ack = ack.mid(1,3);
            if (ack.toDouble()<0.5) {
                ack.insert(0,tr("Bootloader版本V"));
                ack.append(tr("    该版本无法使用加密功能\n"));
                SendMsg(ack);
                break;
            } else {
                ack.insert(0,tr("Bootloader版本V"));
                ack.append("\n");
                SendMsg(ack);
            }
            step = FREE;
            break;
        default:
            break;
        }
    }
    switch (step) {
    case FREE:
        CanWrite("R");
        break;
    case SYNC:
        CanWrite("C");
        break;
    case SEND:
        if (ack.isEmpty())
            break;
        FileRead();
        SendMsg(QString(tr("正在写入第%1帧数据......   ")).arg(page+1, 2, 10, QChar('0')).toUtf8());
        CanWrite(text);
        break;
    case FAIL:
        SendMsg(QString(tr("重新写入第%1帧数据......   ")).arg(page+1, 2, 10, QChar('0')).toUtf8());
        CanWrite(text);
        break;
    case OVER:
        SendMsg(QString(tr("正在完成写入.....   ")).toUtf8());
        CanWrite("E");
        break;
    case PROT:
        SendMsg(QString(tr("正在加密程序.....   ")).toUtf8());
        CanWrite("P");
        break;
    case QUIT:
        CanWrite("G");
        CanQuit();
        step = FREE;
    default:
        break;
    }
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       显示信息
******************************************************************************/
void CWinLoader::SendMsg(QByteArray msg)
{
    ui->textBrowser->insertPlainText(msg);
    ui->textBrowser->moveCursor(QTextCursor::End);
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       延时
******************************************************************************/
void CWinLoader::Delay(int ms)
{
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<ms)
        QCoreApplication::processEvents();
}
/******************************************************************************
 * version:     1.0
 * author:      link
 * date:        2016.08.18
 * brief:       点击
******************************************************************************/
void CWinLoader::on_tabSoftware_cellClicked(int , int )
{
    QString temp = ui->tabSoftware->currentItem()->text();
    if (temp.contains(".bin")) {
        ui->textFile->setText(temp);
    }
}
