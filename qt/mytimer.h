#ifndef MYTIMER_H
#define MYTIMER_H

#include <QTimer>
#include <QMainWindow>
#include <QString>
#include <QLabel>
#include <QBoxLayout>
#include <QWidget>

class MyTimer : public QObject
{
    Q_OBJECT
public:
    MyTimer(QWidget*);
    QTimer* timer;
    QWidget* mywindow;
    char buffer[128];
    char br;

public slots:
    void MyTimerSlot();
};

#endif // MYTIMER_H
