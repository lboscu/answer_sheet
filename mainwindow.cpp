#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QThread>
#include<QDebug>
#include<QImage>
#include<QFileDialog>
#include<QMessageBox>

void mat2qimg(Mat &src,QImage &qimg);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(),this->height());
    qimg = new QImage;

    UserAnswer* userAnswer = new UserAnswer;
    QThread* myThread1 = new QThread;
    userAnswer->moveToThread(myThread1);
    myThread1->start();

    QLabel* status_label = new QLabel(this);
    ui->statusbar->addWidget(status_label);

    connect(ui->selectFile,&QPushButton::clicked,[=](){
        QString filename = QFileDialog::getOpenFileName(NULL,"答题卡","./");
        src = imread(filename.toUtf8().data());
        Mat temp = src.clone();
        cv::resize(temp,temp,Size(ui->label->width(),ui->label->height()));

        mat2qimg(temp,*qimg);
        ui->label->setPixmap(QPixmap::fromImage(*qimg));
        status_label->setText("答题卡加载完成......");
    });

    connect(ui->getStuAnswer,&QPushButton::clicked,this,[=](){
        emit deliverMat(&src);
    });
    connect(this,&MainWindow::deliverMat,userAnswer,&UserAnswer::getAnswer);


    connect(userAnswer,&UserAnswer::deliverMat,this,[=](Mat* img){
        img_mat = img->clone();
        mat2qimg(img_mat,*qimg);
        ui->label->setPixmap(QPixmap::fromImage(*qimg));
    });


    connect(userAnswer,&UserAnswer::deliverArray,this,[=](vector<int>* array){

        int len = array->size();
        ui->tableWidget->setRowCount((len / 10 + 1)*2);
        ui->tableWidget->setColumnCount(10);
        for (int i=0;i<(len/10 + 1)*2;i++)
        {
            ui->tableWidget->setRowHeight(i,25);
        }
        ui->tableWidget->setGeometry(390,90,570,(len / 10 + 1)*2*25);
        ui->tableWidget->updateGeometry();

        for (int i=0;i<len;i++)
        {
            QString ans;
            switch (array->at(i))
            {
            case 0:
                ans = "A";break;
            case 1:
                ans = "B";break;
            case 2:
                ans = "C";break;
            case 3:
                ans = "D";break;
            default:
                break;
            }

            QTableWidgetItem* item = new QTableWidgetItem(ans);
            QTableWidgetItem* index = new QTableWidgetItem(QString::number(i+1));
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackgroundColor(QColor("lightgray"));
            index->setTextAlignment(Qt::AlignCenter);
            index->setTextColor(QColor(200,111,100));
            index->setBackgroundColor(QColor("lightblue"));
            qlst_table.push_back(item);
            ui->tableWidget->setItem(i/10*2+1,i%10,item);
            ui->tableWidget->setItem(i/10*2,i%10,index);
        }
        status_label->setText("获取学生答案完成......");
    });

    connect(ui->calTrue,&QPushButton::clicked,[=](){
        QString trueAnswer = ui->lineEdit->text();
        QStringList qsl = trueAnswer.split(",");
        int len = qlst_table.size();
//        assert(len == qsl.size());
        if (len != qsl.size())
        {
            QMessageBox::warning(this,"提示","正确答案数量与学生答题数量不一致！");
            if (len > qsl.size())
                len = qsl.size();
        }

        accurate = 0;
        for (int i=0;i<len;i++)
        {
            if (qlst_table.at(i)->text() == qsl.at(i))
            {
                accurate += 1;
                qlst_table.at(i)->setTextColor(QColor("blue"));
            }
            else
                qlst_table.at(i)->setTextColor(QColor("red"));
        }
        ui->label_3->setText(QString("正确数：%1 ").arg(accurate));
        status_label->setText("正确数计算完成......");
    });

    connect(ui->actionselectFile,&QAction::triggered,ui->selectFile,&QPushButton::click);
    connect(ui->actionGetStuAns,&QAction::triggered,ui->getStuAnswer,&QPushButton::click);
    connect(ui->actionCalcu,&QAction::triggered,ui->calTrue,&QPushButton::click);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void mat2qimg(Mat &src,QImage &qimg)
{
    if (src.channels()==3)
    {
        cvtColor(src,src,CV_BGR2RGB);
        qimg = QImage((const unsigned char*)(src.data), src.cols, src.rows, src.cols*src.channels(),QImage::Format_RGB888 );
    }
    else
    {
        qimg = QImage((const unsigned char*)(src.data), src.cols, src.rows, src.cols*src.channels(), QImage::Format_Grayscale8 );
    }
    return;
}
