#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
int pattern_x = 3;
int pattern_y = 3;

#define CV_CheckType(type, x,y) if (!(x)) { printf("ERROR: " y "\n");}

class chessfield
{
public:
    Point a;
    Point b;
    int pos_x;
    int pos_y;
};

void my_drawChessboardCorners( InputOutputArray image, Size patternSize,
                                InputArray _corners,
                                bool patternWasFound )
{
    int type = image.type();
    int cn = CV_MAT_CN(type);
    CV_CheckType(type, cn == 1 || cn == 3 || cn == 4,
            "Number of channels must be 1, 3 or 4" );

    int depth = CV_MAT_DEPTH(type);
    CV_CheckType(type, depth == CV_8U || depth == CV_16U || depth == CV_32F,
            "Only 8-bit, 16-bit or floating-point 32-bit images are supported");

    if (_corners.empty())
        return;
    Mat corners = _corners.getMat();
    const Point2f* corners_data = corners.ptr<Point2f>(0);
    int nelems = corners.checkVector(2, CV_32F, true);
    CV_Assert(nelems >= 0);

    const int shift = 0;
    const int radius = 4;
    const int r = radius*(1 << shift);

    double scale = 1;
    switch (depth)
    {
    case CV_8U:
        scale = 1;
        break;
    case CV_16U:
        scale = 256;
        break;
    case CV_32F:
        scale = 1./255;
        break;
    }

    int line_type = (type == CV_8UC1 || type == CV_8UC3) ? LINE_AA : LINE_8;

    if (!patternWasFound){
        return;
    }
    const int line_max = 7;
    static const int line_colors[line_max][4] =
    {
        {0,0,255,0},
        {0,128,255,0},
        {0,200,200,0},
        {0,255,0,0},
        {200,200,0,0},
        {255,0,0,0},
        {255,0,255,0}
    };

    cv::Point2i prev_pt;
    for (int y = 0, i = 0; y < patternSize.height; y++)
    {
        const int* line_color = &line_colors[y % line_max][0];
        Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);
        if (cn == 1)
            color = Scalar::all(200);
        color *= scale;

        for (int x = 0; x < patternSize.width; x++, i++)
        {
            cv::Point2i pt(
                    cvRound(corners_data[i].x*(1 << shift)),
                    cvRound(corners_data[i].y*(1 << shift))
            );

            if (i != 0)
                line(image, prev_pt, pt, color, 1, line_type, shift);

//            line(image, Point(pt.x - r, pt.y - r), Point( pt.x + r, pt.y + r), color, 1, line_type, shift);
//            line(image, Point(pt.x - r, pt.y + r), Point( pt.x + r, pt.y - r), color, 1, line_type, shift);
//            circle(image, pt, r+(1<<shift), color, 1, line_type, shift);
            prev_pt = pt;
        }
        break;
    }

    chessfield fields[8][8];

    vector<chessfield> f;
    for (int y = 0, i = 0; y < patternSize.height-1; y++, i++){
        for (int x = 0; x < patternSize.width-1; x++, i++){
            Point a = corners_data[i];
            Point b = corners_data[i+patternSize.height + 1];
            chessfield cf;
            cf.a = a;
            cf.b = b;
            cf.pos_x = x + 1;
            cf.pos_y = y + 1;
            f.push_back(cf);
        }
    }

    int i = 0;
    for(chessfield fc: f){
        const int* line_color = &line_colors[i % line_max][0];
        Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);

        rectangle(image, fc.a , fc.b , color, 3);
        i++;
    }

/*    for (int y = 0, i = 0; y < patternSize.height; y++, i++){
        const int* line_color = &line_colors[i % line_max][0];
        Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);
        Point a = corners_data[i];
        Point b = corners_data[i+patternSize.height + 1];
        cout << i  << ":" << a << " " << b << endl;
        rectangle(image, a , b , color, 3);
    }
    */
}




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
        const char* window_name ="edges";
        cap >> frame; // get a new frame from camera
        cpy = frame.clone();
        cvtColor(frame, edges, COLOR_BGR2GRAY);
        //GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);

        Size patternsize(7,7); //interior number of corners
        Mat gray = edges; //source image
        vector<Point2f> corners; //this will be filled by the detected corners

        //CALIB_CB_FAST_CHECK saves a lot of time on images
        //that do not contain any chessboard corners
        bool patternfound = findChessboardCorners(gray, patternsize, corners,
                    CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
                    + CALIB_CB_FAST_CHECK);

        if(patternfound){
            putText(cpy,"Found",Point(0,25), FONT_HERSHEY_SIMPLEX, 1,Scalar(0,255,0),2,LINE_AA);
            //cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
            //        TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
        }else {
            putText(cpy,"Not found",Point(0,25), FONT_HERSHEY_SIMPLEX, 1,Scalar(0,0,255),2,LINE_AA);
        }

        my_drawChessboardCorners(cpy, patternsize, Mat(corners), patternfound);

        imshow("edges", cpy);
        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

