#include "serializer.h"
using namespace std;

string serialize_string(string input)
{
    stringstream combo;
    int length = input.size() + 1; /* include null terminator */
    combo << to_string(length) << "/" << input;
    return combo.str();
}

string deserialize_string(char* contentbuf, int& skip_index)
{
    string temp(contentbuf);
    size_t first_idx = temp.find('/');
    string size_string = temp.substr(0, first_idx);
    int first_string_len = stoi(size_string);
    printf("size of string found is %d \n", first_string_len);
    string string_cont = temp.substr(first_idx, first_string_len);
    printf("actual string content is %s \n", string_cont.c_str());
    skip_index += size_string.size() + 1 + first_string_len;
    return string_cont;
}
