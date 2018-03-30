#ifndef PATH_H
#define PATH_H

/*
* PATH.H
*
* Allows serialization of a series of path commands
*
* Creator: Toren Caldwell
*/

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

struct Command{
  int left;
  int right;
  std::string toString;
};

class Path{
public:
  Path();                   //Defult constructor (empty path)
  Path(std::string);        //Load path from .txt file
  ~Path();                  //Destructor

  void sendToFile(std::string); //Store path in text file

  void push_back(Command);  //Add command to the end of the list
  void push_back(int, int); //Creates command to add to the list
  void pop_back();          //Pop from the back of the list

  void insert(int, Command);//Insert command at index
  void clear();             //Clear list
  void remove(int);         //Remove the command at index from list
  int  size();              //Returns the size of the path

  Command new_command(int, int);

  Command get(int);        //Returns command at index
  Command operator[](int); //Returns command at index

private:
  std::vector<Command> list;
};

#endif
