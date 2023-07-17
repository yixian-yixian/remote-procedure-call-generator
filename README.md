# RPC-generator
[Remote Procedure Call](https://en.wikipedia.org/wiki/Remote_procedure_call) is a software communication protocol that one program can use to request a service from a program located in another computer on a network without having to understand the network's details.

This project was to create both client-side and server-side stubs based on a provided IDL file (interface description language). The generated stubs files are written in C++ and designed to serve cross-platform. The current design supports transmission of basic data object in C++ such as 32-bit/64-bit variables, strings, and C++ class objects.  

## Usage 
```./rpcgenerate [provided-interface.idl] ```
You should expect to see these two files generated: 
- provided-interface.proxy.cpp 
- provided-interface.server.cpp

## Collaborator
[Jialin Zhang](https://github.com/jiake3109)


