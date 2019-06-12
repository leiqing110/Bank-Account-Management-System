#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "socket.h"
#include "server.h"

using namespace std;

static bool _running = true;
static vector<IncomeExpend *> clientInfos;
static HANDLE threadTcp;

typedef struct _client_msg_ {
    CLIENT_REQUEST request;
    char userName[50];
    char password[50];
    char cardNumber[50];
    unsigned int money;
}ClientMsg;

void printMain() {
    printf("********************************************\n");
    printf("*          欢迎来到银行账户管理系统          *\n");
    printf("* A.查看所有客户账号     B.增加客户账号      *\n");
    printf("* C.删除客户账号         D.为客户开银行账户  *\n");
    printf("* E.客户明细支出支入     F.保存客户明细      *\n");
    printf("* G.导出客户信息         H.导入客户信息     *\n");
    printf("*               Q.退出银行系统               *\n");
//    printf("*              Q.退出银行系统               *\n");
    printf("********************************************\n");
}

//返回值 1 查看所有客户账号     2.增加客户账号   3.删除客户账号
//     4.为客户开银行账户  5.客户明细支出支入  6.保存客户明细  9. 退出银行系统
int chooseMain() {
    while(true) {
        printMain();
        string readStr;
        getline(cin, readStr);
        if (readStr == "A" || readStr == "a") {
            return 1;
        } else if (readStr == "B" || readStr == "b") {
            return 2;
        } else if (readStr == "C" || readStr == "c") {
            return 3;
        } else if (readStr == "D" || readStr == "d") {
            return 4;
        } else if (readStr == "E" || readStr == "e") {
            return 5;
        } else if (readStr == "F" || readStr == "f") {
            return 6;
        } else if (readStr == "G" || readStr == "g") {
            return 7;
        } else if (readStr == "H" || readStr == "h") {
            return 8;
        } else if (readStr == "Q" || readStr == "q") {
            return 9;
        } else {
            cout << "输入错误，请重新输入" << endl;
        }
    }
}


void printAllClient() {
    printf("**********************************************************************************\n");
    printf("*                                 所有客户账号如下:                              *\n");
    for(unsigned int i = 0; i < clientInfos.size(); i++) {
        cout << "* 【" << i+1 << "】用户名:" << clientInfos[i]->getClientName() << " 密码:" << clientInfos[i]->getClientPassword() << endl;
    }
    printf("**********************************************************************************\n");
//    printf("********************************************\n");
//    printf("*  A.管理客户                B.返回上一级  *\n");
//    printf("********************************************\n");
}

void addClient() {          //新增客户
    while(true) {
        string clientName, password;
        cout << "请输入您想增加客户的用户名:";
        getline(cin, clientName);
        auto iter = clientInfos.begin();
        for (; iter != clientInfos.end(); iter++) {
            if ((*iter)->getClientName() == clientName)
                break;
        }
        if(iter != clientInfos.end()) {
            cout << "用户名已存在" << endl;
        } else {
            cout << "请输入新客户的密码:";
            getline(cin, password);
            IncomeExpend *pNewClient = new IncomeExpend(clientName, password);
            printf("**********************\n");
            printf("*    新增客户成功    *\n");
            printf("**********************\n");
            clientInfos.push_back(pNewClient);
            return;
        }
    }
}

void deleteClient() {       //删除客户
    while(true) {
        string clientName;
        cout << "请输入您想删除客户的用户名:";
        getline(cin, clientName);
        auto iter = clientInfos.begin();
        for (; iter != clientInfos.end(); iter++) {
            if ((*iter)->getClientName() == clientName)
                break;
        }
        if(iter == clientInfos.end()) {
            cout << "用户名不存在" << endl;
        } else {
            clientInfos.erase(iter);
            printf("**********************\n");
            printf("*    删除客户成功    *\n");
            printf("**********************\n");
            return;
        }
    }
}

