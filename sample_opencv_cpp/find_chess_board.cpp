#include "opencv2/opencv.hpp"
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;
int pattern_x = 3;
int pattern_y = 3;

#define CV_CheckType(type, x,y) if (!(x)) { printf("ERROR: " y "\n");}
float innerAngle(Point p1, Point p2, Point c);
float innerAngle(float px1, float py1, float px2, float py2, float cx1, float cy1);

class chessfield
{
public:
    Point a;
    Point b;
    Point c;
    Point d;
    int pos_x;
    int pos_y;
    vector<Point> points;


    const vector<Point>& posAsArray()
    {
        if (this->points.size() == 0){
            points.push_back(a);
            points.push_back(b);
            points.push_back(c);
            points.push_back(d);

        }
        return points;
    }

    Point getCenter()
    {

        float t1_x = (a.x+ b.x+d.x) /3;
        float t1_y = (a.y+ b.y+d.y) /3;

        float t2_x = (b.x+ c.x+d.x) /3;
        float t2_y = (b.y+ c.y+d.y) /3;

        float x = (t1_x + t2_x) / 2;
        float y = (t1_y + t2_y) / 2;

        return Point(x,y);
    }

    string getName()
    {
        stringstream s;
        s << pos_x << ":" << pos_y;
        return s.str();
    }

    bool equal(chessfield& other)
    {
        for(int i =0; i < 4; i++){
            if(this->posAsArray()[i] != other.posAsArray()[i]){
                return false;
            }
        }
        return true;
    }


    vector<Point> matchPoints(chessfield& other)
    {
        return this->matchPoints(other.posAsArray());
    }

    vector<Point> matchPoints(const vector<Point>& other)
    {
        vector<Point> ret;
        for(int i =0; i < 4; i++){
            for(int j=0; j < 4; j++){
                if(this->posAsArray()[i] == other[j]){
                    ret.push_back(this->posAsArray()[i]);
                }
            }
        }
        return ret;
    }

    void orderPointsFromUpperleft(Point p){
        int order[4];
        for(int i = 0; i < 4; i++){
            if (p != this->posAsArray()[i]) continue;
            cout << "Pos of corner:" << i << endl;
            order[0] = i;
            Point prev = this->posAsArray()[(i+4-1) % 4];
            Point next = this->posAsArray()[(i+1) % 4];
            if (innerAngle(next, prev, p) < innerAngle( prev, next, p)){
                order[1] = (i+1) % 4;
                order[2] = (i+2) % 4;
                order[3] = (i+3) % 4;
            } else {
                order[1] = (i+4-1) % 4;
                order[2] = (i+4-2) % 4;
                order[3] = (i+4-3) % 4;
            }
            vector<Point> tmp_points = this->posAsArray();
            this->points.clear();
            for(int j = 0; j < 4; j++){
                this->points.push_back(tmp_points[order[j]]);
            }
            break;
        }
    }

    Point getSouth(Point p){
       for(int i = 0; i < 4; i++){
            if (p != this->posAsArray()[i]) continue;
            Point prev = this->posAsArray()[(i+4-1) % 4];
            Point next = this->posAsArray()[(i+1) % 4];
            if (prev.y - p.y < next.y-p.y){
                return next;
            }
            return prev;
        }
        return Point(0,0);
    }



    void paint(InputOutputArray& image, Scalar& color)
    {
        int lineType = LINE_8;
        Point points[1][4];
        for(int i = 0; i < 4; i++){
            points[0][i]  = this->posAsArray()[i];
        }
        const Point* ppt[1] = { points[0] };
        int npt[] = { 4 };

        Mat img = image.getMat();
        fillPoly(img, ppt, npt, 1, color, lineType);

        Point center = this->getCenter();
        putText(image, this->getName(),center, FONT_HERSHEY_SIMPLEX, 0.3 ,Scalar(0,0,0),0.5,LINE_AA);
    }



};

chessfield* findChessFieldByPoints(vector<chessfield> f, vector<Point> points){
    for(chessfield& fc: f){
        if (fc.matchPoints(points).size() == points.size()){
            return &fc;
        }
    }
    return NULL;
}

float innerAngle(Point p1, Point p2, Point c)
{
    return innerAngle(p1.x, p1.y, p2.x, p2.y, c.x, c.y);
}

float innerAngle(float px1, float py1, float px2, float py2, float cx1, float cy1)
{

    return atan2(py2,px2) - atan2(py1,px1);
}

int count_point(vector<chessfield> &f, Point &p){
    int count = 0;
    for(chessfield fc: f){
        for(Point point: fc.posAsArray()){
            if (point == p){
                count ++;
            }
        }
    }
    return count;
}

void mark_point(InputOutputArray& image, Point &pt){
  const int shift = 0;
    const int radius = 4;
    const int r = radius*(1 << shift);

    int line_type = LINE_8;
    Scalar color(0, 255, 0,  0);
          line(image, Point(pt.x - r, pt.y - r), Point( pt.x + r, pt.y + r), color, 1, line_type, shift);
          line(image, Point(pt.x - r, pt.y + r), Point( pt.x + r, pt.y - r), color, 1, line_type, shift);
          circle(image, pt, r+(1<<shift), color, 1, line_type, shift);
}

