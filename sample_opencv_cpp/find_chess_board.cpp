#include "opencv2/opencv.hpp"
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;
int pattern_x = 3;
int pattern_y = 3;

#define CV_CheckType(type, x,y) if (!(x)) { printf("ERROR: " y "\n");}
class chessfield;
float innerAngle(Point p1, Point p2, Point c);
float innerAngle(float px1, float py1, float px2, float py2, float cx1, float cy1);
void mark_point(InputOutputArray& image, Point &pt, Scalar& color);
bool intersection(Point o1, Point p1, Point o2, Point p2, Point &r);
bool intersection(Point o1, Point p1, Point o2, Point p2);
int count_point(vector<chessfield> &f, Point &p);
chessfield* findChessFieldByPoints(vector<chessfield>& f, vector<Point> points, chessfield& exclude);

Scalar color_red(0, 0, 255, 0);
Scalar color_black(0, 0, 0, 0);
Scalar color_green(0,255, 0,0);
Scalar color_blue(255, 0, 0, 0);
Scalar color_light_blue(255, 255, 0, 0);


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

        float ax = points[0].x;
        float ay = points[0].y;
        float bx = points[1].x;
        float by = points[1].y;
        float cx = points[2].x;
        float cy = points[2].y;
        float dx = points[3].x;
        float dy = points[3].y;
        float t1_x = (ax+ bx+dx) /3;
        float t1_y = (ay+ by+dy) /3;

        float t2_x = (bx+ cx+dx) /3;
        float t2_y = (by+ cy+dy) /3;

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
        int cnt = 0;
        for(int i =0; i < 4; i++){
            for(int j=0; j < 4; j++){
                if(this->posAsArray()[i] == other.posAsArray()[j]){
                    cnt++;
                    break;
                }
            }
        }
        return cnt == 4;
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
            order[0] = i;
            Point prev = this->posAsArray()[(i+4-1) % 4];
            Point next = this->posAsArray()[(i+1) % 4];
            if (innerAngle(next, prev, p) > 0){
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
        putText(image, this->getName(),center, FONT_HERSHEY_SIMPLEX, 0.3 ,color_black,0.5,LINE_AA);
        mark_point(image, this->points[0], color_red);
        mark_point(image, this->points[1], color_green);
        //mark_point(image, this->points[2], color_blue);
        mark_point(image, this->points[3], color_light_blue);
    }


    chessfield* copy_left()
    {
        chessfield* cpy = new chessfield;
        *cpy = *this;

        Point direction = cpy->points[0] - cpy->points[1];
        Point newP0 = cpy->points[0] + direction;

        direction = cpy->points[3] - cpy->points[2];
        Point newP3 = cpy->points[3] + direction;

        cpy->points[1] = cpy->points[0];
        cpy->points[2] = cpy->points[3];
        cpy->points[0] = newP0;
        cpy->points[3] = newP3;
        cpy->pos_x = this->pos_x -1;
        return cpy;
    }

    chessfield* copy_top()
    {
        chessfield* cpy = new chessfield;
        *cpy = *this;

        Point direction = cpy->points[0] - cpy->points[3];
        Point newP0 = cpy->points[0] + direction;

        direction = cpy->points[1] - cpy->points[2];
        Point newP1 = cpy->points[1] + direction;

        cpy->points[2] = cpy->points[1];
        cpy->points[3] = cpy->points[0];
        cpy->points[0] = newP0;
        cpy->points[1] = newP1;
        cpy->pos_y = this->pos_y -1;
        return cpy;
    }

    chessfield* copy_down()
    {
        chessfield* cpy = new chessfield;
        *cpy = *this;

        Point direction = cpy->points[3] - cpy->points[0];
        Point newP3 = cpy->points[3] + direction;

        direction = cpy->points[2] - cpy->points[1];
        Point newP2 = cpy->points[2] + direction;

        cpy->points[0] = cpy->points[3];
        cpy->points[1] = cpy->points[2];
        cpy->points[2] = newP2;
        cpy->points[3] = newP3;
        cpy->pos_y = this->pos_y +1;
        return cpy;
    }

    chessfield* copy_right()
    {
        chessfield* cpy = new chessfield;
        *cpy = *this;

        Point direction = cpy->points[1] - cpy->points[0];
        Point newP1 = cpy->points[1] + direction;

        direction = cpy->points[2] - cpy->points[3];
        Point newP2 = cpy->points[2] + direction;

        cpy->points[0] = cpy->points[1];
        cpy->points[3] = cpy->points[2];
        cpy->points[1] = newP1;
        cpy->points[2] = newP2;
        cpy->pos_y = this->pos_y +1;
        return cpy;
    }

};

chessfield find_smallest(vector<chessfield>& f)
{
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

    for(Point p: smallest.posAsArray()){
        if (count_point(f, p) == 1){
            smallest.orderPointsFromUpperleft(p);
            break;
        }
    }

    return smallest;
}