void newCountForClient() {  //为已有客户开账户
    while(true) {
        string clientName, password;
        cout << "请输入您想开账户的客户用户名:";
        getline(cin, clientName);
        auto iter = clientInfos.begin();
        for (; iter != clientInfos.end(); iter++) {
            if ((*iter)->getClientName() == clientName)
                break;
        }
        if(iter == clientInfos.end()) {
            cout << "用户名不存在" << endl;
        } else {
            cout << "请输入客户密码:";
            getline(cin, password);
            if(password != (*iter)->getClientPassword()) {
                cout << "密码不正确" << endl;
            } else {
                auto & pCount = *iter;
                printf("************************\n");
                printf("*    请选择账户类型    *\n");
                printf("*  A.借记卡  B.信用卡  *\n");
                printf("************************\n");
                string chooseStr, cardNum;
                getline(cin, chooseStr);
                if (chooseStr == "A" || chooseStr == "a") {
                    pCount->openNewCard(DEBIT, cardNum);
                    printf("****************************************\n");
                    printf("*    新账户卡号:%s    *\n", cardNum.c_str());
                    printf("****************************************\n");
                } else if (chooseStr == "B" || chooseStr == "b") {
                    pCount->openNewCard(CREDIT, cardNum);
                    printf("****************************************\n");
                    printf("*    新账户卡号:%s    *\n", cardNum.c_str());
                    printf("****************************************\n");
                } else {
                    cout << "输入错误" << endl;
                }
                return;
            }
        }
    }
}

void getClientDetail() {       //获取客户账户下所有明细
    while(true) {
        string clientName;
        cout << "请输入您想获取客户明细的用户名:";
        getline(cin, clientName);
        auto iter = clientInfos.begin();
        for (; iter != clientInfos.end(); iter++) {
            if ((*iter)->getClientName() == clientName)
                break;
        }
        if(iter == clientInfos.end()) {
            cout << "用户名不存在" << endl;
        } else {
            string str;
            (*iter)->getDetails(str);
            cout << str << endl;
            return;
        }
    }
}

void saveClientDetail()         //导出明细
{
    while(true) {
        string clientName;
        cout << "请输入您想获取客户明细的用户名:";
        getline(cin, clientName);
        auto iter = clientInfos.begin();
        for (; iter != clientInfos.end(); iter++) {
            if ((*iter)->getClientName() == clientName)
                break;
        }
        if(iter == clientInfos.end()) {
            cout << "用户名不存在" << endl;
        } else {
            cout << "请输入您想要导出明细的文件名:";
            string fileStr;
            getline(cin, fileStr);
            string str;
            (*iter)->getDetails(str);
            if((*iter)->saveDetails(fileStr))
                cout << "导出明细成功" << endl;
            else
                cout << "导出明细失败" << endl;
            return;
        }
    }
}

