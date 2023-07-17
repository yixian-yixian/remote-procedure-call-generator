// --------------------------------------------------------------
//
//                        rpcproxyhelper.cpp
//
//        Author: Noah Mendelsohn         
//   
//
//        This file provides a helper routine to open a socket and put
//        it in a global variable where individual proxy routines can find it.
//
//        OPERATION
//
//        Call rpcproxyinitialize(servername) to open the socket.
//        If there's a problem an exception will be thrown.
//
//        LIMITATIONS
//
//              This version does not timeout 
//
//
//       Copyright: 2012 Noah Mendelsohn
//     
// --------------------------------------------------------------

#include "rpcproxyhelper.h"
#include "serializer.h"

using namespace C150NETWORK;  // for all the comp150 utilities 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//    Global variable where proxies can find socket.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

C150StreamSocket *RPCPROXYSOCKET;
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                rpcproxyinitialize
//
//     Opens the socket and leaves it in global variable.
//     Note that the socket call may throw an exception 
//     which is NOT caught here.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void rpcproxyinitialize(char *servername) {

  c150debug->printf(C150RPCDEBUG,"rpcproxyinitialize: Creating C150StreamSocket");
  RPCPROXYSOCKET = new C150StreamSocket();

  // Tell the Streamsocket which server to talk to
  // Note that the port number is defaulted according to
  // student logon by the COMP 150-IDS framework
  RPCPROXYSOCKET -> connect(servername);  
}


void 
send_int(int input)
{
    /* copy the integer binary representation into char buffer */
    char buf[4];
    bzero(buf, 4);
    memcpy((void *)buf, (void *)&input, sizeof(int));
    /* send an integer */
    RPCPROXYSOCKET->write(buf, 4);
}

void 
send_float(float input){
    char buf[4];
    bzero(buf, 4);
    memcpy((void *) buf, (void *) &input, sizeof(float));
    RPCPROXYSOCKET->write(buf, 4);
}

// pack string to a complete version with length 
void 
send_string(string input){
    string send_buf = serialize_string(input);
    RPCPROXYSOCKET->write(send_buf.c_str(), send_buf.size() + 1);
}

int get_string_size()
{
    bool found_end = false;
    int readlen = 0, totallen = 0;
    char *bufp = (char*) calloc(512, 1);
    while (!found_end) { // read a byte
        readlen = RPCPROXYSOCKET->read(bufp, 1);
        // check for eof or error
        if (readlen == 0) {
            break;
        }
        totallen += readlen;
        // check for null and bump buffer pointer
        if (*bufp++ == '/') {
            found_end = true;
        }
    }
    if (found_end) {
        string res(bufp-totallen);
        res = res.substr(0, totallen-1);
        return stoi(res);
    } else {
        throw C150Exception("proxy.cpp: getfirstname() received invalid response from the client");
        return -1;    
    }
}

int 
recv_int(){
    char resBuf[4];
    ssize_t readlen = RPCPROXYSOCKET->read(resBuf, sizeof(int));
    if (readlen != sizeof(int)) throw C150Exception("recv_int(): received invalid response from the client");
    printf("proxy.cpp: returned %d \n", *(int *)resBuf);
    return *(int *)resBuf;
}

float 
recv_float(){
    char resBuf[4];
    ssize_t readlen = RPCPROXYSOCKET->read(resBuf, sizeof(float));
    if (readlen != sizeof(float)) throw C150Exception("recv_float(): received invalid response from the client");
    printf("proxy.cpp: returned %f \n", *(float *)resBuf);
    return *(float *)resBuf;
}

string 
recv_string(){
    /* read struct size from client */
    const int string_size = get_string_size();
    char complete_str[string_size];
    bzero(complete_str, string_size);
    ssize_t readlen = RPCPROXYSOCKET->read(complete_str, string_size);
    if (readlen != string_size) throw C150Exception("recv_string(): received invalid response from the client");
    printf("proxy.cpp: returned %s \n", string(complete_str).c_str());
    return string(complete_str);
}