void build_chess_field(chessfield* field[8][8], vector<chessfield>& f)
{
    chessfield smallest = find_smallest(f);
    for(int y =1; y < 7; y++){
        for(int x =1; x < 7; x++){
            if (x == 1 && y == 1){
                chessfield *cf = new chessfield();
                *cf = smallest;
                cf->pos_x = x;
                cf->pos_y = y;
                field[x][y] = cf;
            } else if (x == 1 && y > 1){
                chessfield *prev = field[x][y-1];
                vector<Point> searcher;
                searcher.push_back(prev->posAsArray()[2]);
                searcher.push_back(prev->posAsArray()[3]);
                chessfield* next = findChessFieldByPoints(f, searcher, *prev);

                if (!next){
                    printf("ERROR: missing next chess field\n");
                    exit(0);
                }
                chessfield *cf = new chessfield();
                *cf = *next;
                cf->pos_x = x;
                cf->pos_y = y;
                vector<Point> tmp = cf->posAsArray();
                cf->points.clear();
                for(int j=0; j < 4; j++){
                    if (tmp[j] == searcher[1]){
                        cf->points.push_back(tmp[j]);
                        tmp.erase(tmp.begin() + j);
                        break;
                    }
                }
                for(int j=0; j < 4; j++){
                    if (tmp[j] == searcher[0]){
                        cf->points.push_back(searcher[0]);
                        tmp.erase(tmp.begin() + j);
                        break;
                    }
                }
                if (!intersection(cf->points[0], tmp[1], cf->points[1], tmp[0])){
                    cf->points.push_back(tmp[0]);
                    cf->points.push_back(tmp[1]);
                } else {
                    cf->points.push_back(tmp[1]);
                    cf->points.push_back(tmp[0]);
                }
                field[x][y] = cf;

            } else if (x > 1){
                chessfield *prev = field[x-1][y];
                vector<Point> searcher;
                searcher.push_back(prev->posAsArray()[1]);
                searcher.push_back(prev->posAsArray()[2]);
                chessfield* next = findChessFieldByPoints(f, searcher, *prev);
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
                            tmp.erase(tmp.begin() + j);
                            break;
                        }
                    }
                    if (!intersection(searcher[0], tmp[0], searcher[1], tmp[1])){
                        cf->points.push_back(tmp[0]);
                        cf->points.push_back(tmp[1]);
                    } else {
                        cf->points.push_back(tmp[1]);
                        cf->points.push_back(tmp[0]);
                    }
                    cf->points.push_back(searcher[1]);
                    field[x][y] = cf;

                } else {
                    cout << "ERROR next not found " << endl;
                }
            }
        }
    }
}

double cross(Point v1,Point v2){
    return v1.x*v2.y - v1.y*v2.x;
}

bool onSegment(Point p, Point q, Point r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
       return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(Point p, Point q, Point r)
{
    // See 10th slides from following link for derivation of the formula
    // http://www.dcs.gla.ac.uk/~pat/52233/slides/Geometry1x1.pdf
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // colinear

    return (val > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(Point p1, Point q1, Point p2, Point q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

bool intersection(Point a1, Point a2, Point b1, Point b2, Point & intPnt){
    return doIntersect(a1, a2, b1, b2);
}

bool intersection(Point o1, Point p1, Point o2, Point p2)
{
    Point p;
    return intersection(o1, p1, o2, p2, p);
}


bool intersection(InputOutputArray& image, Point o1, Point p1, Point o2, Point p2)
{
    Point p;
    return intersection(o1, p1, o2, p2, p);
}

chessfield* findChessFieldByPoints(vector<chessfield>& f, vector<Point> points, chessfield& exclude){
    for(int i= 0; i < f.size(); i++){
        chessfield* fc = &f[i];
        if (fc->matchPoints(points).size() == points.size() &&
                !fc->equal(exclude)){
            return fc;
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

int count_point(vector<chessfield> &f, Point &p)
{
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

void mark_point(InputOutputArray& image, Point &pt, Scalar& color)
{

    const int shift = 0;
    const int radius = 4;
    const int r = radius*(1 << shift);

    int line_type = LINE_8;
    line(image, Point(pt.x - r, pt.y - r), Point( pt.x + r, pt.y + r), color, 1, line_type, shift);
    line(image, Point(pt.x - r, pt.y + r), Point( pt.x + r, pt.y - r), color, 1, line_type, shift);
    circle(image, pt, r+(1<<shift), color, 1, line_type, shift);
}

void mark_point(InputOutputArray& image, Point &pt)
{
    Scalar color(0, 255, 0,  0);
    mark_point(image, pt, color);
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


    /* Copy pices into own structure */
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

    chessfield* field[8][8] = {NULL};
    build_chess_field(field, f);


    /* Build the rest of the board based on the chessfield detection */
    for(int i = 1; i < 7; i++){
        field[0][i] = field[1][i]->copy_left();
    }

    for(int i = 1; i < 7; i++){
        field[7][i] = field[6][i]->copy_right();
    }

    for(int i = 1; i < 7; i++){
        field[i][0] = field[i][1]->copy_top();
    }

    for(int i = 1; i < 7; i++){
        field[i][7] = field[i][6]->copy_down();
    }

    field[0][0] = field[0][1]->copy_top();
    field[0][7] = field[0][6]->copy_down();
    field[7][0] = field[6][0]->copy_right();
    field[7][7] = field[6][7]->copy_right();

    for(int x=0; x < 8; x++){
        for(int y=0; y<8; y++){
            if (field[x][y] == NULL) continue;
            //cout << "Paint:" << Point(x,y) << endl;
            const int* line_color = &line_colors[(x+1*y+1) % line_max][0];
            Scalar color(line_color[0], line_color[1], line_color[2], line_color[3]);

            field[x][y]->paint(image, color);
        }
    }
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

