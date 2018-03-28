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

int findHorizon(Vec4i line, int horizon) 
{
  if (line[1] < horizon)
    horizon = line[1];
  if (line[3] < horizon)
    horizon = line[3];
  return horizon;
}

Vec4i extendLine(Vec4i line, int horizon, float slope)
{
  Point topPoint, bottomPoint;
  if (line[1] > line[3])
  {
    bottomPoint = Point(line[0], line[1]);
    topPoint = Point(line[2], line[3]);
  }
  else
  {
    bottomPoint = Point(line[2], line[3]);
    topPoint = Point(line[0], line[1]);
  }

  int yDistanceToHorizon = topPoint.y - horizon;
  int xDistanceToHorizon = yDistanceToHorizon / slope;
  topPoint = Point((topPoint.x - xDistanceToHorizon), (horizon));

  return Vec4i(bottomPoint.x, bottomPoint.y, topPoint.x, topPoint.y);
}

int getAverageConvergenceXValue(vector<Vec4i> lines)
{
  int sum = 0;
  for(int i; i < lines.size(); i++)
  {
    Vec4i line = lines[i];
    sum += line[2];
  }
  return sum / lines.size();
}

Vec4i getBestLine(vector<Vec4i> lines)
{
  Point beginningPointsSum, endPointsSum;
  int numberOfGoodLines = 0;

  for(int j = 0; j < lines.size(); j++)
  {
    Vec4i currentLine = lines[j];
    if (currentLine[1] > 0)
    {
      beginningPointsSum += Point(currentLine[0], currentLine[1]);
      endPointsSum += Point(currentLine[2], currentLine[3]);
      numberOfGoodLines += 1;
    }
  }


  Point meanBeginningPoints = beginningPointsSum / numberOfGoodLines;
  Point meanEndPoints = endPointsSum / numberOfGoodLines;
  return Vec4i(meanBeginningPoints.x, meanBeginningPoints.y, meanEndPoints.x, meanEndPoints.y);
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
  int yHorizonLine = 9999;

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
    vector<Vec4i> rightLines, leftLines, allLines;
    
    for( size_t i = 0; i < lines.size(); i++ )
    {
      Vec4i l = lines[i];

      // find slope, sort into left and right sides
      float slope = ((float)l[3] - (float)l[1]) / ((float)l[2] - (float)l[0]);
      if (abs(slope) > 0.25 && abs(slope) < 0.99999) 
      {
        yHorizonLine = findHorizon(l, yHorizonLine);
        if (slope > 0)
        {
          l = extendLine(l, yHorizonLine, slope);
          rightLines.push_back(l);
          allLines.push_back(l);
        }
        else
        {
          l = extendLine(l, yHorizonLine, slope);
          leftLines.push_back(l);
          allLines.push_back(l);
        }
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        //cout << "here's a line: (" << l[0] << "," << l[1] << ")(" << l[2] << "," << l[3] << ")\n";
      }
    }

    // Vec4i bestRight, bestLeft = Vec4i(0,0,0,0);
    // if (rightLines.size() > 1) 
    //   bestRight = getBestLine(rightLines);
    // else if (rightLines.size() == 0)
    //   bestRight = rightLines[0];
    // else
    //   bestRight = bestRight;
    
    // if (leftLines.size() > 1)
    //   bestLeft = getBestLine(leftLines);
    // else if (rightLines.size() == 0)
    //   bestLeft = leftLines[0];
    // else
    //   bestLeft = bestLeft;

    if (!allLines.empty())
    {
      int convergance = getAverageConvergenceXValue(allLines);
      cout << "converge at = " << convergance << "\n";
    
      if (convergance > 200)
        cout << "departing to the left\n";
      if (convergance < 130)
        cout << "departing to the right\n";
    }
    // if (bestRight != Vec4i(0,0,0,0))
    // {
    //   line( cdst, Point(bestRight[0], bestRight[1]), Point(bestRight[2], bestRight[3]), Scalar(0,0,255), 3, CV_AA);
    //   cout << "here's a right line: (" << bestRight[0] << "," << bestRight[1] << ")(" << bestRight[2] << "," << bestRight[3] << ")\n";
    // }
    // if (bestLeft != Vec4i(0,0,0,0)){
    //   line( cdst, Point(bestLeft[0], bestLeft[1]), Point(bestLeft[2], bestLeft[3]), Scalar(0,0,255), 3, CV_AA);
    //   cout << "here's a left line: (" << bestLeft[0] << "," << bestLeft[1] << ")(" << bestLeft[2] << "," << bestLeft[3] << ")\n";
    // }

    imshow("detected lines", cdst);


  
    waitKey();
  }

  return 0;
}

// g++ hough.cpp `pkg-config --cflags --libs opencv`
// ./a.out ~/Downloads/dach-normal.avi 