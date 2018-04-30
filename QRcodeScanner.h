#ifndef QRcodeScanner_H
#define QRcodeScanner_H

#include <opencv2/opencv.hpp>
#include <zbar.h>

using namespace cv;
using namespace std;
using namespace zbar;

class QRcodeScanner{
public:
    QRcodeScanner(){};
    ~QRcodeScanner(){};

    typedef struct
	{
	  string type;
	  string data;
	  vector <Point> location;
	} decodedObject;
    
    void decode(Mat&, vector<decodedObject>&);
	void display(Mat&, vector<decodedObject>&); 
};

#endif