#include "BankClient.hpp"

bool BankClient::discoverAdmin() 
{
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast_enable = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    struct timeval tv = {5, 0};
    setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    struct sockaddr_in broadcast_addr{};
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(DISCOVERY_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Or 255.255.255.255 if we have another computer to act as a server
    const char* msg = "DISCOVER_ADMIN";
    sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    char buffer[1024];
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    int n = recvfrom(udp_sock, buffer, 1024, 0, (struct sockaddr*)&server_addr, &server_len);
    close(udp_sock);
    if (n > 0) 
    {
        buffer[n] = '\0';
        return strcmp(buffer, "CONFIRM") == 0;
    }
    return false;
}

bool BankClient::connectToServer() 
{
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        return false;
    }
    char buffer[1024];
    recv(tcp_sock, buffer, 1023, 0); 
    return true;
}

std::string BankClient::sendRequest(const std::string& request) 
{
    send(tcp_sock, request.c_str(), request.length(), 0);
    char buffer[1024];
    int bytes_read = recv(tcp_sock, buffer, 1023, 0);
    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0';
        return std::string(buffer);
    }
    return "";
}

bool BankClient::login(const std::string& username, const std::string& password) 
{
    send(tcp_sock, "login", 5, 0);
    std::string packet = username + ":" + password;
    std::string response = sendRequest(packet);
    return response == "Success";
}

bool BankClient::registerUser(const std::string& username, const std::string& password) 
{
    send(tcp_sock, "register", 8, 0);
    std::string packet = username + ":" + password;
    std::string response = sendRequest(packet);
    return response == "Success";
}

std::string BankClient::getAccountBalance() 
{
    char opt = '1';
    send(tcp_sock, &opt, 1, 0);
    char buffer[1024];
    int bytes_read = recv(tcp_sock, buffer, 1023, 0);
    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0';
        return std::string(buffer);
    }
    return "Error loading balance";
}

std::string BankClient::withdraw(int amount) 
{
    char opt = '2';
    send(tcp_sock, &opt, 1, 0);
    return sendRequest(std::to_string(amount));
}

std::string BankClient::deposit(int amount) 
{
    char opt = '3';
    send(tcp_sock, &opt, 1, 0);
    return sendRequest(std::to_string(amount));
}

std::string BankClient::getDebts()
{
    char opt = '4';
    send(tcp_sock, &opt, 1, 0);
    char buffer[2048]; 
    int bytes_read = recv(tcp_sock, buffer, 2047, 0);
    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0';
        return std::string(buffer);
    }
    return "No debts found or error";
}

std::string BankClient::getLoan(int amount, int days)
{
    char opt = '5';
    send(tcp_sock, &opt, 1, 0);
    
    char buffer[1024];
    int bytes_read = recv(tcp_sock, buffer, 1023, 0);
    if (bytes_read <= 0) return "Connection error";
    buffer[bytes_read] = '\0';
    
    if (strcmp(buffer, "unable") == 0) 
    {
        return "Too many existing loans (Max 10)";
    }
    
    std::string packet = std::to_string(amount) + "," + std::to_string(days);
    send(tcp_sock, packet.c_str(), packet.length(), 0);
    return "Money added to balance";
}

std::string BankClient::payLoan(int amount)
{
    char opt = '6';
    send(tcp_sock, &opt, 1, 0);
    
    std::string amount_str = std::to_string(amount);
    send(tcp_sock, amount_str.c_str(), amount_str.length(), 0);
    
    char buffer[1024];
    int bytes_read = recv(tcp_sock, buffer, 1023, 0);
    if (bytes_read > 0) 
    {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, "unable") == 0) return "Not enough money in the balance to pay";
        return "Loan paid successfully";
    }
    return "Error processing payment";
}

void BankClient::disconnect() 
{
    if (tcp_sock != -1) 
    {
        close(tcp_sock);
        tcp_sock = -1;
    }
}