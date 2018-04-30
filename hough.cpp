#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <raspicam/raspicam_cv.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "PiLo.h"
#include "Path.h"
#include "LaneDeparture.h"
#include "QRcodeScanner.h"

#include <iostream>

using namespace cv;
using namespace std;



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
  camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
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
  QRcodeScanner scanner;
  LaneDeparture laneDetector;

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
    
    //QRcode landmark drive
    vector<QRcodeScanner::decodedObject> decodedObjects;
    scanner.decode(im, decodedObjects);

    if (decodedObjects.size() > 0){
        cout << "QRcode data : " << decodedObjects[0].data << endl;
        //do stuff
    }
    //LaneDetector Drive
    string directionOfDeparture = laneDetector.checkForLanes(frame);
    if (directionOfDeparture == "left")
        pilo.sendCommand(1, -5, -10); 
    else if (directionOfDeparture == "right")
        pilo.sendCommand(1, -10, -5);
    else if (directionOfDeparture == "hardLeft")
        pilo.sendCommand(1, 5, -5);
    else if (directionOfDeparture == "hardRight")
        pilo.sendCommand(1, -5, 5);
    else
        pilo.sendCommand(1, -20, -20);
    
    char c = (char)waitKey(100);
  }
  camera.release();
  return 0;
}

// g++ hough.cpp -I/usr/local/include/ -L/opt/vc/lib -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui `pkg-config --cflags --libs opencv` -lwiringPi -lzbar Path.cpp PiLo.cpp LaneDeparture.cpp QRcodeScanner.cpp

// ./a.out ~/Downloads/dach-normal.avi 
