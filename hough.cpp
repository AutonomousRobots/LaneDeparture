#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

void help()
{
  cout << "\nThis program demonstrates line finding with the Hough transform.\n"
         "Usage:\n"
         "./houghlines <image_name>, Default is pic1.jpg\n" << endl;
}

int main(int argc, char** argv)
{
  VideoCapture cap;
  TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
  Size subPixWinSize(10,10), winSize(31,31);

  cv::CommandLineParser parser(argc, argv, "{@input|0|}");
  string input = parser.get<string>("@input");
  
  if( input.size() == 1 && isdigit(input[0]) )
    cap.open(input[0] - '0');
  else
    cap.open(input);
  if( !cap.isOpened() )
  {
    cout << "Could not initialize capturing...\n";
    return 0;
  }
  
  namedWindow( "detected lines", 1 );
  Mat src;

  for(;;)
  {  
    cap >> src;
    if(src.empty())
    {
        break;
    }

    // get frame dimentions
    Size size = src.size();
    int xMidPoint = size.width / 2;
    int xRightEdge = size.width;
    int xLeftEdge = 0;
    
    // prepare for Hough
    Mat dst, cdst;
    Canny(src, dst, 50, 200, 3);
    cvtColor(dst, cdst, CV_GRAY2BGR);

    // do the Hough
    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
    
    // process each line, draw on cdst and find right and left lanes
    vector<Vec4i> rightLines, leftLines;
    for( size_t i = 0; i < lines.size(); i++ )
    {
      Vec4i l = lines[i];

      // find slope, sort into left and right sides
      float slope = ((float)l[3] - (float)l[1]) / ((float)l[2] - (float)l[0]);
      if (abs(slope) > 0.5)
      {
        if (slope <= 0)
          rightLines.push_back(l);
        else
          leftLines.push_back(l);

        // draw lines on cdst
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
      }
      //cout << "here's a line: (" << l[0] << "," << l[1] << ")(" << l[3] << "," << l[4] << ")\n";
    }
    imshow("detected lines", cdst);
  
    waitKey();
  }

  return 0;
}