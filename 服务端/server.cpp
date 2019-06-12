#include "server.h"
#include <cstring>
#include <string>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <functional>
using namespace std;

void getLocalTime(std::string &timeStr) {
    time_t time_seconds = time(nullptr);
    struct tm now_time;
    localtime_s(&now_time, &time_seconds);
    char buf[100] = {0};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", now_time.tm_year + 1900, now_time.tm_mon + 1,
            now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);

    timeStr.clear();
    timeStr = buf;
}

// 账户类
Account::Account(std::string _cardNumber, int _balance)
    : balance(_balance)
    , cardNumber(_cardNumber)
{

}

Account::Account()
    : balance(0)
{
    //获取随机卡号        //todo 未做防重处理
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<> first(100000000, 999999999);
    std::uniform_int_distribution<> second(1000000000, 0x7fffffff);
    auto firstNum = std::bind(first, engine);
    auto secondNum = std::bind(second, engine);

    cardNumber.clear();
    char buf[30] = {0};
    snprintf(buf, sizeof(buf), "%d", firstNum());
    cardNumber += buf;
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d", secondNum());
    cardNumber += buf;
}

int Account::getBalance() {
    return balance;
}

const std::string &Account::getCardNum() {
    return cardNumber;
}

//存款
void Account::depositMoney(unsigned int money) {
    balance += money;

    InExInfo info;
    info.type = INCOME;
    info.money = money;
    info.restMoney = balance;
    getLocalTime(info.time);

    withdrawAndDeposit.push_back(info);
}
//取款
bool Account::withdrawMoney(unsigned int money) {
    if(money > balance) {
//        cout << "余额不足" << endl;
        return false;
    }
    balance -= money;

    InExInfo info;
    info.type = EXPEND;
    info.money = money;
    info.restMoney = balance;
    getLocalTime(info.time);

    withdrawAndDeposit.push_back(info);
    return true;
}

const std::vector<InExInfo> &Account::getWithDrawDeposit()
{
    return withdrawAndDeposit;
}

//借记卡
DebitAccount::DebitAccount()
    : interests(0)
{

}

DebitAccount::DebitAccount(std::string _cardNumber, int _balance, unsigned int _intersts)
    : Account(_cardNumber, _balance)
    , interests(_intersts)
{

}

unsigned int DebitAccount::getInterests() {
    return interests;
}

//信用卡
CreditAccount::CreditAccount()
        : enableBalance(10000)  //默认信用额度是10000元
        , totalBalance(10000)
{

}

CreditAccount::CreditAccount(std::string _cardNumber, int _balance, unsigned int _enableBalance, unsigned int _totalBalance)
        : Account(_cardNumber, _balance)
        , enableBalance(_enableBalance)
        , totalBalance(_totalBalance)
{

}

//还款
void CreditAccount::refundMoney(unsigned int money) {
    enableBalance += money;

    InExInfo info;
    info.type = INCOME;
    info.money = money;
    info.restMoney = enableBalance;
    getLocalTime(info.time);

    refundAndOverdraft.push_back(info);
}
//透支
bool CreditAccount::overdraft(unsigned int money) {
    if(money > enableBalance) {
//        cout << "信用卡可用额度不足" << endl;
        return false;
    }
    enableBalance -= money;

    InExInfo info;
    info.type = EXPEND;
    info.money = money;
    info.restMoney = enableBalance;
    getLocalTime(info.time);

    refundAndOverdraft.push_back(info);
    return true;
}

const std::vector<InExInfo> &CreditAccount::getRefundOverdraft()  //获取还款透支明细
{
    return refundAndOverdraft;
}

unsigned int CreditAccount::getEnableBalance() {
    return enableBalance;
}

unsigned int CreditAccount::getTotalBalance() {
    return totalBalance;
}


// 支入支出类
IncomeExpend::IncomeExpend(std::string _clientName, std::string _password)
        : clientName(_clientName)
        , password(_password)
{

}

IncomeExpend::~IncomeExpend()
{
    for(auto & pSigDebitCard : pDebitInfo) {
        if(pSigDebitCard) {
            delete(pSigDebitCard);
            pSigDebitCard = nullptr;
        }
    }
    pDebitInfo.clear();
    for(auto & pSigCreditCard : pCreditInfo) {
        if(pSigCreditCard) {
            delete(pSigCreditCard);
            pSigCreditCard = nullptr;
        }
    }
    pCreditInfo.clear();
}

