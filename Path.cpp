#include "Path.h"

/*
* PATH.CPP
*
* Allows serialization of a series of path commands
*
* Creator: Toren Caldwell
*/


/*
* Default Constructor (Empty path)
*/
Path::Path(){

}


/*
* Constructor builds path from file
*/
Path::Path(string filename){
  ifstream in;
  in.open(filename);

  //Check if file is open
  if(!in.is_open()){
    cout << "Could not open file " << filename << endl;
  }else{
    string line;

    //Read file line by line
    while(getline(in, line)){
      int left, right;

      line.erase(0, 1);                   //Erase open parenth
      line.erase(line.size()-1, 1);       //Erase close parenth

      int comma = line.find_first_of(',', 0); //Let's play a game of find the comma

      left = stoi(line.substr(0, comma)); //Parse the left wheel
      line.erase(0, comma+1);             //Erase the left wheel and comma
      right = stoi(line);                 //Parse the right wheel

      push_back(left, right);             //Add command to the list
    }
    in.close();
  }
}


/*
* Destructor
*/
Path::~Path(){

}


/*
* Stores path in a text file
*/
void Path::sendToFile(string filename){
  ofstream out;
  out.open(filename);

  if(!out.is_open()){
    cout << "Could not open file " << filename << endl;
  }else{
    for(int i=0; i<size(); i++){
      out << get(i).toString << endl;
    }
    out.close();
  }
}


/*
* Add a command to the end of the path
*/
void Path::push_back(Command command){
  list.push_back(command);
}


/*
* Build a goto command and add it to the end of the path
*/
void Path::push_back(int left, int right){
  list.push_back(Path::new_command(left, right));
}


/*
* Delete a command from the end of the path
*/
void Path::pop_back(){
  list.pop_back();
}


/*
* Insert a command at the specified index
*/
void Path::insert(int index, Command command){
  list.insert(list.begin()+index, command);
}


/*
* Clear the entire path
*/
void Path::clear(){
  list.clear();
}


/*
* Remove a command at the specified index
*/
void Path::remove(int index){
  list.erase(list.begin()+index);
}


/*
* Returns the size of the path
*/
int Path::size(){
  return list.size();
}


/*
* Builds a new command from a left and right integer
*/
Command Path::new_command(int left, int right){
  Command command;
  command.left = left;
  command.right = right;

  stringstream s;
  s << "(" << left << "," << right << ")";
  command.toString = s.str();

  return command;
}


/*
* Returns the command at the specified index
*/
Command Path::get(int index){
  return list.at(index);
}


/*
* Returns the command at the specified index
*/
Command Path::operator[](int index){
  return get(index);
}
