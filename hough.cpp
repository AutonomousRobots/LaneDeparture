#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <raspicam/raspicam_cv.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "PiLo.h"
#include "Path.h"

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
  for(int i = 0; i < lines.size(); i++)
  {
    Vec4i line = lines[i];
    sum += line[2];
  }
  return sum / lines.size();
}

Vec4i getBestLine(vector<Vec4i> lines, bool rightLine)
{
  int numberOfGoodLines = 0;
  Vec4i bestLine = lines[0];

  for(int j = 0; j < lines.size(); j++)
  {
    Vec4i currentLine = lines[j];
    if (currentLine[1] > bestLine[1])
    {
        bestLine = currentLine;
    }
  }
  for(int j = 0; j < lines.size(); j++)
  {
    Vec4i currentLine = lines[j];
    if (!rightLine)
    {
        if (currentLine[1] == bestLine[1] && currentLine[0] > bestLine[0])
        {
            bestLine = currentLine;
        }
    } else {
        if (currentLine[1] == bestLine[1] && currentLine[0] < bestLine[0]) 
        {
            bestLine = currentLine;
        }
    }
  }

  return bestLine;
}


int main(int argc, char** argv)
{
  bool usingCamera;
  raspicam::RaspiCam_Cv camera;
  VideoCapture cap;
  TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
  Size subPixWinSize(10,10), winSize(31,31);

  cv::CommandLineParser parser(argc, argv, "{@input|0|}");
  string input = parser.get<string>("@input");
  
/*  if( input.size() == 1 && isdigit(input[0]) ) 
   {
      cap.open(input[0] - '0');
      usingCamera = false;
      if( !cap.isOpened())
      {
        cout << "Could not initialize capturing...\n";
        return 0;
      }
    }
  else 
  {
*/    
  usingCamera = true;
  camera.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
  if (!camera.open()) 
  {
    runtime_error tError("[Raspi_Capture] Camera failed to open");
  throw tError;
  }
//  }  
  PiLo pilo;

  if(!pilo.ok()){
    return 1;
  }
  
  
  namedWindow( "detected lines", 1 );
  Mat frame;
  int yHorizonLine = 9999;

  for(;;)
  {  
    
    if (usingCamera == false) 
    {
      cap >> frame;
    } else {
      camera.grab();
      camera.retrieve(frame);
    }
    if(frame.empty())
    {
        break;
    }

    // get frame dimentions
    Size size = frame.size();
    Mat src;
    Size newSize((size.width / 2), (size.height / 2));
    resize(frame, src, newSize);
    
    int xMidPoint = newSize.width / 2;
    int xRightEdge = newSize.width;
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
      if (abs(slope) != numeric_limits<float>::infinity()) 
      {
        yHorizonLine = 0;
        if (slope > 0)
        {
          l = extendLine(l, yHorizonLine, slope);
          rightLines.push_back(l);
        }
        else
        {
          l = extendLine(l, yHorizonLine, slope);
          leftLines.push_back(l);
        }
        //cout << "here's a line: (" << l[0] << "," << l[1] << ")(" << l[2] << "," << l[3] << ")\n";
      }
    }

    Vec4i bestRight, bestLeft;
    if (rightLines.size() > 0) 
    {
        bestRight = getBestLine(rightLines, true);
        allLines.push_back(bestRight);
        line( cdst, Point(bestRight[0], bestRight[1]), Point(bestRight[2], bestRight[3]), Scalar(0,0,255), 3, CV_AA);
    }
    if (leftLines.size() > 0)
    {
        bestLeft = getBestLine(leftLines, false);
        allLines.push_back(bestLeft);
        line( cdst, Point(bestLeft[0], bestLeft[1]), Point(bestLeft[2], bestLeft[3]), Scalar(0,0,255), 3, CV_AA);
    }
    
    if (!allLines.empty())
    {
      int convergance = getAverageConvergenceXValue(allLines);
      cout << "converge at = " << convergance << "\n";
    
      if (convergance > xMidPoint + 50)
      {
        cout << "departing to the left\n";
        pilo.sendCommand(pilo.TICKS, 2, -2); 
	  }
      else if (convergance < xMidPoint - 50)
      {
        cout << "departing to the right\n";
        pilo.sendCommand(pilo.TICKS, -2, 2); 
      }
      else 
      {
          pilo.sendCommand(pilo.TICKS, -20, -20); 
      }
    }

    imshow("detected lines", cdst);
    char c = (char)waitKey(1000);
    if( c == 27 )
       break;

  }
  camera.release();
  return 0;
}

// g++ hough.cpp -I/usr/local/include/ -L/opt/vc/lib -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui `pkg-config --cflags --libs opencv` -lwiringPi Path.cpp PiLo.cpp

// ./a.out ~/Downloads/dach-normal.avi 
