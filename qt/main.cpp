#include "mytimer.h"
#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Create an application and mytimer window
    QApplication app(argc, argv);
    QWidget* window = new QWidget();
    QWidget* box = new QWidget();
    MyTimer* mytimer = new MyTimer(window);
    QGridLayout* layout = new QGridLayout(window);
    
    // Create a label and add it to the window
    QLabel label;
    label.setText("Printing Line 1\nPrinting Line 2\n");
    label.setAlignment(Qt::AlignCenter);
    layout->addWidget(&label, 0, 0);
    layout->addWidget(box, 1, 0);
    window->setLayout(layout);
    window->showFullScreen();
    window->show();

    // Create a few palletts
    QPalette red = box->palette();
    red.setColor(QPalette::Window, Qt::red);
    QPalette green = box->palette();
    green.setColor(QPalette::Window, Qt::green);
    QPalette yellow = box->palette();
    yellow.setColor(QPalette::Window, Qt::yellow);
    QPalette blue = box->palette();
    blue.setColor(QPalette::Window, Qt::blue);

    box->setAutoFillBackground(true);

    // Every 0.3s, update the label's text
    while(1){
	char temp[128];
	sprintf(temp, "%s", mytimer->buffer);
	
	if(mytimer->br == 'H') box->setPalette(red);
	else if(mytimer->br == 'M') box->setPalette(yellow);
	else if(mytimer->br == 'L') box->setPalette(green);
	else box->setPalette(blue);

        label.setText(temp);
        qApp->processEvents();
	usleep(100000);
    }
    return app.exec();
}