bool saveClients(const string &fileName)
{
    ofstream out(fileName);
    if(!out) {
//        cout << "open " << saveFile << " error" << endl;
        return false;
    }
    string writeStr;
    for(const auto & ptr : clientInfos) {
        writeStr += "【用户信息】\n";
        writeStr += "用户名：" + ptr->getClientName() + "\t密码：" + ptr->getClientPassword() + "\n";
        writeStr += "\t借记卡：\n";
        for(const auto pDebit : ptr->getDebitInfo()){
            writeStr += "\t\t卡号：" + pDebit->getCardNum() + "\t余额：" + intToStr(pDebit->getBalance()) + "\t利息：" + intToStr(pDebit->getInterests()) + "\n";
        }
        writeStr += "\t信用卡：\n";
        for(const auto pCre : ptr->getCreditInfo()){
            writeStr += "\t\t卡号：" + pCre->getCardNum() + "\t余额：" + intToStr(pCre->getBalance()) + "\t可用额度：" +
                    intToStr(pCre->getEnableBalance()) + "\t固定额度：" + intToStr(pCre->getTotalBalance()) + "\n";
        }
    }
    if(writeStr.empty())
        return false;

    out.write(writeStr.c_str(), writeStr.size());
    return true;
}
#if 1
bool loadClients(const string &fileName)
{
    ifstream in(fileName);
    if(!in) {
        return false;
    }
    stringstream ss;
    ss << in.rdbuf();
    string readStr(ss.str());
    vector<string> Array;
    split(Array, readStr, "【用户信息】\n", false);
    for(const auto &cli : Array) {
        vector<string> arr;
        split(arr, cli, "\n", false);
        if(arr.size() < 3)
            continue;

        vector<string> tmp;
        split(tmp, arr[0], "\t", false);
        if(tmp.size() < 2)
            continue;
        string usr = "用户名：";
        auto pos = tmp[0].find(usr);
        if(pos == string::npos)
            continue;
        usr = tmp[0].substr(usr.size());
        string pass = "密码：";
        pos = tmp[1].find(pass);
        if(pos == string::npos)
            continue;
        pass = tmp[1].substr(pass.size());

//        static vector<IncomeExpend *> clientInfos;
        IncomeExpend * pCli = new IncomeExpend(usr, pass);
        clientInfos.push_back(pCli);

        for(const auto & card : arr) {
            string splitStr = "卡号";
            if(card.substr(0, splitStr.size()) != splitStr)
                continue;
            tmp.clear();
            split(tmp, card, "\t", false);
            if(tmp.size() == 3)
            {
                string cardNum = "卡号：", balance = "余额：", interest = "利息：";
                pos = tmp[0].find(cardNum);
                if(pos == string::npos)
                    continue;
                cardNum = tmp[0].substr(cardNum.size());

                pos = tmp[1].find(balance);
                if(pos == string::npos)
                    continue;
                balance = tmp[1].substr(balance.size());

                pos = tmp[2].find(interest);
                if(pos == string::npos)
                    continue;
                interest = tmp[2].substr(interest.size());
                DebitAccount * pDeb = new DebitAccount(cardNum, atoi(balance.c_str()), (unsigned int)atoi(interest.c_str()));
                auto &deVec = pCli->getDebitInfo();
                deVec.push_back(pDeb);
            }
            else if(tmp.size() == 4)
            {
                string cardNum = "卡号：", balance = "余额：", enable = "可用额度：", total = "固定额度：";
                pos = tmp[0].find(cardNum);
                if(pos == string::npos)
                    continue;
                cardNum = tmp[0].substr(cardNum.size());

                pos = tmp[1].find(balance);
                if(pos == string::npos)
                    continue;
                balance = tmp[1].substr(balance.size());

                pos = tmp[2].find(enable);
                if(pos == string::npos)
                    continue;
                enable = tmp[2].substr(enable.size());

                pos = tmp[3].find(total);
                if(pos == string::npos)
                    continue;
                total = tmp[3].substr(total.size());

                CreditAccount * pCre = new CreditAccount(cardNum, atoi(balance.c_str()), (unsigned int)atoi(enable.c_str()), (unsigned int)atoi(total.c_str()));
                auto &crVec = pCli->getCreditInfo();
                crVec.push_back(pCre);
            }
        }
    }
    return true;
}
#endif
// ret == 0 正常返回 == -1 关闭银行系统
int switchChoose(int ret)
{
    switch (ret)
    {
        case 1:
        {
            printAllClient();
            return 0;
        }
        case 2:
        {
            addClient();
            return 0;
        }
        case 3:
        {
            deleteClient();
            return 0;
        }
        case 4:
        {
            newCountForClient();
            return 0;
        }
        case 5:
        {
            getClientDetail();
            return 0;
        }
        case 6:
        {
            saveClientDetail();
            return 0;
        }
        case 7:
        {
            cout << "请输入要导出的文件名:";
            string fileName;
            getline(cin, fileName);
            trim(fileName);
            if(saveClients(fileName))
                cout << "导出客户信息成功" << endl;
            else
                cout << "导出客户信息失败" << endl;
            return 0;
        }
        case 8:
        {
            cout << "请输入要导入的文件名:";
            string fileName;
            getline(cin, fileName);
            trim(fileName);
            if(loadClients(fileName))
                cout << "导入客户信息成功" << endl;
            else
                cout << "导入客户信息失败" << endl;
            return 0;
        }
        case 9:
        default:
            _running = false;
            CloseHandle(threadTcp);
            WaitForSingleObject(threadTcp, INFINITE); // 等待线程退出
            return -1;   //关闭银行系统
    }
}

