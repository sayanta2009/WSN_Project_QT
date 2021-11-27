#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QLabel>
#include <qdebug.h>

extern int x_gateway_mote1, x_mote2, x_mote3, x_mote4, x_mote5, x_mote6;
extern int y_gateway_mote1, y_mote2, y_mote3, y_mote4, y_mote5, y_mote6;
extern int x, y;
bool mote_detected_tiger = false, mote_detected_lion = false;
QTableWidget* table;

/*
 * This method has been used to initialise the positions of all static motes and allocating them their
 * names and ID#s to be displayed on the GUI. It also sets the GUI background and initialises the
 * three sensor LCD screens and progress bars. This method also initialises the two timers that are
 * meant to check after each fixed interval whether the mobile motes are getting out of boundary range.
 * */
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
    static_mote_position<<gatewayMote<<mote2<<mote3<<mote4<<mote5<<mote6;
    static_mote_names<<"2050(01) -- Gateway"<<"2093(02)"<<"2363(03)"<<"1972(04)"<<"2119(05)"<<"2080(06)";
    mote_ids<<"ef1c"<<"ed9c"<<"2dc1"<<"f442"<<"f46c"<<"ef31";

    routing_table_position << QPoint(gatewayMote.x(),gatewayMote.y()-225);
    routing_table_position << QPoint(mote2.x()-25,mote2.y()+125);
    routing_table_position << QPoint(mote3.x()-50,mote3.y()+150);
    routing_table_position << QPoint(mote4.x()+100,mote4.y()+100);
    routing_table_position << QPoint(mote5.x()-50,mote5.y()-200);
    routing_table_position << QPoint(mote6.x(),mote6.y()+125);

    ui->setupUi(this);
    QPixmap bkgnd(":/prefix_image/images/forest_pic.jpg");
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(palette);
    QPixmap project_logo(":/prefix_image/images/Project_logo_3.png");
    ui->logo->setPixmap(project_logo.scaled(400, 400, Qt::KeepAspectRatio));
    // Get all available COM Ports and store them in a QList.
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    ui->alarm_label->setText("<font color='red'>Alarm Box</font>");

    ui->label_temp->setText("<font color='white'>Temparature(°C)</font>");
    ui->label_heartbeat->setText("<font color='white'>Heartbeat(BPM)</font>");
    ui->label_battery->setText("<font color='white'>Battery(%)</font>");
    ui->label_tiger->setText("<font color='yellow'>Tiger1</font>");
    ui->label_lion->setText("<font color='yellow'>Tiger2</font>");

    ui->lcdNumber_battery_lion->setPalette(Qt::darkCyan);
    ui->lcdNumber_temp_lion->setPalette(Qt::darkCyan);
    ui->lcdNumber_heartbeat_lion->setPalette(Qt::darkCyan);
    ui->lcdNumber_battery_tiger->setPalette(Qt::darkCyan);
    ui->lcdNumber_temp_tiger->setPalette(Qt::darkCyan);
    ui->lcdNumber_heartbeat_tiger->setPalette(Qt::darkCyan);

    tiger_Timer = new QTimer(this);
    lion_Timer = new QTimer(this);

    connect(tiger_Timer, SIGNAL(timeout()),this, SLOT(tiger_out_of_range()));
    connect(lion_Timer, SIGNAL(timeout()),this, SLOT(lion_out_of_range()));

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

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(repaint()));
    timer->start(500);

}

/*
 * This method is basically used to resize the background image to full window size.
*/
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

/*
 * This method is called when the 'Open' button is clicked in the GUI
 * */
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

/*
 * This method is called when the 'Close' button is clicked in the GUI
 * */
void MainWindow::on_pushButton_close_clicked()
{
    if (port.isOpen())port.close();
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->comboBox_Interface->setEnabled(true);

    tiger_Timer->stop();
    lion_Timer->stop();
    mote_detected_tiger = false;
    mote_detected_lion = false;
    ui->textEdit_Status->clear();
    table->hide();
}

