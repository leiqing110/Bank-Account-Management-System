#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <iostream>
//#include <windows.h>
#include <winsock2.h>
//#include <Ws2spi.h>
//#include <Sporder.h>

#pragma comment(lib, "ws2_32.lib")

class Socket
{
public:
    Socket();
    ~Socket();

public:
//    static bool get_host_addr(char(& ip)[64], const char* name);
    static void fill_addr(struct sockaddr_in* sa, const char* ip, int port);
    SOCKET  createx();
    int     listenx(int port=0, const char* svc_addr="0.0.0.0");
    bool    connectx(const char* ip, int port);
    void    closex();
    SOCKET socketx() { return _sock; }

protected:
    SOCKET  _sock;
    int     _port;
};


#endif // SOCKET_H
