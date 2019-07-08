#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineF>
#include <QDesktopWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <qdebug.h>

extern int x_gateway_mote1, x_mote2, x_mote3, x_mote4, x_mote5, x_mote6;
extern int y_gateway_mote1, y_mote2, y_mote3, y_mote4, y_mote5, y_mote6;
bool mote_detected = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setAttribute(Qt::WA_NoSystemBackground);
    QPoint gatewayMote, mote2, mote3, mote4, mote5, mote6;
    gatewayMote = QPoint(::x_gateway_mote1,::y_gateway_mote1);//2050(01)--Gateway
    mote2 = QPoint(::x_mote2,::y_mote2);//2093(02)
    mote3 = QPoint(::x_mote3,::y_mote3);//2363(03)
    mote4 = QPoint(::x_mote4,::y_mote4);//1972(04)
    mote5 = QPoint(::x_mote5,::y_mote5);//2119(05)
    mote6 = QPoint(::x_mote6,::y_mote6);//2080(06)
    staticMotes<<gatewayMote<<mote2<<mote3<<mote4<<mote5<<mote6;
    static_mote_names<<"2050(01) -- Gateway"<<"2093(02)"<<"2363(03)"<<"1972(04)"<<"2119(05)"<<"2080(06)";
    mote_ids<<"ef1c"<<"ed9c"<<"2dc1"<<"f442"<<"f46c"<<"ef31";

    ui->setupUi(this);
    QPixmap bkgnd(":/prefix_image/images/forest_pic.jpg");
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(palette);
    // Get all available COM Ports and store them in a QList.
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    // Read each element on the list, but
    // add only USB ports to the combo box.
    for (int i = 0; i < ports.size(); i++) {
        if (ports.at(i).portName.contains("USB")){
            ui->comboBox_Interface->addItem(ports.at(i).portName.toLocal8Bit().constData());
        }
    }
    // Show a hint if no USB ports were found.
    if (ui->comboBox_Interface->count() == 0){
        ui->textEdit_Status->insertPlainText("No USB ports available.\nConnect a USB device and try again.");
    }
}

void MainWindow::resizeEvent(QResizeEvent *evt)
{
    QPixmap bkgnd(":/prefix_image/images/forest_pic.jpg");
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(palette);

QMainWindow::resizeEvent(evt); //call base implementation
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_open_clicked()
{
    port.setQueryMode(QextSerialPort::EventDriven);
    port.setPortName("/dev/" + ui->comboBox_Interface->currentText());
    port.setBaudRate(BAUD115200);
    port.setFlowControl(FLOW_OFF);
    port.setParity(PAR_NONE);
    port.setDataBits(DATA_8);
    port.setStopBits(STOP_1);
    port.open(QIODevice::ReadWrite);

    if (!port.isOpen())
    {
        error.setText("Unable to open port!");
        error.show();
        return;
    }

    QObject::connect(&port, SIGNAL(readyRead()), this, SLOT(receive()));

    ui->pushButton_close->setEnabled(true);
    ui->pushButton_open->setEnabled(false);
    ui->comboBox_Interface->setEnabled(false);
}

void MainWindow::on_pushButton_close_clicked()
{
    if (port.isOpen())port.close();
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->comboBox_Interface->setEnabled(true);
}

void MainWindow::receive()
{
    static QString str;
    char ch;
    while (port.getChar(&ch))
    {
        str.append(ch);
        if (ch == '\n')     // End of line, start decoding
        {
            QStringList list = str.split("-");
            if(list.at(3) == "ee66"){
                mobile_mote_id = "2110(07)";
            }else if(list.at(3) == "ee65"){
                mobile_mote_id = "2066(08)";
            }
            mote_detected = true;
            track_mote_neighbour = staticMotes.at(mote_ids.indexOf(list.at(4)));
            received_list = list;
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QString mote_id;
    int index = 0, x_static_mote = 0, y_static_mote = 0, x_mobile_mote = 0, y_mobile_mote = 0;
    QImage static_mote_image(":/prefix_image/images/static_mote.png");
    QImage mobile_mote_image(":/prefix_image/images/mobile_mote.png");

    QPen blue_pen(Qt::blue);
    blue_pen.setWidth(6);
    painter.setPen(blue_pen);
    //qDebug() << staticMotes;
    foreach(QPoint point, staticMotes)
    {
        //painter.drawEllipse( point, 30, 30);
        painter.drawImage(point, static_mote_image);
        mote_id = static_mote_names.at(index);
        index++;
        x_static_mote = point.x() - 20;
        y_static_mote = point.y() + 100;
        point.setX(x_static_mote);
        point.setY(y_static_mote);
        painter.setFont(QFont("Arial", 15));
        painter.drawText(point, mote_id);
    }

    //qDebug() << track_mote_neighbour;
    QPen green_pen(Qt::green);
    green_pen.setWidth(6);
    painter.setPen(green_pen);
    if(mote_detected){
        x_mobile_mote = track_mote_neighbour.x() + 150;
        y_mobile_mote = track_mote_neighbour.y() + 25;
        mobile_mote.setX(x_mobile_mote);
        mobile_mote.setY(y_mobile_mote);
        painter.drawImage(mobile_mote, mobile_mote_image);
        x_mobile_mote = mobile_mote.x() - 20;
        y_mobile_mote = mobile_mote.y() + 100;
        mobile_mote_text.setX(x_mobile_mote);
        mobile_mote_text.setY(y_mobile_mote);
        painter.drawText(mobile_mote_text, mobile_mote_id);
    }
    QPen black_pen(Qt::black);
    black_pen.setWidth(3);
    painter.setPen(black_pen);
    for(int i=3; i<received_list.size()-1; i++){
        if(i == 3){
            draw_point1 = mobile_mote;
        }else{
            draw_point1 = staticMotes.at(mote_ids.indexOf(received_list.at(i)));
        }
        draw_point2 = staticMotes.at(mote_ids.indexOf(received_list.at(i+1)));
        painter.drawLine(draw_point1, draw_point2);
    }
}

//:/prefix_image/images/forest.jpg





