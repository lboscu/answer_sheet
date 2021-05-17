#include "useranswer.h"
#include<QDebug>


UserAnswer::UserAnswer()
{
    rect = Mat::zeros(116*4,98*4,CV_8UC3);
}

void UserAnswer::getAnswer(Mat* src)
{
    getRedRegion(src); //提取红色区域

    getPolyRect(src); //提取答案部分的矩形框，并进行透视变换

    getBinary(rect); //二值化矩形区域
    emit deliverMat(&rect);
//    imwrite("/home/lb/opencv/demo/anser_sheet/test.jpg",temp2);
//    imshow("temp2",temp2);

    getAnswerArray(rect,answerArray); // 返回答案数组

//    for (int i=0;i<answerArray.size();i++)
//    {
//        qDebug()<<answerArray[i];
//    }

    emit deliverArray(&answerArray);
}

uchar UserAnswer::getMedian(Mat& src)
{
    Mat tmp = src.reshape(1, 1);//make matrix new number of channels and new number of rows. here Put data: 1 row, all cols

    Mat sorted; //after sorted data

    cv::sort(tmp, sorted, CV_SORT_ASCENDING);

    int meddate = sorted.at<uchar>(sorted.cols / 2);//find median data in median of cols

    return meddate;
}

void UserAnswer::getAnswerFromPoint(Point& p,int& question_index, int& answer)
{
    int x = p.x;
    int y = p.y;
    if (y<30.5)
        answer = 0; //A
    for (int i=1;i<7;i++)
    {
        if (y>75*(i-1) && y<30.5+66*i)
            answer = 0;
    }

    for (int i=0;i<7;i++)
    {
        if (y>30.5+66*i && y<42+66*i)
            answer = 1; //B
    }

    for (int i=0;i<7;i++)
    {
        if (y>42+66*i && y<53.5+66*i)
            answer = 2; //C
    }

    for (int i=0;i<7;i++)
    {
        if (y>53.5+66*i && y<75+66*i)
            answer = 3; //D
    }

    int row_t = 0;
    if (y>75)
        row_t = int((y-75)/66) + 1;
    int col_tt = 0;
    if (x<97.5)
        col_tt = 0;
    else if (x<199.5)
        col_tt = 1;
    else if (x<298.5)
        col_tt = 2;
    else
        col_tt = 3;
//    int block_index = row_t*4+col_tt+1;


    question_index = row_t*4+col_tt+1;
//    assert(question_index<=block_index*5 && question_index>(block_index-1)*5);

}

void UserAnswer::getRedRegion(Mat* src)
{
    Mat temp;
    cv::bilateralFilter(*src,temp,35,2,2);
    redRegion = Mat::zeros(src->rows,src->cols,CV_8UC1);
    cvtColor(temp,temp,COLOR_BGR2HSV);
    for (int i=0;i<temp.rows;i++)
    {
        cv::Vec3b* data = temp.ptr<Vec3b>(i);
        uchar* data2 = redRegion.ptr<uchar>(i);
        for (int j=0;j<temp.cols;j++)
        {
            if (((data[j][0]>0 && data[j][0]<8) || (data[j][0]>160 && data[j][0]<180)))
            {
                data2[j] = 0;
            }
            else
                data2[j] = 255;
        }
    }
    redRegion = ~redRegion;
}

