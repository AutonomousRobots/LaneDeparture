#ifndef PILO_H
#define PILO_H

/*
* PILO.H
*
* A library for interfacing a Raspberry Pi 3 with a parallax arlo (or ActivityBot)
*   via a 2-wire serial connection.
*
* Creator: Toren Caldwell
*/


#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <sstream>
#include <unistd.h>
#include "Path.h"

#define MAX_SPEED 127          //Max speed value limit
#define MIN_SPEED 50

using namespace std;

class PiLo{
public:
  PiLo();                //Default Constructor
  ~PiLo();               //Destructor

  /*
  * Serial Operations
  */
  bool ok();              //Returns true if serial and wiringPi both started successfully
  void sendLine(string);  //Sends a string over serial to activity board
  string sendCommand(int, int, int);  //Sends a command
  bool sendPath(Path);    //Sends a list of path instructions

  const int SPEED = 1;
  const int TICKS = 2;

private:
  int serial;
  bool isSerialOpen, wiringPiStarted;

  bool commandFinished();  //Indicates when path command is finished executing
};


#endif
