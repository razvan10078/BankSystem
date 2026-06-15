#pragma once
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class BankClient {
public:
    BankClient() : tcp_sock(-1) {}
    ~BankClient() { disconnect(); }
    bool discoverAdmin();
    bool connectToServer();
    void disconnect();

    bool login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password);

    std::string getAccountBalance();
    std::string withdraw(int amount);
    std::string deposit(int amount);
    std::string getDebts();
    std::string getLoan(int amount, int days);
    std::string payLoan(int amount);
    std::string transferFindDest(const std::string& dest);
    std::string transferSendMoney(const std::string& dest, int amount);

private:
    int tcp_sock;
    const int DISCOVERY_PORT = 8888;
    const int TCP_PORT = 9999;
    std::string sendRequest(const std::string& request);
};
