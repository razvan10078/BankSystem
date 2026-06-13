#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <crypt.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>

#define DISCOVERY_PORT 8888
#define TCP_PORT 9999
#define BUFFER_SIZE 1024
int udp_sock, tcp_sock, client_sock;
char name[30],pas[20];
int accountD;

typedef struct detail{
    char name[30];
    int sumaCont;
    int imprumut[10];
    int zileScadente[10];
    int nrImprumuturi;
    time_t momentImprumut[10];
}detail;

detail d;

void generate_salt(char *salt_output) {
    const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    int i;
    strcpy(salt_output, "$6$");
    FILE *urandom = fopen("/dev/urandom", "r");
    if (urandom == NULL) {
        perror("Failed to open /dev/urandom");
        exit(1);
    }
    for (i = 0; i < 16; i++) {
        unsigned char random_byte;
        fread(&random_byte, 1, 1, urandom);
        salt_output[3 + i] = charset[random_byte % strlen(charset)];
    }
    salt_output[19] = '$';
    salt_output[20] = '\0';
    fclose(urandom);
}

int reg()
{
    int bytes_read;
    char buf[BUFFER_SIZE];
    bytes_read=recv(client_sock,buf,BUFFER_SIZE-1,0);
    buf[bytes_read]=0;
    char *token=strtok(buf,":");
    strcpy(name,token);
    token=strtok(NULL,":");
    strcpy(pas,token);
    FILE *f=fopen(".login_info","r");
    char line[256];
    while(fgets(line,sizeof(line),f))
    {
        token=strtok(line,":");
        if(strcmp(token,name)==0)
        {
            fclose(f);
            return 0;
        }
    }
    char salt[21];
    generate_salt(salt);
    char *ps=crypt(pas,salt);
    snprintf(line,sizeof(line),"%s:%s\n",name,ps);
    int fd=open(".login_info",O_WRONLY | O_APPEND);
    write(fd,line,strlen(line));
    close(fd);
    fd=open(".account_details",O_WRONLY | O_APPEND);
    detail d;
    strcpy(d.name,name);
    d.sumaCont=0;
    d.nrImprumuturi=0;
    write(fd,&d,sizeof(d));
    close(fd);
    return 1;
}

int login()
{
    int bytes_read;
    char buf[BUFFER_SIZE];
    bytes_read=recv(client_sock,buf,BUFFER_SIZE-1,0);
    buf[bytes_read]=0;
    char *token=strtok(buf,":");
    strcpy(name,token);
    token=strtok(NULL,":");
    strcpy(pas,token);
    char line[256];
    FILE *f=fopen(".login_info","r");
    while(fgets(line,sizeof(line),f))
    {
        token=strtok(line,":\n");
        if(strcmp(token,name)==0)
        {
            fclose(f);
            token=strtok(NULL,": \n");
            char *hash=crypt(pas,token);
            if(strcmp(hash,token)==0)
            {
                return 1;
            }
            else return 0;
        }
    }
    fclose(f);
    return 0;
}

void showSum()
{
    char msg[32];
    snprintf(msg,sizeof(msg),"Suma cont: %d\n",d.sumaCont);
    send(client_sock,msg,strlen(msg),0);
}

