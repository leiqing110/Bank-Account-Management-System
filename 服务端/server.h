#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <stdbool.h>
#include <vector>

typedef enum _em_client_request_ {
    LOGIN = 0x4567,
    GET_COUNT_INFO,     //获取账户信息
    WITHDRAW_MONEY,     //取款
    DEPOSIT_MONEY,      //存款
    REFUND_MONEY,       //还款
    OVERDRAFT,          //透支
}CLIENT_REQUEST;      //客户端 枚举类型

typedef enum _em_bank_request_ {
    CHECK_ALL_CLIENTS = 0x5678,      //查看所有客户
    CHECK_CLIENT_ACCOUNTS,           //查看客户账户
    ADD_CLIENT,                      //添加客户
    DELETE_CLIENT,                   //删除客户
    OPEN_AN_ACCOUNT,                 //开账户
    CLOSE_AN_ACCOUNT,                //关账户
}BANK_REQUEST;   //服务端银行枚举类型

typedef enum _em_detail_type_ {
    INCOME = 0x123,
    EXPEND,
}DetailType;

typedef enum _cardType {
    DEBIT = 1,     //借记卡
    CREDIT,        //信用卡
}cardType;   

typedef struct _st_income_expend_info_ {
    DetailType type;
    std::string time;
    unsigned int money;
    int restMoney;
}InExInfo;   //用户 收入和支出信息

//typedef enum _em_server_reback_ {
//
//}SERVER_REBACK;

//获取当前时间
void getLocalTime(std::string &timeStr);

// 账户类
class Account {
protected:
    Account(std::string _cardNumber, int _balance = 0);
    Account();
    virtual ~Account() = default;

public:
    int getBalance();                         //获取余额
    const std::string &getCardNum();          //获取卡号
    const std::vector<InExInfo> & getWithDrawDeposit();  //获取存款明细
    void depositMoney(unsigned int money);     //存款
    bool withdrawMoney(unsigned int money);    //取款

protected:
    int balance;            //余额
    std::string cardNumber; //卡号
    std::vector<InExInfo> withdrawAndDeposit;      //记录此卡存款取款明细
};


//借记卡
class DebitAccount : public Account {
public:
    DebitAccount();
    DebitAccount(std::string _cardNumber, int _balance, unsigned int _intersts);
    ~DebitAccount() override = default;

    unsigned int getInterests();   //获取利息

private:
    unsigned int interests;     //利息

};

//信用卡
class CreditAccount : public Account {
public:
    CreditAccount();
    CreditAccount(std::string _cardNumber, int _balance, unsigned int _enableBalance, unsigned int _totalBalance);
    ~CreditAccount() override = default;
    const std::vector<InExInfo> & getRefundOverdraft(); //获取此卡还款的透支明细

    unsigned int getEnableBalance();
    unsigned int getTotalBalance();

    void refundMoney(unsigned int money);     //信用卡还款
    bool overdraft(unsigned int money);       //信用卡透支

private:
    unsigned int enableBalance;         //可用余额
    unsigned int totalBalance;          //固定额度
    std::vector<InExInfo> refundAndOverdraft;      //记录此卡还款透支明细
};

// 支入支出类
class IncomeExpend {
public:
    IncomeExpend(std::string _clientName, std::string _password);
    ~IncomeExpend();
    const std::string & getClientName();
    const std::string & getClientPassword();
    void openNewCard(cardType type, std::string &cardNum);        //开心账户

    void getDetails(std::string & writeStr);         //获取还款消费明细
    bool saveDetails(std::string saveFile);          //存储还款消费明细

    std::vector<DebitAccount *> & getDebitInfo();
    std::vector<CreditAccount *> & getCreditInfo();

    void getDebitStr(std::string &infoStr);
    void getCreditStr(std::string &infoStr);
protected:
//    bool loadDetails(std::string loadFile);     //加载还款消费明细


private:
    std::string clientName;                    //客户账号
    std::string password;                      //客户密码
    std::vector<DebitAccount *> pDebitInfo;    //客户借记卡信息
    std::vector<CreditAccount *> pCreditInfo;  //客户信用卡信息
};

std::string intToStr(int num);
void trim(std::string& str, const char* white_space=" \t\r\n");
int split(std::vector<std::string> &results, const std::string& input, const std::string& token, bool includeEmpties=true);
#endif // SERVER_H