/*
 * This method receives all the serial informations coming from the Gateway static mote.
 */
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
                QStringList list = str.split("-");

                QString battery = list.at(0);
                QStringList battery_list = battery.split(":");
                double battery_value = battery_list.at(1).toDouble();
                battery_value = battery_value/37;

                QString temp = list.at(1);
                QStringList temp_list = temp.split(":");
                double temp_value = temp_list.at(1).toDouble();
                temp_value = temp_value/1000;

                QString heartbeat = list.at(2);
                QStringList heartbeat_list = heartbeat.split(":");
                double heartbeat_value = heartbeat_list.at(1).toDouble();

                ui->failure_box->clear();
                QFont font(ui->textEdit_Status->font());
                font.setBold(true);
                ui->textEdit_Status->setFont(font);
                ui->textEdit_Status->setTextColor(QColor("green"));

                if(list.at(3) == "ee66"){
                    ui->textEdit_Status->clear();
                    ui->textEdit_Status->append("Tiger ee66(2110) routing path found!!!!");
                    mobile_mote_id_tiger = "2110(07)";
                    ui->lcdNumber_battery_tiger->display(battery_value);
                    ui->lcdNumber_temp_tiger->display(temp_value);
                    ui->lcdNumber_heartbeat_tiger->display(heartbeat_value);

                    ui->progressBar_battery_tiger->setValue((int)battery_value);
                    ui->progressBar_temp_tiger->setValue((int)temp_value);
                    ui->progressBar_heartbeat_tiger->setValue((int)heartbeat_value);

                    ui->progressBar_battery_tiger->setMaximum(100);
                    ui->progressBar_temp_tiger->setMaximum(40);
                    ui->progressBar_heartbeat_tiger->setMaximum(70);

                    track_mote_neighbour_tiger = static_mote_position.at(mote_ids.indexOf(list.at(4)));
                    received_list_tiger = list;
                    tiger_Timer->start(120000);
                    mote_detected_tiger = true;

                }else if(list.at(3) == "ee65"){
                    ui->textEdit_Status->clear();
                    ui->textEdit_Status->append("Tiger ee65(2066) routing path found!!!!");
                    mobile_mote_id_lion = "2066(08)";
                    ui->lcdNumber_battery_lion->display(battery_value);
                    ui->lcdNumber_temp_lion->display(temp_value);
                    ui->lcdNumber_heartbeat_lion->display(heartbeat_value);

                    ui->progressBar_battery_lion->setValue((int)battery_value);
                    ui->progressBar_temp_lion->setValue((int)temp_value);
                    ui->progressBar_heartbeat_lion->setValue((int)heartbeat_value);

                    ui->progressBar_battery_lion->setMaximum(100);
                    ui->progressBar_temp_lion->setMaximum(40);
                    ui->progressBar_heartbeat_lion->setMaximum(70);

                    track_mote_neighbour_lion = static_mote_position.at(mote_ids.indexOf(list.at(4)));
                    received_list_lion = list;
                    lion_Timer->start(120000);
                    mote_detected_lion = true;
                }
                str = "";
            } else if(str.contains("$")){
                str.remove("\n", Qt::CaseSensitive);
                str.remove("$", Qt::CaseSensitive);
                QStringList list = str.split(":-");
                QPoint table_position = routing_table_position.at(mote_ids.indexOf(list.at(0)));
                QString table_data = list.at(1);
                QStringList table_row_data = table_data.split("-");
                int table_rows = table_row_data.size()/2;
                ui->textEdit_Status->clear();
                ui->textEdit_Status->insertPlainText("Routing Table Information of ");
                ui->textEdit_Status->insertPlainText(static_mote_names.at(mote_ids.indexOf(list.at(0))));
                ui->textEdit_Status->insertPlainText(" received!!!");
                table = new QTableWidget(this);
                //table->hide();
                const QSize* table_size = new QSize(120,160);
                const QRect* rect = new QRect(table_position, *table_size);
                table->setColumnCount(2);
                table->setRowCount(table_rows);

                QTableWidgetItem* header1 = new QTableWidgetItem("Nbr");
                table->setHorizontalHeaderItem(0, header1);
                QTableWidgetItem* header2 = new QTableWidgetItem("Hops");
                table->setHorizontalHeaderItem(1, header2);
                table->setColumnWidth(0, 50);
                table->setColumnWidth(1, 50);
                table->setGeometry(*rect);

                for(int i=0; i<table_rows; i++){
                    //static_mote_names.at(mote_ids.indexOf(table_row_data.at(2*i)))
                    QTableWidgetItem* neighbour = new QTableWidgetItem(static_mote_names.at(mote_ids.indexOf(table_row_data.at(2*i))));
                    table->setItem(i, 0, neighbour);
                    QTableWidgetItem* hop_count = new QTableWidgetItem(table_row_data.at(2*i+1));
                    table->setItem(i, 1, hop_count);
                }
                table->show();
                str = "";
            } else{
                str = "";
            }
        }
    }
}

/*
 * This method is used for all QT painting activities including the static and mobile motes.
 * */
