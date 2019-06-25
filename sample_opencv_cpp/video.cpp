#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    Mat edges;
    namedWindow("edges",1);
    for(;;)
    {
        Mat frame,cpy;
        cap >> frame; // get a new frame from camera
        cpy = frame.clone();
        cvtColor(frame, edges, COLOR_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);

        vector<Vec4i> linesP; // will hold the results of the detection
        HoughLinesP(edges, linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection
        for( size_t i = 0; i < linesP.size(); i++ )
        {
            Vec4i l = linesP[i];
            line( cpy, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
        }


        imshow("edges", cpy);
        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

