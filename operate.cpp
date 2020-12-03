#include "operate.h"
#include "dectectcheck.h"
#include <QThread>

operate::operate(QObject *parent) : QObject(parent)
{
    QThread *workthread = new QThread;
    DectectCheck *pdect = new DectectCheck;
    pdect->moveToThread(workthread);
    connect(workthread, &QThread::started, pdect, &DectectCheck::slotStartPlugDetect);
    workthread->start();
}
