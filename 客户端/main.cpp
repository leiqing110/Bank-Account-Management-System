#include <iostream>
#include "socket.h"
#include <string>

using namespace std;

static bool _running = true;
static int m_nPort = 8081;
static const char * ip = "127.0.0.1";
static Socket tcpClientSock;
static string usrName, password;

typedef enum _em_client_request_ {
    LOGIN = 0x4567,
    GET_COUNT_INFO,     //获取账户信息
    WITHDRAW_MONEY,     //取款
    DEPOSIT_MONEY,      //存款
    REFUND_MONEY,       //还款
    OVERDRAFT,          //透支
    EXIT_SYSTEM,        //退出系統

    REQUEST_INVALID,    //非法请求
}CLIENT_REQUEST;


typedef struct _client_msg_ {
    CLIENT_REQUEST request;
    char userName[50];
    char password[50];
    char cardNumber[50];
    unsigned int money;
}ClientMsg;

void switchChoose(CLIENT_REQUEST ret);

const char * requestMap(CLIENT_REQUEST request)
{
    switch (request)
    {
        case LOGIN:
            return "登录";
        case GET_COUNT_INFO:
            return "获取账户信息";
        case WITHDRAW_MONEY:
            return "取款";
        case DEPOSIT_MONEY:
            return "存款";
        case REFUND_MONEY:
            return "还款";
        case OVERDRAFT:
            return "透支";
        case EXIT_SYSTEM:
            break;
    }
}

void printMain() {
    printf("********************************************\n");
    printf("*          欢迎来到银行客户系统             *\n");
    printf("* A.获取账户信息        B.取款             *\n");
    printf("* C.存款                D.还款             *\n");
    printf("* E.透支                Q.退出银行系统      *\n");
    printf("********************************************\n");
}


CLIENT_REQUEST chooseMain() {
    while(true) {
        CLIENT_REQUEST ret = REQUEST_INVALID;
        printMain();
        string readStr;
        getline(cin, readStr);
        if (readStr == "A" || readStr == "a") {
            ret = GET_COUNT_INFO;
        } else if (readStr == "B" || readStr == "b") {
            ret = WITHDRAW_MONEY;
        } else if (readStr == "C" || readStr == "c") {
            ret = DEPOSIT_MONEY;
        } else if (readStr == "D" || readStr == "d") {
            ret = REFUND_MONEY;
        } else if (readStr == "E" || readStr == "e") {
            ret = OVERDRAFT;
        } else if (readStr == "Q" || readStr == "q") {  //退出银行系统
            return EXIT_SYSTEM;
        } else {
            cout << "输入错误，请重新输入" << endl;
        }
        switchChoose(ret);
    }
}

void sendMsg(ClientMsg &msg)
{
    tcpClientSock.createx();
    if(!tcpClientSock.connectx(ip, m_nPort))
    {
//        printf("CONNECT FAILED(ip: %s, port: %d)\n", ip, m_nPort);
        tcpClientSock.closex();
        return;
    }

    SOCKET sockfd = tcpClientSock.socketx();
    unsigned int msgLen = sizeof(ClientMsg);
    msgLen = htonl(msgLen);
    send(sockfd, (const char*)&msgLen, sizeof(msgLen), 0);
    send(sockfd, (const char *)&msg, sizeof(ClientMsg), 0);
}

void recvMsg(string & str)
{
    str.clear();
    if(tcpClientSock.socketx() == INVALID_SOCKET)
        return;

    int recvTimeout = 1000;
    SOCKET sockfd = tcpClientSock.socketx();
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(int));

    char *recvBuf = new char[10 * 1024];
    memset(recvBuf, 0, 10*1024);
    unsigned int msg_len = 0;
    int ret = recv(sockfd, (char*)&msg_len, sizeof(msg_len), 0);
    if (ret != sizeof(msg_len)) {
        tcpClientSock.closex();
        return;
    }
    msg_len = ntohl(msg_len);
//    printf("msgLen = %d\n", msg_len);
    if(msg_len > 10*1024)
    {
        printf("msgLen is too long(%u > 10240)\n", msg_len);
        tcpClientSock.closex();
        return;
    }

    int result = 0;
    int recvlen = 0;
    while(recvlen < msg_len)
    {
        result = recv(sockfd, &recvBuf[recvlen], msg_len-recvlen, 0);
        if(result < 0)
        {
            printf("recv return < 0\n");
            tcpClientSock.closex();
            return;
        }
        recvlen += result;
    }
    str = recvBuf;
    tcpClientSock.closex();

    delete []recvBuf;
    recvBuf = nullptr;
}

//返回值 1: 登录成功  2: 登录失败
int login()
{
    usrName.clear();
    password.clear();
    printf("********************************************\n");
    printf("*          欢迎来到银行客户系统             *\n");
    printf("********************************************\n");
    printf("请输入登录用户名:");
    getline(cin, usrName);
    printf("请输入登录密码:");
    getline(cin, password);

    ClientMsg msg;
    memset(&msg, 0, sizeof(ClientMsg));
    msg.request = LOGIN;
    snprintf(msg.userName, sizeof(msg.userName), "%s", usrName.c_str());
    snprintf(msg.password, sizeof(msg.password), "%s", password.c_str());
    sendMsg(msg);
    string recvStr;
    recvMsg(recvStr);
    if(recvStr.substr(0, 2) == "ok")
        return 1;
    else
        return -1;
}

void switchChoose(CLIENT_REQUEST ret)
{
    ClientMsg msg;
    memset(&msg, 0, sizeof(ClientMsg));
    string recvStr;
    msg.request = ret;
    snprintf(msg.userName, sizeof(msg.userName), "%s", usrName.c_str());
    snprintf(msg.password, sizeof(msg.password), "%s", password.c_str());

    switch (ret)
    {
        case GET_COUNT_INFO:         //获取账户信息
        {
            msg.request = GET_COUNT_INFO;
            sendMsg(msg);
            recvMsg(recvStr);
            if(recvStr.substr(0, 2) == "ok") {
                recvStr = recvStr.substr(2);
                cout << recvStr << endl;
            }
            else
                cout << "【获取账户信息失败】" << endl;

            return;
        }
        case WITHDRAW_MONEY:        //取款
        case DEPOSIT_MONEY:         //存款
        case REFUND_MONEY:          //还款
        case OVERDRAFT:             //透支
        {
            string cinStr;
            printf("请输入%s卡号:", requestMap(msg.request));
            getline(cin, cinStr);
            snprintf(msg.cardNumber, sizeof(msg.cardNumber), "%s", cinStr.c_str());
            printf("请输入%s金额:", requestMap(msg.request));
            cinStr.clear();
            getline(cin, cinStr);
            msg.money = (unsigned int) atoi(cinStr.c_str());
            sendMsg(msg);
            recvMsg(recvStr);
            if (recvStr.size() >= 2 && recvStr.substr(0, 2) == "ok")
                printf("【%s成功】\n", requestMap(msg.request));
            else
                printf("【%s失败】\n", requestMap(msg.request));
            break;
        }
        default:
            break;
    }
}


int main()
{
    while(true) {
        int ret = login();
        if(ret == 1)    //登录成功
        {
            cout << "【登录成功】" << endl;
            chooseMain();
            tcpClientSock.closex();
            return 0;
        }
        else
            cout << "【登录失败】" << endl;
    }
    return 0;
}