void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.eraseRect(0,0,1920,1200);//to erase the painting activites in the entire screen
    QString mote_id;
    int index = 0, x_static_mote = 0, y_static_mote = 0, x_mobile_mote = 0, y_mobile_mote = 0;
    /*
     * Image customisation for the static abd dynamic motes
     * */
    QImage static_mote_image(":/prefix_image/images/static_mote_2.png");
    QImage mobile_mote_image(":/prefix_image/images/tiger.png");
    QImage mobile_mote_image_second(":/prefix_image/images/lion.png");

    QImage static_mote_image_resized = static_mote_image.scaled(50, 50, Qt::KeepAspectRatio);
    QImage mobile_mote_image_resized = mobile_mote_image.scaled(100, 100, Qt::KeepAspectRatio);
    QImage mobile_mote_image_second_resized = mobile_mote_image_second.scaled(100, 100, Qt::KeepAspectRatio);

    /*
     * Static mote painting
     * */
    QPen blue_pen(Qt::blue);
    blue_pen.setWidth(6);
    painter.setPen(blue_pen);
    foreach(QPoint point, static_mote_position){
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

    /*
     * Dynamic mote and routing path painting
     * */
    QPen green_pen(Qt::green);
    green_pen.setWidth(6);
    painter.setPen(green_pen);

    if(mote_detected_tiger){
        if(!received_list_tiger.empty()){
            x_mobile_mote = track_mote_neighbour_tiger.x() + 150;
            y_mobile_mote = track_mote_neighbour_tiger.y() + 25;
            mobile_mote.setX(x_mobile_mote);
            mobile_mote.setY(y_mobile_mote);
            painter.drawImage(mobile_mote, mobile_mote_image_resized);
            x_mobile_mote = mobile_mote.x() - 20;
            y_mobile_mote = mobile_mote.y() + 100;
            mobile_mote_text.setX(x_mobile_mote);
            mobile_mote_text.setY(y_mobile_mote);
            painter.drawText(mobile_mote_text, mobile_mote_id_tiger);

            QPen black_pen(Qt::black);
            black_pen.setWidth(3);
            painter.setPen(black_pen);
            for(int i=3; i<received_list_tiger.size()-1; i++){
                if(i == 3){
                    draw_point1 = mobile_mote;
                }else{
                    draw_point1 = static_mote_position.at(mote_ids.indexOf(received_list_tiger.at(i)));
                }
                draw_point2 = static_mote_position.at(mote_ids.indexOf(received_list_tiger.at(i+1)));
                painter.drawLine(draw_point1, draw_point2);
            }
        }
    }

    QPen dark_green_pen(Qt::green);
    dark_green_pen.setWidth(6);
    painter.setPen(dark_green_pen);

    if(mote_detected_lion){
        if(!received_list_lion.empty()){
            x_mobile_mote = track_mote_neighbour_lion.x() - 150;
            y_mobile_mote = track_mote_neighbour_lion.y() + 25;
            mobile_mote.setX(x_mobile_mote);
            mobile_mote.setY(y_mobile_mote);
            painter.drawImage(mobile_mote, mobile_mote_image_second_resized);
            x_mobile_mote = mobile_mote.x() - 20;
            y_mobile_mote = mobile_mote.y() - 40;
            mobile_mote_text.setX(x_mobile_mote);
            mobile_mote_text.setY(y_mobile_mote);
            painter.drawText(mobile_mote_text, mobile_mote_id_lion);

            QPen red_pen(Qt::red);
            red_pen.setWidth(3);
            painter.setPen(red_pen);
            for(int i=3; i<received_list_lion.size()-1; i++){
                if(i == 3){
                    draw_point1 = mobile_mote;
                }else{
                    draw_point1 = static_mote_position.at(mote_ids.indexOf(received_list_lion.at(i)));
                }
                modified_mobile_point1.setX(draw_point1.x() + 20);
                modified_mobile_point1.setY(draw_point1.y() + 20);
                draw_point2 = static_mote_position.at(mote_ids.indexOf(received_list_lion.at(i+1)));
                modified_mobile_point2.setX(draw_point2.x() - 20);
                modified_mobile_point2.setY(draw_point2.y() + 20);
                painter.drawLine(modified_mobile_point1, modified_mobile_point2);
            }
        }
    }
}

/*
 *This method is called when one of the two mobile motes is not transmitting anymore
 * to any of the static motes waiting an interval of 25 secs.
 * */
void MainWindow::tiger_out_of_range(void){
    QFont font(ui->failure_box->font());
    font.setBold(true);
    ui->failure_box->clear();
    ui->failure_box->setFont(font);
    ui->failure_box->setTextColor(QColor("red"));
    ui->failure_box->append("Caution!!!! Tiger ee66(2110) Lost Out of Range!!!! ");
    mote_detected_tiger = false;
}

/*
 *This method is called when the other mobile mote is not transmitting anymore
 * to any of the static motes waiting an interval of 25 secs.
 * */
void MainWindow::lion_out_of_range(void){
    QFont font(ui->failure_box->font());
    font.setBold(true);
    ui->failure_box->clear();
    ui->failure_box->setFont(font);
    ui->failure_box->setTextColor(QColor("red"));
    ui->failure_box->append("Caution!!!! Tiger ee65(2066) Lost Out of Range!!!! ");
    mote_detected_lion = false;
}
