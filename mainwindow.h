#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QMessageBox>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QTimer>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTimer *tiger_Timer,*lion_Timer;

private:
    Ui::MainWindow *ui;
    QextSerialPort port;
    QMessageBox error;
    /*
     * static_mote_position -- This list contains all the Qpoints of static motes
     * routing_table_position -- This list contains all the Qpoints of respective routing table of each static mote
     * */
    QVector<QPoint> static_mote_position, routing_table_position;
    QStringList static_mote_names;// This list contains all the Mote numbers of the static motes
    /*
     * mote_ids -- This list contains the Mote ID's of all the static motes
     * received_list_tiger, received_list_lion -- These lists will hold the entire routing paths and
     * the sensor values of the respective tiger motes that has been sent information to the gateway.
     * */
    QStringList mote_ids, received_list_tiger, received_list_lion;
    /*
     * track_mote_neighbour_tiger, track_mote_neighbour_lion -- These QPoints contain the location of the static motes adjacent to the mobile motes
     * mobile_mote -- This Qpoint tracks the location of the mobile mote
     * mobile_mote_text -- This QPoint tracks the position of the text descriptor of the mobile mote.
     * draw_point1, draw_point2 -- These QPoints are used as reference positions for drawing a line between them to show
     * the routing path of the first mobile mote.
     * modified_mobile_point1, modified_mobile_point2 -- These QPoints are used as reference positions for drawing a line between
     * them to show the routing path of the second mobile mote.
     * */
    QPoint track_mote_neighbour_tiger, track_mote_neighbour_lion, mobile_mote, mobile_mote_text, draw_point1, draw_point2, modified_mobile_point1, modified_mobile_point2;
    QString mobile_mote_id_tiger, mobile_mote_id_lion;
    QTimer *timer;//This timer is used to call the paintevent method multiple times after each specified interval.

protected:
    void resizeEvent(QResizeEvent* evt) override;
    void paintEvent(QPaintEvent *e);
    void changeEvent(QEvent *e);

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_open_clicked();
    void receive();

public slots:
    void tiger_out_of_range();
    void lion_out_of_range();
};

#endif
