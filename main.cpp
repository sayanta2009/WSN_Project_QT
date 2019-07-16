#include "mainwindow.h"
#include <QApplication>

int x_gateway_mote1, x_mote2, x_mote3, x_mote4, x_mote5, x_mote6;
int y_gateway_mote1, y_mote2, y_mote3, y_mote4, y_mote5, y_mote6;
int x, y;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*
     * x_gateway_mote1, y_gateway_mote1: x,y co-ordinates for the static gateway mote
     * Remaining are the x,y co-ordinates for the other 5 static motes
     * */
    x_gateway_mote1 = 800;
    y_gateway_mote1 = 900;

    x_mote2 = 500;
    y_mote2 = 600;

    x_mote3 = 1200;
    y_mote3 = 600;

    x_mote4 = 500;
    y_mote4 = 400;

    x_mote5 = 1200;
    y_mote5 = 400;

    x_mote6 = 800;
    y_mote6 = 100;

    x = 1920;
    y = 1200;

    MainWindow w;
    w.setWindowTitle("WildSENSE");
    w.setFixedSize(x, y);
    w.show();

    return a.exec();
}
