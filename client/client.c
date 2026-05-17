#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DISCOVERY_PORT 8888
#define TCP_PORT 9999
#define BUFFER_SIZE 1024
int udp_sock, tcp_sock;
char name[30],pas[20];

int reg()
{
    printf("User name:");
    fgets(name,sizeof(name),stdin);
    name[strcspn(name,"\n")]=0;
    while(1)
    {
        printf("Password:");
        fgets(pas,sizeof(pas),stdin);
        pas[strcspn(pas,"\n")]=0;
        if(strlen(pas)<8)
        {
            printf("Password must be at least 8 characters long\n");
        }
        else
        {
            break;
        }
    }
    char packet[128];
    snprintf(packet,sizeof(packet),"%s:%s",name,pas);
    send(tcp_sock,packet,strlen(packet),0);
    char buf[32];
    int bytes_read;
    bytes_read=recv(tcp_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    if(strcmp(buf,"Success")==0)
    {
        printf("%s\n",buf);
        return 1;
    }
    else
    {
        printf("%s\n",buf);
        return 0;
    }
}

int login()
{
    int bytes_read;
    printf("User name:");
    fgets(name,sizeof(name),stdin);
    name[strcspn(name,"\n")]=0;
    printf("Password:");
    fgets(pas,sizeof(pas),stdin);
    pas[strcspn(pas,"\n")]=0;
    char packet[128];
    snprintf(packet,sizeof(packet),"%s:%s",name,pas);
    send(tcp_sock,packet,strlen(packet),0);
    char buf[64];
    bytes_read=recv(tcp_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    if(strcmp(buf,"Success")==0)
    {
        printf("%s\n",buf);
        return 1;
    }
    else
    {
        printf("%s\n",buf);
        return 0;
    }
}

void showAccount()
{
    char buf[32];
    int bytes_read=recv(tcp_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    printf("Available money: %s",buf);
}

int withdraw()
{
    char buf[32],msg[64];
    printf("Sum: ");
    fgets(buf,sizeof(buf),stdin);
    buf[strcspn(buf,"\n")]=0;
    send(tcp_sock,buf,strlen(buf),0);
    int bytes_read=recv(tcp_sock,msg,sizeof(msg)-1,0);
    msg[bytes_read]=0;
    if(strcmp(msg,"Insufficient funds")==0)
    {
        printf("%s\n",msg);
        return 0;
    }
    else
    {
        printf("%s\n",msg);
        return 1;
    }
}

void deposit()
{
    char buf[32];
    printf("Sum: ");
    fgets(buf,sizeof(buf),stdin);
    buf[strcspn(buf,"\n")]=0;
    send(tcp_sock,buf,strlen(buf),0);
    int bytes_read=recv(tcp_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    printf("%s\n",buf);
}

void showDebts()
{
    printf("Debts of user %s:",name);
    while(1)
    {
        char msg[64];
        int bytes_read=recv(tcp_sock,msg,sizeof(msg)-1,0);
        msg[bytes_read]=0;
        if(strcmp(msg,"end")==0)break;
        printf("%s\n",msg);
    }
}

int main() 
{
    struct sockaddr_in broadcast_addr, server_addr;
    socklen_t server_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast_enable = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(DISCOVERY_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//change to 255.255.255.255 if you have 2 different computers
    const char *msg = "DISCOVER_ADMIN";
    printf("[Client] Searching for Admin Server on local network...\n");
    sendto(udp_sock, msg, strlen(msg), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    int n = recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &server_len);
    if (n < 0) 
    {
        printf("[Client] Timeout: No admin server found on the network.\n");
        close(udp_sock);
        return 1;
    }
    buffer[n] = '\0';
    if(strcmp(buffer,"CONFIRM")!=0)
    {
        printf("[Client] Unrecognized response from admin.\n");
        return 1;
    }
    close(udp_sock);
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_port = htons(TCP_PORT);
    if (connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("[Client] TCP Connection failed");
        return 1;
    }
    memset(buffer, 0, BUFFER_SIZE);
    recv(tcp_sock, buffer, BUFFER_SIZE-1, 0);
    while(1)
    {
        char opt[20];
        printf("%s",buffer);
        fgets(opt,sizeof(opt),stdin);
        opt[strcspn(opt,"\n")]=0;
        send(tcp_sock,opt,strlen(opt),0);
        if(strcmp(opt,"login")==0)
        {
            while(login()==0);
            break;
        }
        else if(strcmp(opt,"register")==0)
        {
            while(reg()==0);
            break;
        }
    }
    while(1)
    {
        printf("Operation:\n1 - Show account\n2 - Withdraw\n3 - Deposit\n4 - Show debts and paydays\n5 - Exit\n");
        char input[10];
        fgets(input,sizeof(input),stdin);
        char optiune = input[0];
        if(optiune=='5')break;
        send(tcp_sock,&optiune,sizeof(optiune),0);
        if(optiune=='1')
        {
            showAccount();
        }
        else if(optiune=='2')
        {
            while(withdraw()==0);
        }
        else if(optiune=='3')
        {
            deposit();
        }
        else if(optiune=='4')
        {
            showDebts();
        }
    }
    close(tcp_sock);
    return 0;
}