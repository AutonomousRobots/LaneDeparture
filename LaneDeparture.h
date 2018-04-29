#ifndef LaneDeparture_H
#define LaneDeparture_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>


using namespace std;
using namespace cv;

class LaneDeparture{
public:
    LaneDeparture();
    ~LaneDeparture();
    
    string checkForLanes(Mat);

private:
    Mat element;
    int yHorizonLine;
    
    Mat blueFilter(const Mat&, Mat);
    Vec4i getBestLine(vector<Vec4i>, bool);
    int getAverageConvergenceXValue(vector<Vec4i>);
    Vec4i extendLine(Vec4i, int, float, int);
    int findHorizon(Vec4i, int); 
};

#endif