void sendMsg(string & sendStr, SOCKET clientSock)
{
    if(sendStr.empty())
        return;
    unsigned int msgLen = (unsigned int)sendStr.size();
    msgLen = htonl(msgLen);
    send(clientSock, (const char*)&msgLen, sizeof(msgLen), 0);
    send(clientSock, sendStr.c_str(), (int)sendStr.size(), 0);
}

void processMsg(ClientMsg * recvStr, SOCKET clientSock)
{
    CLIENT_REQUEST request = recvStr->request;
    string userName = recvStr->userName;
    string password = recvStr->password;
    string cardNum = recvStr->cardNumber;

    string sendStr;
    auto iter = clientInfos.begin();
    for(; iter != clientInfos.end(); iter++) {
        if((*iter)->getClientName() == userName)
            break;
    }
    if(iter == clientInfos.end()) {
        sendStr += "no";
        sendMsg(sendStr, clientSock);
        return;
    }
    switch (request)
    {
        case LOGIN:
        {
            sendStr += (*iter)->getClientPassword() == password ? "ok" : "no";
            sendMsg(sendStr, clientSock);
            break;
        }
        case GET_COUNT_INFO:    //获取账户信息
        {
            string str1, str2;
            (*iter)->getDebitStr(str1);
            (*iter)->getCreditStr(str2);
            sendStr += "ok" + str1 + str2;
            sendMsg(sendStr, clientSock);
            break;
        }
        case WITHDRAW_MONEY:    //取款
        {
            auto &dCards = (*iter)->getDebitInfo();
            auto &cCards = (*iter)->getCreditInfo();
            auto d_iter = dCards.begin();
            for(; d_iter != dCards.end(); d_iter++) {
                if((*d_iter)->getCardNum() == cardNum)
                {
                    sendStr += (*d_iter)->withdrawMoney(recvStr->money) ? "ok" : "no";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            auto c_iter = cCards.begin();
            for(; c_iter != cCards.end(); c_iter++) {
                if((*c_iter)->getCardNum() == cardNum)
                {
                    sendStr += (*c_iter)->withdrawMoney(recvStr->money) ? "ok" : "no";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            sendStr += "no";
            sendMsg(sendStr, clientSock);
            break;
        }
        case DEPOSIT_MONEY:     //存款
        {
            auto &dCards = (*iter)->getDebitInfo();
            auto &cCards = (*iter)->getCreditInfo();
            auto d_iter = dCards.begin();
            for(; d_iter != dCards.end(); d_iter++) {
                if((*d_iter)->getCardNum() == cardNum)
                {
                    (*d_iter)->depositMoney(recvStr->money);
                    sendStr += "ok";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            auto c_iter = cCards.begin();
            for(; c_iter != cCards.end(); c_iter++) {
                if((*c_iter)->getCardNum() == cardNum)
                {
                    (*c_iter)->depositMoney(recvStr->money);
                    sendStr += "ok";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            sendStr += "no";
            sendMsg(sendStr, clientSock);
            break;
        }
        case REFUND_MONEY:          //还款
        {
            auto &cCards = (*iter)->getCreditInfo();
            auto c_iter = cCards.begin();
            for(; c_iter != cCards.end(); c_iter++) {
                if((*c_iter)->getCardNum() == cardNum)
                {
                    (*c_iter)->refundMoney(recvStr->money);
                    sendStr += "ok";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            sendStr += "no";
            sendMsg(sendStr, clientSock);
            break;
        }
        case OVERDRAFT:         //透支
        {
            auto &cCards = (*iter)->getCreditInfo();
            auto c_iter = cCards.begin();
            for(; c_iter != cCards.end(); c_iter++) {
                if((*c_iter)->getCardNum() == cardNum)
                {
                    sendStr += (*c_iter)->overdraft(recvStr->money) ? "ok" : "no";
                    sendMsg(sendStr, clientSock);
                    break;
                }
            }

            sendStr += "no";
            sendMsg(sendStr, clientSock);
            break;
        }
        default:
            break;
    }
}

void closeSock(SOCKET &sock)
{
    if(INVALID_SOCKET != sock)
    {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
    }
    sock = INVALID_SOCKET;
}

DWORD WINAPI listenTcpThread(LPVOID lpThreadParameter)
{
    int m_nPort = 8081;

    Socket tcpSock;
    while (tcpSock.listenx(m_nPort) < 0 && _running)
    {
        Sleep(2000);
        printf("Listen %d failed, retry after 2 seconds...\n", m_nPort);
    }
//    printf("Listen %d success...\n", m_nPort);

    fd_set rfd;
    int selectResult;
    SOCKET clientsock;
    struct sockaddr_in client_addr;
//    struct timeval recvTimeout = {0, 100000};
    int recvTimeout = 1000;
    SOCKET sockfd = tcpSock.socketx();

    ClientMsg * recvBuf = new ClientMsg;
    while(_running)
    {
        FD_ZERO(&rfd);
        FD_SET(sockfd, &rfd);
        struct timeval selectTimeout={1,0};
        selectResult = select(0, &rfd, nullptr, nullptr, &selectTimeout);

        if(selectResult == 0)
            continue;
        else if(selectResult > 0)
        {
//            printf("select > 0\n");
            FD_CLR(sockfd, &rfd);
            int clientlen = sizeof(sockaddr_in);
            memset(&client_addr, 0, sizeof(client_addr));
            if ( (clientsock = accept(sockfd, (struct sockaddr *)&client_addr, &clientlen)) == INVALID_SOCKET )
            {
                printf("ACCEPT ERROR\n");
                break;
            }
//            printf("connect from [%s:%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            setsockopt(clientsock, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(int));

            unsigned int msg_len = 0;
            int ret = recv(clientsock, (char*)&msg_len, sizeof(msg_len), 0);
            if (ret != sizeof(msg_len))
            {
                continue;
            }
            msg_len = ntohl(msg_len);
//            printf("msgLen = %d\n", msg_len);

            if(msg_len != sizeof(ClientMsg))
            {
                printf("msgLen is invalid(%u)\n", msg_len);
                continue;
            }
            memset(recvBuf, 0, sizeof(ClientMsg));
            int result = 0;
            int recvlen = 0;
            while(recvlen < msg_len)
            {
                result = recv(clientsock,(char *)&recvBuf[recvlen], msg_len-recvlen, 0);
                if(result < 0)
                {
                    printf("recv return < 0\n");
                    closeSock(clientsock);
                    break;
                }
                recvlen += result;
            }

//            cout << "begin process msg" << endl;
            processMsg(recvBuf, clientsock);
            closeSock(clientsock);
        }
    }

    delete recvBuf;
    recvBuf = nullptr;
    return 0;
}

int main()
{
    threadTcp = CreateThread(NULL, 0, listenTcpThread, NULL, 0, NULL);
    int ret;
    while(true) {
        ret = chooseMain();
        ret = switchChoose(ret);
        if(-1 == ret)
            return 0;
    }
    return 0;
}
