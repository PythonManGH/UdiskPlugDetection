#include <QCoreApplication>
#include "operate.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
    setbuf(stdout, nullptr);
    QCoreApplication a(argc, argv);
    operate op;
    return a.exec();
}
