#ifndef USERANSWER_H
#define USERANSWER_H

#include <QObject>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include<vector>

using namespace std;
using namespace cv;

class UserAnswer : public QObject
{
    Q_OBJECT
public:
    UserAnswer();

    uchar getMedian(Mat& src);
    void getAnswerFromPoint(Point& p,int& question_index, int& answer);
    void getRedRegion(Mat* src);
    void getPolyRect(Mat* src);
    void getBinary(Mat& temp);
    void getAnswerArray(Mat& temp, vector<int>& answerArray);

    Mat rect;
    Mat redRegion;
    Mat rectTemp;
    vector<int> answerArray;

public slots:
    void getAnswer(Mat* src);

signals:
    void deliverArray(vector<int>* array);
    void deliverMat(Mat*);
};

#endif // USERANSWER_H
