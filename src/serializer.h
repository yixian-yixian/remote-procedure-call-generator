#ifndef __SERIALIZER_H_INCLUDED__  
#define __SERIALIZER_H_INCLUDED__  

#include <string>
#include <sstream>
using namespace std;
string serialize_string(string input);
string deserialize_string(char* contentbuf, int& skip_index);

#endif