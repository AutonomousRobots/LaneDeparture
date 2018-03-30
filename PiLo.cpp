#include "PiLo.h"

/*
* PILO.CPP
*
* A library for interfacing a Raspberry Pi 3 with a parallax arlo (or ActivityBot)
*   via a 2-wire serial connection.
*
* Creator: Toren Caldwell
*/


/*
* Default Constructor
*/
PiLo::PiLo(){
  //Open the serial port to the activity board
  if((serial = serialOpen("/dev/ttyS0", 19200)) < 0){
    printf("Error: Could not open serial device\n");
    isSerialOpen = false;
  }else{
    isSerialOpen = true;
  }

  //Start wiringPi
  if(wiringPiSetup() == -1){
    printf("Error: Could not start WiringPi\n");
    wiringPiStarted = false;
  }else{
    wiringPiStarted = true;
  }

}


/*
* Destructor
*/
PiLo::~PiLo(){

}

/*
* Returns true if serial and wiringPi both started successfully
*/
bool PiLo::ok(){
  if(isSerialOpen && wiringPiStarted){
    return true;
  }
  return false;
}

/*
* sends a string over serial to the activity board
*/
void PiLo::sendLine(string input){
  string output = "";

  for(int i=0; i<input.length(); i++){
    output += input[i];
    if(output.length() == 8){             //Sends only 8 characters at a time
      serialPrintf(serial, output.c_str());
      serialFlush(serial);
      output.clear();

      usleep(5000);                      //Pause to allow serial flush
    }
  }
  serialPrintf(serial, output.c_str()); //Send the remaining string
  serialFlush(serial);
}


/*
* Sends a single command to the activity board
*/
string PiLo::sendCommand(int type, int left, int right){
  stringstream s;

  string cmd_base;

  cout << "Type = " << type << endl;

  if(type == 1){
    cmd_base = "GO:";
  }else{
    cmd_base = "MOVE:";
  }
  s << cmd_base << left << ":" << right << "\r";
  sendLine(s.str());

  return s.str();
}


/*
* Sends a list of path instructions
*/
bool PiLo::sendPath(Path path){

  for(int i=0; i<path.size(); i++){
    cout << sendCommand(2, path[i].left, path[i].right) << endl;
    while(!commandFinished());
  }

  return false;
}


/*
* Returns true upon receipt of the 'done' signal from the activity board
*/
bool PiLo::commandFinished(){
  string incoming;

  if(serialDataAvail(serial) > 0){
    while(serialDataAvail(serial) > 0){
      incoming += (char)serialGetchar(serial);
      //serialFlush(serial);
      usleep(2000);
    }
    cout << incoming << endl;
    if(incoming.compare("done") == 0){
      return true;
    }
  }
  return false;
}
