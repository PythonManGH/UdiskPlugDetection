#ifndef OPERATE_H
#define OPERATE_H

#include <QObject>

class operate : public QObject
{
    Q_OBJECT
public:
    explicit operate(QObject *parent = nullptr);

signals:

public slots:
};

#endif // OPERATE_H