void UserAnswer::getPolyRect(Mat* src)
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(redRegion,contours,hierarchy,RETR_CCOMP, CHAIN_APPROX_SIMPLE,Point());

    int largest_area=0;

    int largest_contour_index=0;

    for(size_t i = 0; i< contours.size(); i++ )
    {
        double area = contourArea(contours[i]);
        if( area > largest_area )
        {
            largest_area = area;
            largest_contour_index = i;
        }
    }

    double len = cv::arcLength(contours[largest_contour_index],true);

    vector<Point2f> polys;
    cv::approxPolyDP(contours[largest_contour_index],polys,0.02*len,true);


    int max_val = -1;
    int max_index = -1;
    for (int i=0;i<4;i++)
    {
        int temp = polys[i].x + polys[i].y;
        if (max_val < temp)
        {
            max_val = temp;
            max_index = i;
        }
    }
    vector<Point2f> b_dsts;
    b_dsts.push_back(polys[(max_index-2)%4]);
    b_dsts.push_back(polys[(max_index-1)%4]);
    b_dsts.push_back(polys[(max_index-0)%4]);
    b_dsts.push_back(polys[(max_index+1)%4]);

    vector<Point2f> dsts;
    dsts.push_back(Point2f(0,0));
    dsts.push_back(Point2f(0,116*4));
    dsts.push_back(Point2f(98*4,116*4));
    dsts.push_back(Point2f(98*4,0));

    Mat H = cv::getPerspectiveTransform(b_dsts,dsts);
    cv::warpPerspective(*src,rect,H,Size(98*4,116*4));
}

void UserAnswer::getBinary(Mat& temp)
{
    cv::cvtColor(temp,temp,COLOR_BGR2GRAY);
    cv::threshold(temp,temp,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

//    findContours(temp5,contours2,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
//    drawContours(temp5,contours2,-1,Scalar(0,0,255),2);
    erode(temp,temp,cv::getStructuringElement(MORPH_RECT,Size(3,1)),Point(-1,-1),5);
    dilate(temp,temp,cv::getStructuringElement(MORPH_RECT,Size(3,1)),Point(-1,-1),5);
    erode(temp,temp,cv::getStructuringElement(MORPH_RECT,Size(3,3)),Point(-1,-1),1);
    dilate(temp,temp,cv::getStructuringElement(MORPH_RECT,Size(3,3)),Point(-1,-1),1);

    for (int i=0;i<10;i++)
    {
        uchar* data = temp.ptr<uchar>(i);
        for (int j=0;j<temp.cols;j++)
        {
            data[j] = 0;
        }
    }
    for (int i=temp.rows-1;i>temp.rows-5;i--)
    {
        uchar* data = temp.ptr<uchar>(i);
        for (int j=0;j<temp.cols;j++)
        {
            data[j] = 0;
        }
    }
}

void UserAnswer::getAnswerArray(Mat& binary, vector<int>& answerArray)
{
    vector<vector<Point>> contours2;
    vector<Vec4i> hierarchy2;

    findContours(binary,contours2,hierarchy2,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE,Point());
    vector<Point> select_point;
    vector<int> block_index;
    vector<int> answer_t;
    for (int i=0;i<contours2.size();i++)
    {
        Rect rect = boundingRect(contours2[i]);
        Point rect_center;
        rect_center.x = rect.x + cvRound(rect.width / 2);
        rect_center.y = rect.y + cvRound(rect.height / 2);
        int answer=-1;
        int index=-1;
        select_point.push_back(rect_center);
        getAnswerFromPoint(rect_center,index,answer);
        block_index.push_back(index);
        answer_t.push_back(answer);
//        qDebug()<<index<<answer;
    }
    int max_block = -1;
    for (int i=0;i<block_index.size();i++)
    {
        if (max_block < block_index[i])
            max_block = block_index[i];
    }
    const int len = select_point.size();
    int answer_array[len] = {0};
    for (int i=1;i<=max_block;i++)
    {
        vector<int> temp;
        for (int j=0;j<block_index.size();j++)
        {
            if (block_index[j] == i)
                temp.push_back(j);
        }
        for (int k = temp.size();k>0;k--)
            for (int j=0;j<k-1;j++)
                if (select_point[temp[j]].x > select_point[temp[j+1]].x)
                {
                    int t = temp[j];
                    temp[j] = temp[j+1];
                    temp[j+1] = t;
                }
//        for (int j=0;j<temp.size();j++)
//        {
//            qDebug()<<select_point[temp[j]];
//        }
        for (int j=0;j<temp.size();j++)
        {
            int q = (block_index[temp[j]] - 1)*5 + j;
            answer_array[q] = answer_t[temp[j]];
        }
    }
    answerArray.clear();
    for (int i=0;i<len;i++)
    {
        answerArray.push_back(answer_array[i]);
    }
}