int withdraw()
{
    char buf[32],msg[32];
    int bytes_read;
    bytes_read=recv(client_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    int sm=atoi(buf);
    if(d.sumaCont<sm)
    {
        snprintf(msg,sizeof(msg),"Insufficient funds");
        send(client_sock,msg,strlen(msg),0);
        return 0;
    }
    d.sumaCont-=sm;
    lseek(accountD,-(off_t)sizeof(d),SEEK_CUR);
    write(accountD,&d,sizeof(d));
    snprintf(msg,sizeof(msg),"Successful withdrawal");
    send(client_sock,msg,strlen(msg),0);
    return 1;
}

void deposit()
{
    char buf[32];
    int bytes_read;
    bytes_read=recv(client_sock,buf,sizeof(buf)-1,0);
    buf[bytes_read]=0;
    int sm=atoi(buf);
    d.sumaCont+=sm;
    lseek(accountD,-(off_t)sizeof(d),SEEK_CUR);
    write(accountD,&d,sizeof(d));
    snprintf(buf,sizeof(buf),"Successful deposit");
    send(client_sock,buf,strlen(buf),0);
}

void showDebts()
{
    int n = d.nrImprumuturi;
    char msg[BUFFER_SIZE];
    char temp_line[128];
    if(n==0) 
    {
        strcpy(msg, "No debts\n");
    } 
    else 
    {
        for(int i=0;i<n;i++) 
        {
            snprintf(temp_line, sizeof(temp_line), "Sum: %d\tLast payday in: %d days\n",d.imprumut[i], d.zileScadente[i]);
            strcat(msg, temp_line); 
        }
    }
    send(client_sock, msg, strlen(msg), 0);
}

void getLoan()
{
    char msg[32];
    if(d.nrImprumuturi==10)
    {
        snprintf(msg,sizeof(msg),"unable");
        send(client_sock,msg,strlen(msg),0);
    }
    else
    {
        snprintf(msg,sizeof(msg),"able");
        send(client_sock,msg,strlen(msg),0);
        int bytes_read;
        bytes_read=recv(client_sock,msg,sizeof(msg)-1,0);
        msg[bytes_read]=0;
        char *token=strtok(msg,",");
        int imp=atoi(token);
        token=strtok(NULL,",");
        int days=atoi(token);
        d.sumaCont+=imp;
        d.imprumut[d.nrImprumuturi]=imp;
        d.zileScadente[d.nrImprumuturi]=days;
        d.momentImprumut[d.nrImprumuturi]=time(NULL);
        d.nrImprumuturi++;
        lseek(accountD,-(off_t)sizeof(d),SEEK_CUR);
        write(accountD,&d,sizeof(d));
    }
}

void payLoan()
{
    char buff[16],msg[32];
    int n=d.nrImprumuturi;
    int bytes_read=recv(client_sock,buff,sizeof(buff)-1,0);
    buff[bytes_read]=0;
    int sm=atoi(buff);
    if(sm>d.sumaCont)
    {
        snprintf(msg,sizeof(msg),"unable");
        send(client_sock,msg,strlen(msg),0);
    }
    else
    {
        snprintf(msg,sizeof(msg),"able");
        send(client_sock,msg,strlen(msg),0);
        int original_sm=sm;
        int remaining_loans=0;
        for(int i=0;i<n;i++)
        {
            if(sm>=d.imprumut[i])
            {
                sm-=d.imprumut[i];
            }
            else
            {
                if(sm>0)
                {
                    d.imprumut[i]-=sm;
                    sm=0;
                }
                d.imprumut[remaining_loans] = d.imprumut[i];
                d.zileScadente[remaining_loans] = d.zileScadente[i];
                d.momentImprumut[remaining_loans] = d.momentImprumut[i];
                remaining_loans++;
            }
        }
        d.nrImprumuturi = remaining_loans;
        int actual_paid = original_sm - sm;
        d.sumaCont -= actual_paid;
    }
    lseek(accountD,-(off_t)sizeof(d),SEEK_CUR);
    write(accountD,&d,sizeof(d));
}

void handle_client() {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    const char *msg = "Login\nOr\nRegister\n";
    send(client_sock, msg, strlen(msg), 0);
    bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) 
    {
        close(client_sock);
        exit(0);
    }
    buffer[bytes_read] = 0;
    if (strcmp(buffer, "login") == 0) 
    {
        char msg[32];
        while (login() == 0) 
        {
            snprintf(msg, sizeof(msg), "Invalid password or username");
            send(client_sock, msg, strlen(msg), 0);
        }
        snprintf(msg, sizeof(msg), "Success");
        send(client_sock, msg, strlen(msg), 0);
    } 
    else if (strcmp(buffer, "register")==0) 
    {
        char msg[32];
        while (reg() == 0) 
        {
            snprintf(msg, sizeof(msg), "Username already exists");
            send(client_sock, msg, strlen(msg), 0);
        }
        snprintf(msg, sizeof(msg), "Success");
        send(client_sock, msg, strlen(msg), 0);
    }
    accountD = open(".account_details", O_RDWR);
    while (read(accountD, &d, sizeof(d)) == sizeof(d)) 
    {
        if (strcmp(d.name, name) == 0) 
        {
            break;
        }
    }
    char optiune;
    while (recv(client_sock, &optiune, sizeof(optiune), 0) > 0) 
    {
        if(optiune=='1') 
        {
            showSum();
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
        else if(optiune=='5')
        {
            getLoan();
        }
        else if(optiune=='6')
        {
            payLoan();
        }
    }
    close(accountD);
    close(client_sock);
    exit(0);
}

void updateDebts() {
    int fd = open(".account_details", O_RDWR);
    if (fd < 0) 
    {
        perror("Failed to open account details for updating");
        return; 
    }
    detail d;
    time_t curTime = time(NULL);
    int trunc = 0;
    while (read(fd, &d, sizeof(detail)) == sizeof(detail)) 
    {
        if (d.name[0] == '\0') continue;
        int record_updated = 0;
        int close_account = 0;
        for (int i = 0; i < d.nrImprumuturi; i++) 
        {
            double elapsed_seconds = difftime(curTime, d.momentImprumut[i]);
            int days_passed = (int)(elapsed_seconds / 86400);
            
            if (days_passed > 0) 
            {
                d.zileScadente[i] -= days_passed;
                d.momentImprumut[i] += (days_passed * 86400);
                
                if (d.zileScadente[i] < 0)
                {
                    if(d.zileScadente[i] < -2)
                    {
                        if(d.sumaCont >= d.imprumut[i])
                        {
                            d.sumaCont -= d.imprumut[i];
                            for (int j = i; j < d.nrImprumuturi-1; j++)
                            {
                                d.imprumut[j] = d.imprumut[j+1];
                                d.zileScadente[j] = d.zileScadente[j+1]; 
                                d.momentImprumut[j] = d.momentImprumut[j+1]; 
                            }
                            d.nrImprumuturi--;
                            i--;
                        }
                        else close_account = 1;
                    }
                    else
                    {
                        d.imprumut[i]+=d.imprumut[i]/20;
                    }
                }
                record_updated = 1;
            }
        }
        
        if (close_account)
        {
            memset(&d, 0, sizeof(detail)); 
            lseek(fd, -(off_t)sizeof(detail), SEEK_CUR);
            write(fd, &d, sizeof(detail));
            trunc = 1;
        }
        else if (record_updated) 
        {
            lseek(fd, -(off_t)sizeof(detail), SEEK_CUR);
            write(fd, &d, sizeof(detail));
        }
    }
    if (trunc) 
    {
        lseek(fd, 0, SEEK_SET); 
        off_t write_pos = 0;
        while (read(fd, &d, sizeof(detail)) == sizeof(detail)) 
        {
            if (d.name[0] != '\0') 
            {
                off_t current_read_pos = lseek(fd, 0, SEEK_CUR); 
                lseek(fd, write_pos, SEEK_SET);
                write(fd, &d, sizeof(detail));
                write_pos += sizeof(detail);
                lseek(fd, current_read_pos, SEEK_SET);
            }
        }
        ftruncate(fd, write_pos);
    }
    close(fd);
}

int main() 
{
    signal(SIGCHLD, SIG_IGN); 
    struct stat buf;
    char path[] = ".login_info";
    if (stat(path, &buf) == -1) 
    {
        int f = creat(path, S_IRUSR | S_IWUSR);
        close(f);
    }
    if (stat(".account_details", &buf) == -1) 
    {
        int f = creat(".account_details", S_IRUSR | S_IWUSR);
        close(f);
    }
    struct sockaddr_in server_udp_addr, server_tcp_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server_udp_addr, 0, sizeof(server_udp_addr));
    server_udp_addr.sin_family = AF_INET;
    server_udp_addr.sin_addr.s_addr = INADDR_ANY;
    server_udp_addr.sin_port = htons(DISCOVERY_PORT);
    bind(udp_sock, (struct sockaddr*)&server_udp_addr, sizeof(server_udp_addr));
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&server_tcp_addr, 0, sizeof(server_tcp_addr));
    server_tcp_addr.sin_family = AF_INET;
    server_tcp_addr.sin_addr.s_addr = INADDR_ANY;
    server_tcp_addr.sin_port = htons(TCP_PORT);
    bind(tcp_sock, (struct sockaddr*)&server_tcp_addr, sizeof(server_tcp_addr));
    listen(tcp_sock, 5);
    printf("[Admin] Server running. Listening for UDP discovery and TCP connections...\n");
    fd_set readfds;
    int max_sd = (udp_sock > tcp_sock) ? udp_sock : tcp_sock;
    if (fork() == 0) 
    {
        while(1) 
        {
            updateDebts();
            sleep(86400);
        }
        exit(0);
    }
    while (1) 
    {
        FD_ZERO(&readfds);
        FD_SET(udp_sock, &readfds);
        FD_SET(tcp_sock, &readfds);
        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) 
        {
            perror("select error");
            continue;
        }
        if (FD_ISSET(udp_sock, &readfds)) 
        {
            memset(buffer, 0, BUFFER_SIZE);
            recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer, "DISCOVER_ADMIN") == 0) 
            {
                printf("[Admin] Discovery request received from %s:%d\n", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                const char *reply = "CONFIRM";
                sendto(udp_sock, reply, strlen(reply), 0, (struct sockaddr*)&client_addr, client_len);
            }
        }
        if (FD_ISSET(tcp_sock, &readfds)) {
            client_sock = accept(tcp_sock, (struct sockaddr*)&client_addr, &client_len);
            if (client_sock < 0) continue;
            printf("[Admin] Client successfully connected via TCP from %s:%d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            if(fork() == 0) 
            {
                close(tcp_sock);
                close(udp_sock);
                handle_client();
            } 
            else 
            {
                close(client_sock);
            }
        }
    }
    close(tcp_sock);
    close(udp_sock);
    return 0;
}