#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"useranswer.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include<QList>
#include<QTableWidgetItem>
#include<QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:
    void deliverMat(Mat*);

private:
    Ui::MainWindow *ui;
    Mat src;

    Mat img_mat;
    QImage* qimg;
    QList<QTableWidgetItem*> qlst_table;
    double accurate;
};
#endif // MAINWINDOW_H
