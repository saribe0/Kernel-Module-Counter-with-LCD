#include "mytimer.h"
#include <QDebug>
#include <QMainWindow>
#include <QWidget>
#include <QLayout>
#include <QBoxLayout>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

MyTimer::MyTimer(QWidget* window)
{
    // create a timer
    timer = new QTimer(this);
    mywindow = window;

    for(int i = 0; i < 128; i++)
        buffer[i] = 0;

    sprintf(buffer,"Value\tSpeed\tState\tDirection\tBrightness\n");

    // setup signal and slot
    connect(timer, SIGNAL(timeout()), this, SLOT(MyTimerSlot()));

    // msec
    timer->start(300);
}

void MyTimer::MyTimerSlot()
{
    char line[128];
    char sp, st[5], dir[5]; 
    int pFile;
    int  value, period, start_hold, up_down, brightness;

    // Open file
    pFile = open("/dev/mygpio", O_RDONLY);
    if (pFile < 0) {
        fputs("mygpio module isn't loaded\n",stderr);
    }

    // Get values from file
    read(pFile, line, 128);
    sscanf(line, "%d\n%d\n%d\n%d\n%d\n", &value, &period, &start_hold, &up_down, &brightness);

    //Frequency
    if(period == 4000) sp = 'L';
    else if(period == 2000) sp ='M';
    else if(period == 500) sp = 'H';
    else sp = '?';

    //State
    if(start_hold) strcpy(st, "Run ");
    else strcpy(st, "Hold");

    //Direction
    if(!up_down) strcpy(dir, "Up  ");
    else strcpy(dir, "Down");

    //Brightness
    if(brightness == 0) br = 'L';
    else if(brightness == 1) br ='M';
    else if(brightness == 2) br = 'H';
    else br = '?';

    // Print values to buffer
    sprintf(buffer,"Value: %d\nSpeed: %c\nState: %s\nDirection: %s\nBrightness: %c\n",value,sp,st,dir,br);

    timer->start(300);
}
