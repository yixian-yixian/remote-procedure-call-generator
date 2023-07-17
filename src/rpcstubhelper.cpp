// --------------------------------------------------------------
//
//                        rpcstubhelper.h
//
//        Author: Noah Mendelsohn         
//   
//
//        This file provides a helper routine to open a socket and put
//        it in a global variable where individual stub routines can find it.
//
//        OPERATION
//
//        Call rpcstubinitialize() to open the socket.
//        If there's a problem an exception will be thrown.
//
//        After calling rpcstubinitialize, call 
//        RPCSTUBSOCKET->accept() to accept a new connection,
//        RPCSTUBSOCKET-> read/write
//        when RPCSTUBSOCKET->eof goes true, then 
//        RPCSTUBSOCKET->closerpcstubaccept
//        to wait for a new incoming connection
//
//        All the operations listed above can be repeated from accept
//        to close, to process new connections.
//
//        LIMITATIONS
//
//              This version does not timeout 
//
//
//       Copyright: 2012 Noah Mendelsohn
//     
// --------------------------------------------------------------

#include "rpcstubhelper.h"
#include "serializer.h"

using namespace C150NETWORK;  // for all the comp150 utilities 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//    Global variable where proxies can find socket.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

C150StreamSocket *RPCSTUBSOCKET;
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//
//                rpcstubinitialize
//
//     Opens the socket and leaves it in global variable.
//     Put it in listening state.
//     Note that the socket call may throw an exception 
//     which is NOT caught here.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void rpcstubinitialize() {

  // create new socket
  c150debug->printf(C150RPCDEBUG,"rpcstubinitialize: Creating C150StreamSocket");
  RPCSTUBSOCKET = new C150StreamSocket();

  // Tell the OS to start allowing connections
  // The application will pick those up one at a time by doing accept calls
  RPCSTUBSOCKET -> listen();  
}


void 
send_int(int input)
{
    /* copy the integer binary representation into char buffer */
    char buf[4];
    bzero(buf, 4);
    memcpy((void *)buf, (void *)&input, sizeof(int));
    /* send an integer */
    RPCSTUBSOCKET->write(buf, 4);
}

void 
send_float(float input){
    char buf[4];
    bzero(buf, 4);
    memcpy((void *) buf, (void *) &input, sizeof(float));
    RPCSTUBSOCKET->write(buf, 4);
}

// pack string to a complete version with length 
void 
send_string(string input){
    string send_buf = serialize_string(input);
    RPCSTUBSOCKET->write(send_buf.c_str(), send_buf.size() + 1);
}

int get_string_size()
{
    bool found_end = false;
    int readlen = 0, totallen = 0;
    char *bufp = (char*) calloc(512, 1);
    while (!found_end) {
        readlen = RPCSTUBSOCKET->read(bufp, 1);  // read a byte
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
        throw C150Exception("struct.proxy.cpp: getfirstname() received invalid response from the client");
        return -1;    
    }
}

int 
recv_int(){
    char resBuf[4];
    bzero(resBuf, 4);
    ssize_t readlen = RPCSTUBSOCKET->read(resBuf, sizeof(int));
    if (readlen != sizeof(int)) throw C150Exception("recv_int(): received invalid response from the client");
    printf("stub.cpp: received %d \n", *(int *)resBuf);
    return *(int *)resBuf;
}

float 
recv_float(){
    char resBuf[4];
    bzero(resBuf, 4);
    ssize_t readlen = RPCSTUBSOCKET->read(resBuf, sizeof(float));
    if (readlen != sizeof(float)) throw C150Exception("recv_float(): received invalid response from the client");
    printf("stub.cpp: received %f \n", *(float *)resBuf);
    return *(float *)resBuf;
}

string 
recv_string(){
    /* read struct size from client */
    const int string_size = get_string_size();
    char complete_str[string_size];
    bzero(complete_str, string_size);
    ssize_t readlen = RPCSTUBSOCKET->read(complete_str, string_size);
    if (readlen != string_size) throw C150Exception("recv_string(): received invalid response from the client");
    printf("stub.cpp: received %s \n", string(complete_str).c_str());
    return string(complete_str);
}

void getFunctionNameFromStream(char *buffer, unsigned int bufSize) {
  unsigned int i;
  char *bufp;    // next char to read
  bool readnull;
  ssize_t readlen;             // amount of data read from socket
  
  //
  // Read a message from the stream
  // -1 in size below is to leave room for null
  //
  readnull = false;
  bufp = buffer;
  for (i=0; i< bufSize; i++) {
    readlen = RPCSTUBSOCKET-> read(bufp, 1);  // read a byte
    // check for eof or error
    if (readlen == 0) {
      break;
    }
    // check for null and bump buffer pointer
    if (*bufp++ == '\0') {
      readnull = true;
      break;
    }
  }
  
  //
  // With TCP streams, we should never get a 0 length read
  // except with timeouts (which we're not setting in pingstreamserver)
  // or EOF
  //
  if (readlen == 0) {
    c150debug->printf(C150RPCDEBUG,"simplefunction.stub: read zero length message, checking EOF");
    if (RPCSTUBSOCKET-> eof()) {
      c150debug->printf(C150RPCDEBUG,"simplefunction.stub: EOF signaled on input");

    } else {
      throw C150Exception("simplefunction.stub: unexpected zero length read without eof");
    }
  }

  //
  // If we didn't get a null, input message was poorly formatted
  //
  else if(!readnull) 
    throw C150Exception("simplefunction.stub: method name not null terminated or too long");
}

void __badFunction(char *functionName) {
  char doneBuffer[5] = "BAD";  // to write magic value DONE + null


  //
  // Send the response to the client indicating bad function
  //

  c150debug->printf(C150RPCDEBUG,"simplefunction.stub.cpp: received call for nonexistent function %s()",functionName);
  RPCSTUBSOCKET->write(doneBuffer, strlen(doneBuffer)+1);
}