#include "LaneDeparture.h"

LaneDeparture::LaneDeparture() {
    int yHorizonLine = 9999;
    Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
}

LaneDeparture::~LaneDeparture(){
    
}

string LaneDeparture::checkForLanes(Mat frame) {
    // get frame dimentions
    Size size = frame.size();
    Mat src;
    Size newSize((size.width / 2), (size.height / 2));
    resize(frame, src, newSize);
    
    int xMidPoint = newSize.width / 2;
    // prepare for Hough
    Mat hsvSrc;
    cvtColor(src, hsvSrc, CV_BGR2HSV);
    Mat blueSrc = blueFilter(hsvSrc, element);
    Mat dst, cdst;
    Canny(blueSrc, dst, 100, 500, 3);
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
    
    imshow("detected lines", cdst);

    if (!allLines.empty())
    {
      int convergance = getAverageConvergenceXValue(allLines);
      cout << "converge at = " << convergance << "\n";
    
      if (convergance > xMidPoint + 20 && convergance < xMidPoint +50)
      {
        cout << "departing to the left\n";
        return "left";
	  }
	  else if (convergance > xMidPoint + 50)
          return "hardLeft";
      else if (convergance < xMidPoint - 20 && convergance > xMidPoint -50)
      {
        cout << "departing to the right\n";
        return "right";
      }
      else if (convergance < xMidPoint -50)
          return "hardRight";
      else 
      {
          return "none";
      }
    }
    else
    {
        return "none";
    }
}

int LaneDeparture::findHorizon(Vec4i line, int horizon) 
{
  if (line[1] < horizon)
    horizon = line[1];
  if (line[3] < horizon)
    horizon = line[3];
  return horizon;
}

Vec4i LaneDeparture::extendLine(Vec4i line, int horizon, float slope)
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

int LaneDeparture::getAverageConvergenceXValue(vector<Vec4i> lines)
{
  int sum = 0;
  for(int i = 0; i < lines.size(); i++)
  {
    Vec4i line = lines[i];
    sum += line[2];
  }
  return sum / lines.size();
}

Vec4i LaneDeparture::getBestLine(vector<Vec4i> lines, bool rightLine)
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

Mat LaneDeparture::blueFilter(const Mat&src, Mat element)
{
    assert(src.type() == CV_8UC3);
    
    Mat blueOnly;
    inRange(src, Scalar(80,20,0), Scalar(150,255,230), blueOnly);
    
    bitwise_not(blueOnly, blueOnly);
    erode(blueOnly, blueOnly, Mat());
    dilate(blueOnly, blueOnly, element);
    
    return blueOnly;
}
