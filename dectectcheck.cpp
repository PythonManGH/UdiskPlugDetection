#include "dectectcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <QDebug>
#include <QString>

#define UEVENT_BUFFER_SIZE 2048
static bool isUsbConnected = false;


#include <errno.h>
#include <sys/types.h>
#include <asm/types.h>
//该头文件需要放在netlink.h前面防止编译出现__kernel_sa_family未定义
#include <sys/socket.h>
#include <linux/netlink.h>



DectectCheck::DectectCheck(QObject *parent) : QObject(parent)
{
    HotplugSockInit();
}

int DectectCheck::HotplugSockInit()
{

    const int buffersize = 1024;
    int ret;
    struct sockaddr_nl snl;
    bzero(&snl, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    //s_hotplugSock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    s_hotplugSock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (s_hotplugSock == -1) {
        qDebug() << __FUNCTION__ << "socket failed";
        return -1;
    }
    setsockopt(s_hotplugSock, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    ret = bind(s_hotplugSock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (ret < 0) {
        qDebug() << __FUNCTION__ << "bind failed";
        close(s_hotplugSock);
    }
    qDebug() << __FUNCTION__ << "HotplugSockInit over ret=" << ret;
    return ret;
}

void DectectCheck::HotPlugMonitor()
{
    char *result = NULL;
    char buf[UEVENT_BUFFER_SIZE * 2] = {0};     //  Netlink message buffer
    memset(buf, 0, UEVENT_BUFFER_SIZE * 2);

    QString strmsg, backmsg;
    while (!bquit) {
        recv(s_hotplugSock, &buf, sizeof(buf), 0); // 获取 USB 设备的插拔会出现字符信息，MSG_WAITALL阻塞直到接受到消息
        strmsg = buf;
        result = strtok(buf, "@");
        // 查看 USB的插入还是拔出信息
        if (strmsg.contains("@") && result) {
            if ((result == "add" && !backmsg.contains(result)) || backmsg.isEmpty()) {
                backmsg = result;
                emit sigDevicNumChanged();
                qDebug() << "+++++++";
            } else if ((result == "remove" && !backmsg.contains(result)) || backmsg.isEmpty()) {
                backmsg = result;
                emit sigDevicNumChanged();
                qDebug() << "---------";
            }
            qDebug() << __FUNCTION__ << "=====" << strmsg << backmsg << result << (result == "remove") << !backmsg.contains(result);
        }
        sleep(1);
        memset(buf, 0, UEVENT_BUFFER_SIZE * 2);
        continue;
    }
    sleep(1);
}

void DectectCheck::MonitorNetlinkUevent()
{
    int sockfd;
    struct sockaddr_nl sa;
    int len;
    char buf[4096];
    struct iovec iov;
    struct msghdr msg;
    int i;
    memset(buf, 0, sizeof(buf));
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;//getpid(); both is ok
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void *)buf;
    iov.iov_len = sizeof(buf);
    msg.msg_name = (void *)&sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sockfd = socket(AF_NETLINK/*PF_NETLINK*/, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sockfd == -1)
        printf("socket creating failed:%s\n", strerror(errno));
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
        printf("bind error:%s\n", strerror(errno));
    QString strmsg;
    bool bfirst = true;
    while (!bquit) {
        len = recvmsg(sockfd, &msg, 0); //MSG_WAITALL
        int flags = fcntl(sockfd, F_GETFL, NULL);
        if (len == -1) {
            if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) == 0) {
                qDebug() << "******************";
                bfirst = true;
                //qDebug() << len << strmsg;
                printf(strmsg.toStdString().c_str());
                // continue;
            }
        } else if (bfirst) {
            if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == 0) {
                bfirst = false;
            }
        }
        for (i = 0; i < len; i++)
            if (*(buf + i) == '\0')
                buf[i] = '\n';
        strmsg.append(buf);

        memset(buf, 0, sizeof(buf));
        if (!bfirst)
            usleep(500);
    }
}
void DectectCheck::slotStartPlugDetect()
{
    MonitorNetlinkUevent();
    //HotPlugMonitor();
}

void DectectCheck::slotQuit()
{
    bquit = true;
}
