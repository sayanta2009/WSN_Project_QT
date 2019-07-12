#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QLabel>
#include <qdebug.h>

extern int x_gateway_mote1, x_mote2, x_mote3, x_mote4, x_mote5, x_mote6;
extern int y_gateway_mote1, y_mote2, y_mote3, y_mote4, y_mote5, y_mote6;
extern int x, y;
bool mote_detected = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
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

    ui->label_temp->setText("<font color='blue'>Temparature</font>");
    ui->label_heartbeat->setText("<font color='blue'>Heartbeat</font>");
    ui->label_battery->setText("<font color='blue'>Battery</font>");
    ui->label_tiger->setText("<font color='yellow'>Tiger</font>");
    ui->label_lion->setText("<font color='yellow'>Lion</font>");

    //tiger_Timer = new QTimer(this);
    //lion_Timer = new QTimer(this->getMyMainWindow());

    // setup signal and slot
    //connect(tiger_Timer, SIGNAL(timeout()),this, SLOT(tiger_out_of_range()));
    //connect(lion_Timer, SIGNAL(timeout()),this, SLOT(lion_out_of_range()));

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

    //tried changes
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(repaint()));
    timer->start(500);    //end tried changes

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

    tiger_Timer->stop();
    lion_Timer->stop();
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
            if (str.contains("Battery:")){
                str.remove("\n", Qt::CaseSensitive);
                ui->textEdit_Status->append(str);
                QStringList list = str.split("-");

                QString battery = list.at(0);
                QStringList battery_list = battery.split(":");
                double battery_value = battery_list.at(1).toDouble();
                battery_value = battery_value/1000;

                QString temp = list.at(1);
                QStringList temp_list = temp.split(":");
                double temp_value = temp_list.at(1).toDouble();
                temp_value = temp_value/1000;

                QString heartbeat = list.at(2);
                QStringList heartbeat_list = heartbeat.split(":");
                double heartbeat_value = heartbeat_list.at(1).toDouble();

                if(list.at(3) == "ee66"){
                    mobile_mote_id = "2110(07)";
                    ui->lcdNumber_battery_tiger->display(battery_value);
                    ui->lcdNumber_temp_tiger->display(temp_value);
                    ui->lcdNumber_heartbeat_tiger->display(heartbeat_value);

                    ui->progressBar_battery_tiger->setValue((int)battery_value);
                    ui->progressBar_temp_tiger->setValue((int)temp_value);
                    ui->progressBar_heartbeat_tiger->setValue((int)heartbeat_value);
                }else if(list.at(3) == "ee65"){
                    mobile_mote_id = "2066(08)";
                    ui->lcdNumber_battery_lion->display(battery_value);
                    ui->lcdNumber_temp_lion->display(temp_value);
                    ui->lcdNumber_heartbeat_lion->display(heartbeat_value);

                    ui->progressBar_battery_lion->setValue((int)battery_value);
                    ui->progressBar_temp_lion->setValue((int)temp_value);
                    ui->progressBar_heartbeat_lion->setValue((int)heartbeat_value);
                }
                mote_detected = true;
                track_mote_neighbour = staticMotes.at(mote_ids.indexOf(list.at(4)));
                received_list = list;
                //qDebug << received_list;
                str = "";
            }else{
                str = "";
            }
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.eraseRect(0,0,1920,1200);
    QString mote_id;
    int index = 0, x_static_mote = 0, y_static_mote = 0, x_mobile_mote = 0, y_mobile_mote = 0;
    QImage static_mote_image(":/prefix_image/images/static_mote_2.png");
    QImage mobile_mote_image(":/prefix_image/images/mobile_mote.png");

    QImage static_mote_image_resized = static_mote_image.scaled(50, 50, Qt::KeepAspectRatio);
    QImage mobile_mote_image_resized = mobile_mote_image.scaled(50, 50, Qt::KeepAspectRatio);

    QPen blue_pen(Qt::blue);
    blue_pen.setWidth(6);
    painter.setPen(blue_pen);
    foreach(QPoint point, staticMotes)
    {
        painter.drawImage(point, static_mote_image_resized);
        mote_id = static_mote_names.at(index);
        index++;
        x_static_mote = point.x() - 20;
        y_static_mote = point.y() + 100;
        if(mote_id == "2050(01) -- Gateway"){
            x_static_mote = point.x() - 20;
            y_static_mote = point.y() - 30;
        }
        point.setX(x_static_mote);
        point.setY(y_static_mote);
        painter.setFont(QFont("Arial", 15));
        painter.drawText(point, mote_id);
    }
    QPen green_pen(Qt::green);
    green_pen.setWidth(6);
    painter.setPen(green_pen);
    if(mote_detected){
        x_mobile_mote = track_mote_neighbour.x() + 150;
        y_mobile_mote = track_mote_neighbour.y() + 25;
        mobile_mote.setX(x_mobile_mote);
        mobile_mote.setY(y_mobile_mote);
        painter.drawImage(mobile_mote, mobile_mote_image_resized);
        x_mobile_mote = mobile_mote.x() - 20;
        y_mobile_mote = mobile_mote.y() + 100;
        mobile_mote_text.setX(x_mobile_mote);
        mobile_mote_text.setY(y_mobile_mote);
        painter.drawText(mobile_mote_text, mobile_mote_id);

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

}

void MainWindow::tiger_out_of_range(void)
{

}
void MainWindow::lion_out_of_range(void)
{

}