void IncomeExpend::getDetails(std::string & writeStr)
{
    writeStr.clear();
    writeStr += "【账户支出支入明细】\n";

    //借记卡
    if(!pDebitInfo.empty())
    {
        writeStr += "借记卡明细\n";
        for(const auto & pSigDebitCard : pDebitInfo) {
            writeStr += "卡号:" + pSigDebitCard->getCardNum() + '\n';
            for(const auto & sigInfo : pSigDebitCard->getWithDrawDeposit()) {
                writeStr += sigInfo.time + '\t';
                if(INCOME == sigInfo.type)
                    writeStr += "收入:";
                else
                    writeStr += "支出:";
                char money[50] = {0};
                snprintf(money, sizeof(money), "%d元\t", sigInfo.money);
                writeStr += money;
                memset(money, 0, sizeof(money));
                writeStr += "余额:";
                snprintf(money, sizeof(money), "%d元\n", sigInfo.restMoney);
                writeStr += money;
            }
        }
        writeStr += '\n';
    }

    //信用卡
    if(!pCreditInfo.empty())
    {
        writeStr += "信用卡明细\n";
        for(const auto & pSigCreditCard : pCreditInfo) {
            writeStr += "卡号:" + pSigCreditCard->getCardNum() + '\n';
            //存款取款明细
            for(const auto & sigInfo : pSigCreditCard->getWithDrawDeposit()) {
                writeStr += sigInfo.time + '\t';
                if(INCOME == sigInfo.type)
                    writeStr += "收入:";
                else
                    writeStr += "支出:";
                char money[50] = {0};
                snprintf(money, sizeof(money), "%d元\t", sigInfo.money);
                writeStr += money;
                memset(money, 0, sizeof(money));
                writeStr += "余额:";
                snprintf(money, sizeof(money), "%d元\n", sigInfo.restMoney);
                writeStr += money;
            }
            //还款透支明细
            for(const auto & sigInfo : pSigCreditCard->getRefundOverdraft()) {
                writeStr += sigInfo.time + '\t';
                if(INCOME == sigInfo.type)
                    writeStr += "还款:";
                else
                    writeStr += "透支:";
                char money[50] = {0};
                snprintf(money, sizeof(money), "%d元\t", sigInfo.money);
                writeStr += money;
                memset(money, 0, sizeof(money));
                writeStr += "可用额度:";
                snprintf(money, sizeof(money), "%d元\n", sigInfo.restMoney);
                writeStr += money;
            }
        }
        writeStr += '\n';
    }
//    cout << "writeStr:\n" << writeStr <<endl;
}

bool IncomeExpend::saveDetails(std::string saveFile)
{
    ofstream out(saveFile);
    if(!out) {
//        cout << "open " << saveFile << " error" << endl;
        return false;
    }
    string writeStr;
    getDetails(writeStr);
    if(writeStr.empty())
        return false;

    out.write(writeStr.c_str(), writeStr.size());
    return true;
}

const std::string &IncomeExpend::getClientName() {
    return clientName;
}

const std::string &IncomeExpend::getClientPassword() {
    return password;
}

void IncomeExpend::openNewCard(cardType type, std::string &cardNum) {
    cardNum.clear();
    if(type == DEBIT) {
        DebitAccount * pNewCard = new DebitAccount();
        cardNum = pNewCard->getCardNum();
        pDebitInfo.push_back(pNewCard);
    } else if (type == CREDIT) {
        CreditAccount * pNewCard = new CreditAccount();
        cardNum = pNewCard->getCardNum();
        pCreditInfo.push_back(pNewCard);
    }
}

std::vector<DebitAccount *> &IncomeExpend::getDebitInfo() {
    return pDebitInfo;
}

std::vector<CreditAccount *> &IncomeExpend::getCreditInfo() {
    return pCreditInfo;
}

void IncomeExpend::getDebitStr(std::string &infoStr) {
    infoStr.clear();
    infoStr += "借记卡卡号\t\t余额\t利息\n";
    for(const auto & i : pDebitInfo) {
        infoStr += i->getCardNum() + "\t" + intToStr(i->getBalance()) + '\t' + intToStr(i->getInterests()) + '\n';
    }
}

void IncomeExpend::getCreditStr(std::string &infoStr) {
    infoStr.clear();
    infoStr += "信用卡卡号\t\t余额\t可用额度\t固定额度\n";
    for(const auto & i : pCreditInfo) {
        infoStr += i->getCardNum() + "\t" + intToStr(i->getBalance()) + '\t' + intToStr(i->getEnableBalance()) + "\t\t" + intToStr(i->getTotalBalance()) +'\n';
    }
}


std::string intToStr(int num)
{
    string returnStr;
    char buf[50] = {0};
    snprintf(buf, sizeof(buf), "%d", num);
    returnStr = buf;
    return move(returnStr);
}

// 删除字符串两端的空白字符或指定字符
void trim(std::string& str, const char* white_space)
{
    std::string::size_type pos = str.find_last_not_of(white_space);
    if(pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(white_space);
        if(pos != std::string::npos)
        {
            str.erase(0, pos);
        }
    }
    else
    {
        str.erase(str.begin(), str.end());
    }
};


//用于分割字符串
int split(std::vector<std::string> &results, const std::string& input, const std::string& token, bool includeEmpties)
{
    results.clear();

    std::string::size_type szTotal = input.size();
    std::string::size_type szTk = token.size();

    std::string::size_type lpos = 0;
    std::string::size_type rpos = input.find(token, lpos);
    while ( rpos!=std::string::npos && lpos<szTotal)
    {
        std::string str = input.substr(lpos, rpos-lpos);
        trim(str);
        if ( !str.empty() || includeEmpties )
        {
            results.push_back(str);
        }

        lpos = rpos+szTk;
        rpos = input.find(token, lpos);
    }

    if ( lpos<szTotal )
    {
        std::string str = input.substr(lpos, szTotal-lpos);
        trim(str);
        if ( !str.empty() || includeEmpties )
        {
            results.push_back(str);
        }
    }

    return 0;
};