void create_extra_lines(InputOutputArray& image, chessfield& fc, int m)
{
    int line_type = LINE_8;
    Scalar color(0, 255, 0,  0);
    vector<Point> pos = fc.posAsArray();

    Point p = pos[(pos.size() + m -1) % pos.size()];
    Point n = pos[(pos.size() + m +1) % pos.size()];

    line(image, p, pos[m], color, 1, line_type);
    line(image, n, pos[m], color, 1, line_type);


}

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
    }


    vector<chessfield> f;
    for (int y = 0, i = 0; y < patternSize.height-1; y++, i++){
        for (int x = 0; x < patternSize.width-1; x++, i++){
            Point a = corners_data[i];
            Point b = corners_data[i + 1];
            Point c = corners_data[i+patternSize.height +1];
            Point d = corners_data[i+patternSize.height];
            chessfield cf;
            cf.a = a;
            cf.b = b;
            cf.c = c;
            cf.d = d;
            cf.pos_x = x + 1;
            cf.pos_y = y + 1;
            f.push_back(cf);

        }
    }

    int i = 0;
    for(chessfield fc: f){
        const int* line_color = &line_colors[i % line_max][0];
        Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);

        //rectangle(image, fc.a , fc.d , color, 3);


//fc.paint(image, color);
        i++;
    }


    chessfield smallest = f.front();
    Point smallest_p(99999,99999);
    Point zero (0,0);
    double dist = norm(smallest_p - zero);
    for(chessfield fc: f){
        vector<Point> pos = fc.posAsArray();

        for(Point p : fc.posAsArray()){
            double dist2 = norm(p - zero);
            if (dist > dist2 &&
                    ((fc.pos_x==6 && fc.pos_y == 6) ||
                     (fc.pos_x==1 && fc.pos_y == 1) ||
                     (fc.pos_x==6 && fc.pos_y == 1) ||
                     (fc.pos_x==1 && fc.pos_y == 6))){
                dist = dist2;
                smallest = fc;
            }
        }
    }
    Scalar b(0,0,0,0);
    smallest.paint(image,b);

    int m = 0;
    for(Point p: smallest.posAsArray()){
        if (count_point(f, p) == 1){
            cout << "Found corner point[" << m << "]" << p << endl;
            mark_point(image, p);
            //create_extra_lines(image, fc, m);

            Scalar color(0, 255, 0,  0);
            smallest.orderPointsFromUpperleft(p);
        }
        m++;
    }

    chessfield* field[8][8] = {NULL};
    for(int y =1; y < 2; y++){
        for(int x =1; x < 7; x++){
            if (x == 1 && y == 1){
                chessfield *cf = new chessfield();
                *cf = smallest;
                cf->pos_x = x;
                cf->pos_y = y;
                cout << "add " << x << ":" << y << endl;
                field[x][y] = cf;
            } else if (x > 1){
                cout << "add " << x << ":" << y << endl;
                chessfield *prev = field[x-1][y];
                vector<Point> searcher;
                searcher.push_back(prev->posAsArray()[1]);
                searcher.push_back(prev->posAsArray()[2]);
                chessfield* next = findChessFieldByPoints(f, searcher);
                if (next){
                    chessfield *cf = new chessfield();
                    *cf = *next;
                    cf->pos_x = x;
                    cf->pos_y = y;
                    vector<Point> tmp = cf->posAsArray();
                    cf->points.clear();
                    for(int j=0; j < 4; j++){
                        if (tmp[j] == searcher[0]){
                            cf->points.push_back(tmp[j]);
                            tmp.erase(tmp.begin() + j);
                            break;
                        }
                    }
                    for(int j=0; j < 4; j++){
                        if (tmp[j] == searcher[1]){
                            cf->points.push_back(tmp[j]);
                            tmp.erase(tmp.begin() + j);
                            break;
                        }
                    }
                    if (norm(cf->points[0]-tmp[0]) < norm(cf->points[0] - tmp[1])){
                        cf->points.push_back(tmp[0]);
                        cf->points.push_back(tmp[1]);
                    } else {
                        cf->points.push_back(tmp[1]);
                        cf->points.push_back(tmp[0]);
                    }



                } else {
                    cout << "ERROR next not found " << endl;
                }
            }
        }
    }

#if 0
    for(int i =0; i < 6 * 6; i++){
        if (!(f[i].pos_x == smallest.pos_x && f[i].pos_y == smallest.pos_y)){
            continue;
        }
        int x = 0, y = 0;
        for(int j =0; j < 6 * 6; j++){
            chessfield *fc = new chessfield();
            cout << "GET: "<< (i+j) % (6*6) << endl;
            *fc = f[(i+j) % (6*6)];
            cout << Point(fc->pos_x, fc->pos_y) << " -> "  << Point(x+1,y+1) << endl;
            fc->pos_x = x + 1;
            fc->pos_y = y+ 1;
            field[x+1][y+1]=fc;
            x++;
            if (x > 5){
                y = (y +1) % 5;
                x= 0;
            }
        }
    }
#endif
    for(int x=0; x < 8; x++){
        for(int y=0; y<8; y++){
            if (field[x][y] == NULL) continue;
            //cout << "Paint:" << Point(x,y) << endl;
            const int* line_color = &line_colors[(x*y) % line_max][0];
            Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);

            //field[x][y]->paint(image, color);
        }
    }

    //chessfield fc = f.front();
    //Point direction = fc.a - fc.b;
    //line(image, fc.a, fc.b + 2 * direction, Scalar(255, 0, 0, 0), 2);

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

