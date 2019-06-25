#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
using namespace cv;
using namespace std;
Mat src, src_gray;
Mat dst, detected_edges, cpy;
int lowThreshold = 0;
const int max_lowThreshold = 500;
const int ratio = 3;
const int kernel_size = 3;
const char* window_name = "Edge Map";

static void repaint(int i, void*)
{
    cpy = src.clone();
    vector<Vec4i> linesP; // will hold the results of the detection
    HoughLinesP(detected_edges, linesP, 1, CV_PI/180, 50, lowThreshold, 10 ); // runs the actual detection
   for( size_t i = 0; i < linesP.size(); i++ )
    {
        Vec4i l = linesP[i];
        line( cpy, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
        cout << "Line:" << l[0] << ", " << l[1] << " => " << l[2] << ", " << l[3] << endl;
        cout << l << endl;
    }

    imshow( window_name, cpy );

}

int main( int argc, char** argv )
{
    CommandLineParser parser( argc, argv, "{@input | ../data/fruits.jpg | input image}" );
    src = imread( parser.get<String>( "@input" ), IMREAD_COLOR ); // Load an image
    if( src.empty() )
    {
        std::cout << "Could not open or find the image!\n" << std::endl;
        std::cout << "Usage: " << argv[0] << " <Input image>" << std::endl;
        return -1;
    }

    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    namedWindow( window_name, WINDOW_AUTOSIZE );
    blur( src_gray, detected_edges, Size(kernel_size,kernel_size) );

    lowThreshold = 80;
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
    dst = Scalar::all(0);
    src.copyTo( dst, detected_edges);

    imshow( "FOO DETECTED EDGES", detected_edges );

    createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, repaint );
    imshow( window_name, src );
    waitKey(0);
    return 0;
}
