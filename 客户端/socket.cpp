#include "socket.h"
#include <atlstr.h>
#include <Ws2tcpip.h>

Socket::Socket()
        : _sock(INVALID_SOCKET)
        , _port(0)
{

}

Socket::~Socket()
{
    closex();
};

void Socket::fill_addr(struct sockaddr_in* sa, const char* ip, int port)
{
    ZeroMemory(sa, sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    //sa->sin_addr.s_addr = inet_addr(ip);
	InetPton(AF_INET, CString(ip), &sa->sin_addr.s_addr);
    sa->sin_port = htons((unsigned short)port);
}

SOCKET Socket::createx()
{
    WORD sockVersion = MAKEWORD(2,2);   //调用Winsock版本2.2
    WSADATA wsaData;
    if ( 0 != WSAStartup(sockVersion, &wsaData) ) //对Winsock服务的初始化
    {
        printf("WSAStartup failed\n");
        return _sock;
    }

    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( INVALID_SOCKET==_sock )
    {
        int err = -1;
        err = WSAGetLastError();
        printf("Socket::create failed: %d\n", err);
    }

    return _sock;
}

int Socket::listenx(int port, const char* svc_addr)
{
    printf("try to listen %d\n", _port);
    if ( INVALID_SOCKET != _sock || INVALID_SOCKET != createx() )
    {
        _port = port;
        struct sockaddr_in service = {0};
        fill_addr(&service, svc_addr, _port);

        int on = 1;
        if ( setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0 ){
            perror("setsockopt");
        }

        if ( SOCKET_ERROR != bind(_sock, (struct sockaddr*)&service, sizeof(service))
             && (SOCKET_ERROR != listen(_sock, 8192)) )
        {
            printf("port = %d\n", _port);
        }
        else
        {
            _port = -1;
            int err = -1;
            err = WSAGetLastError();
            printf("Socket[%d]::listen failed: %d\n", _port, err);
            closex();
        }
    }
    else
    {
        _port = -1;
    }
    return _port;
}


bool Socket::connectx(const char* ip, int port)
{
    bool result = false;
    if ( INVALID_SOCKET!=_sock || INVALID_SOCKET!=createx() )
    {
        struct sockaddr_in r_addr = {0};
        fill_addr(&r_addr, ip, port);
        int err = connect(_sock, (sockaddr*)&r_addr, sizeof(r_addr));
        if ( 0 == err )
        {
//            printf("[unet] Socket[%d]::connect('%s', %d), successed.\n", _port, ip, port);
            result = true;
        }
        else
        {
            err = WSAGetLastError();
            printf("Socket[%d]::connect('%s', %d), failed: %d\n", _port, ip, port, err);
        }
    }
    return result;
}

void Socket::closex()
{
    if ( INVALID_SOCKET!=_sock )
    {
        shutdown(_sock, SD_BOTH);
        closesocket(_sock);
    }
    _sock = INVALID_SOCKET;
}
