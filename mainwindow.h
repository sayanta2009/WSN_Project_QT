#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QMessageBox>
#include "qextserialport.h"
#include "qextserialenumerator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QextSerialPort port;
    QMessageBox error;
    QVector<QPoint> staticMotes;
    QStringList static_mote_names;
    QStringList mote_ids, received_list;
    QPoint track_mote_neighbour, mobile_mote, mobile_mote_text, draw_point1, draw_point2;
    QString mobile_mote_id;

protected:
    void resizeEvent(QResizeEvent* evt) override;
    void paintEvent(QPaintEvent *e);
    void changeEvent(QEvent *e);

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_open_clicked();
    void receive();
};

#endif // MAINWINDOW_H
