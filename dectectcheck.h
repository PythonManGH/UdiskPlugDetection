#ifndef DECTECTCHECK_H
#define DECTECTCHECK_H

#include <QObject>

class DectectCheck : public QObject
{
    Q_OBJECT
public:
    explicit DectectCheck(QObject *parent = nullptr);

private:
    int HotplugSockInit();
    void HotPlugMonitor();
    void MonitorNetlinkUevent();

    int s_hotplugSock = 0;
    bool bquit = false;
signals:
    void sigDevicNumChanged();

public slots:
    void slotStartPlugDetect();
    void slotQuit();
};

#endif // DECTECTCHECK_H
