#include <arpa/inet.h>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../include/global.h"
#include "../include/logger.h"

using namespace std;

#define SERV_PORT 6666
#define STDIN 0
#define RT_ERR -1
#define BACKLOG 5

//-----------------------------frequently used data---------------------------------//

string myHostname;
string myPort;
string myIP;
string space = " ";
int sockfd;
struct addrinfo *myAddrInfo;
struct addrinfo hints;

fd_set masterfds;
fd_set readfds;
int fdmax;

//------------------------------data structure-------------------------------------//

class Client{
    public:
        string hostname;
        string ip;
        string port;
        string status;

        int cfd;
        int num_msg_sent;
        int num_msg_rcv;

        vector<string> blockeduser;
        vector<string> msgbuffer;
        
        bool operator<(const Client &another) const {
            return atoi(this->port.c_str()) < atoi(another.port.c_str());
        }
        
        Client(int cfd, string hostname, string ip, string port){
            this->hostname = hostname;
            this->ip = ip;
            this->port = port;
            this->status = "logged-in";

            this->cfd = cfd;
            this->num_msg_rcv = 0;
            this->num_msg_sent = 0;
        }
};

vector<Client> socketlist;

Client* getClient(int cfd = -1, string ip = "", string port = "") {
    if(cfd > 0){
        for (int i = 0; i < socketlist.size(); ++i) {
            if (socketlist[i].cfd == cfd) {
                return &socketlist[i];
            }
        }
        return NULL;
    }

    if(ip.size() > 0 && port.size() > 0){
        for(vector<Client>::iterator itr = socketlist.begin(); itr != socketlist.end(); itr++){
            if (itr->ip == ip && itr->port == port) {
                return &(*itr);
            }
        }
        return NULL;
    }

    if(ip.size() > 0){
        for (int i = 0; i < socketlist.size(); ++i) {
            if (socketlist[i].ip == ip) {
                return &socketlist[i];
            }
        }
        return NULL;
    }

    return NULL;
}

//------------------------------helper functions------------------------------------//
// initialization, for server & client
void initMyAddr(const char* port){
    myPort = port;

    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    myHostname = hostname;

    struct hostent *ht = gethostbyname(myHostname.c_str());
    for(int i = 0; ht->h_addr_list[i] != 0; i++){
        struct in_addr in;
        memcpy(&in, ht->h_addr_list[i], sizeof(struct in_addr));
        myIP = inet_ntoa(in);
    }
    
    // hints & myAddrInfo & sockfd
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, myPort.c_str(), &hints, &myAddrInfo);
    sockfd = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
    if(bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen)<0){
        perror("Bind failed");
    }else{
        cout<< "Bind successful!"<<endl;
    }

    freeaddrinfo(myAddrInfo);
}

void split_msg(string &src, char spt, vector<string> &dest){
    stringstream sstrm(src);
    string tmp;
    dest.clear();
    while(getline(sstrm, tmp, spt)){
        dest.push_back(tmp);
    }
}

bool valid_ip(string ip_test) {
    int num = count(ip_test.begin(),ip_test.end(),'.');
    if(num != 3) return false;
    vector<string> ip_parts;
    split_msg(ip_test,'.', ip_parts);
    int i = 0;
    while(i < 4){
        for(int j = 0; j < ip_parts[i].length(); ++j){
            if(ip_parts[i][j] >= '0' && ip_parts[i][j] <= '9'){
                continue;
            }else{
                return false;
            }
        }
        if(atoi(ip_parts[i].c_str()) > 255 || atoi(ip_parts[i].c_str()) < 0){
            return false;
        }
        i++;
    }
    return true;
}

bool valid_port(string port_test){
    for(int i = 0; i < port_test.size(); i++){
        if(port_test[i] > '9' || port_test[i] < '0'){
            return false;
        }
    }
    if(atoi(port_test.c_str()) > 65535 || atoi(port_test.c_str()) == 0){
        return false;
    }
    return true;
}

//----------------------------------logger------------------------------------------//
void log_ERROR(string cmd) {
    cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_Client_EVENT(string client_ip, string msg) {
    string cmd = "RECEIVED";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip.c_str(), msg.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_Server_EVENT(string from_ip, string msg, string to_ip) {
    string cmd = "RELAYED";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from_ip.c_str(), to_ip.c_str(), msg.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_BLOCKED(string cli_ip) {
    string cmd = "BLOCKED";
    if (!valid_ip(cli_ip) || getClient(-1, cli_ip) == NULL) {
        log_ERROR(cmd);
        return;
    }
    Client* hd = getClient(-1, cli_ip);
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    for (int i = 0; i < hd->blockeduser.size(); ++i) {
        Client* new_hd = getClient(-1, hd->blockeduser[i]);
        cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1,
                new_hd->hostname.c_str(), new_hd->ip.c_str(), new_hd->port.c_str());
    }
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
// log general info when !SUCCESS!
//    type includes: AUTHOR, IP, PORT, STATISTICS, LIST
//    also: SEND, EXIT, LOGOUT, REFRESH, BLOCK, UNBLOCK (do nothing specifically)
void log_GeneralInfo(string type){
    // begining
    cse4589_print_and_log("[%s:SUCCESS]\n", type.c_str());

    // different cases
    if(type == "AUTHOR"){
        string ubit_name = "junsongh";
        cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",  ubit_name.c_str());
    }
    if(type == "IP"){
        cse4589_print_and_log("IP:%s\n", myIP.c_str());
    }
    if(type == "PORT"){
        cse4589_print_and_log("PORT:%d\n", atoi(myPort.c_str()));
    }
    if(type == "STATISTICS"){
        sort(socketlist.begin(), socketlist.end());
        for (int i = 0; i < socketlist.size(); ++i) {
            cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i + 1, socketlist[i].hostname.c_str(),
                                  socketlist[i].num_msg_sent, socketlist[i].num_msg_rcv, socketlist[i].status.c_str());
        }
    }
    if(type == "LIST"){
        sort(socketlist.begin(), socketlist.end());
        for (int i = 0; i < socketlist.size(); ++i) {
            if(socketlist[i].status == "logged-out"){
                continue;
            }
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1, socketlist[i].hostname.c_str(),
                                  socketlist[i].ip.c_str(), atoi(socketlist[i].port.c_str()));
        }
    }

    // end
    cse4589_print_and_log("[%s:END]\n", type.c_str());
}

//----------------------------------clientEnd---------------------------------------//
void clientEnd(char *port){
    bool loged_in = false;

    FD_ZERO(&masterfds);
    FD_SET(0, &masterfds);
    fdmax = 2;
    
    string myServerIP;
    string myServerPORT;

    char message[BUFSIZ];
    string msg;
    vector<string> msg_buf;
    vector<string> msg_vec;

    initMyAddr(port);
    
    while(true){
        // copy fds
        readfds = masterfds;

        // two cases: loged in or not
        if(loged_in){
            // if already loged in
            FD_SET(sockfd, &readfds);
            fdmax = sockfd;
            select(fdmax+1, &readfds, NULL, NULL, NULL);
            
            if(FD_ISSET(0, &readfds)){
                memset(message, 0, sizeof(message));
                read(STDIN,message,sizeof(message));
                msg = message;
           	    msg = msg.substr(0, msg.length() - 1);
                fflush(0);
                split_msg(msg, ' ', msg_vec);

                // for different instructions
                if(msg_vec[0] == "AUTHOR" || msg_vec[0] == "IP" || msg_vec[0] == "PORT"){
                    log_GeneralInfo(msg_vec[0]);
                }else if(msg_vec[0] == "LOGOUT"){
                    msg = "LOGOUT " + myIP;
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    loged_in = false;
                    log_GeneralInfo(msg_vec[0]);
                }else if(msg_vec[0] == "EXIT"){
                    msg = "EXIT " + myIP;
                    send(sockfd, (const char*)msg.c_str(), msg.length(), 0); 
                    log_GeneralInfo(msg_vec[0]);
                    exit(0);
                }else if(msg_vec[0] == "REFRESH"){
                    msg = "REFRESH " + myIP; 
                    send(sockfd, (const char*)msg.c_str(), msg.length(), 0); 
                    log_GeneralInfo(msg_vec[0]);
                }else if(msg_vec[0] == "LIST"){
                    log_GeneralInfo(msg_vec[0]);
                }else if(msg_vec[0] == "BLOCK"){
                    if (!valid_ip(msg_vec[1]) || getClient(-1, msg_vec[1]) == NULL)
                    {
                        log_ERROR("BLOCK");
                        continue;
                    }

                    Client *hd = getClient(-1, myIP);
                    if (find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_vec[1]) != hd->blockeduser.end())
                    {
                        log_ERROR("BLOCK");
                        continue;
                    }
                    hd->blockeduser.push_back(msg_vec[1]);
                    msg = "BLOCK " + myIP + " " + msg_vec[1];
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_GeneralInfo("BLOCK");
                }else if(msg_vec[0] == "UNBLOCK"){
                    if (getClient(-1, msg_vec[1]) == NULL){
                        log_ERROR("UNBLOCK");
                        continue;
                    }
                    Client *hd = getClient(sockfd);
                    if (hd == NULL){
                        msg = "UNBLOCK " + myIP + " " + msg_vec[1];
                        send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                        log_GeneralInfo("UNBLOCK");
                    }else{
                        if (find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_vec[1]) == hd->blockeduser.end()){
                            continue;
                        }
                        msg = "UNBLOCK " + myIP + " " + msg_vec[1];
                        send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                        log_GeneralInfo("UNBLOCK");
                    }
                }else if(msg_vec[0] == "SEND"){
                    if(!valid_ip(msg_vec[1]) || getClient(-1, msg_vec[1]) == NULL){
                        log_ERROR("SEND");
                        continue;
                    }
                    msg = "SEND " + myIP + " " + msg_vec[1];
                    for(int i = 2; i < msg_vec.size(); i++){
                        msg += " " + msg_vec[i];
                    }
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_GeneralInfo("SEND");
                }else if(msg_vec[0] == "BROADCAST"){
                    msg = "BROADCAST " + myIP;
                    for(int i = 1; i < msg_vec.size(); i++){
                        msg += " " + msg_vec[i];
                    }
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_GeneralInfo("BROADCAST");
                }
            }else if(FD_ISSET(sockfd, &readfds)){
                memset(message, 0, sizeof(message));
                if(recv(sockfd, message, sizeof(message), 0) == 0){
                    // this client is closed by server
                    close(sockfd);
                    fdmax = 0;
                    loged_in = false;
                    continue;
                }

                msg = message;
                split_msg(msg, ' ', msg_vec);
                if(msg_vec[0] == "REFRESH"){
                    socketlist.clear();
                    for(int j = 1; j < (msg_vec.size() - 1); j += 3){
                        if(getClient(-1, msg_vec[j+1], msg_vec[j+2]) == NULL){
                            Client clt(-2, msg_vec[j], msg_vec[j+1], msg_vec[j+2]);
                            socketlist.push_back(clt);
                        }
                    }
                }else if(msg_vec[0] == "SEND"){
                    msg = msg_vec[3];
                    for(int j = 4; j < msg_vec.size(); j++){
                        msg += " " + msg_vec[j];
                    }
                    log_Client_EVENT(msg_vec[1], msg);
                }else if(msg_vec[0] == "BROADCAST"){
                    msg = msg_vec[2];
                    for(int j = 3; j < msg_vec.size(); j++){
                        msg += " " + msg_vec[j];
                    }
                    log_Client_EVENT(msg_vec[1], msg);
                }
            }
        }else{
            // if not loged in, listen to "stdin", for instructions
            select(fdmax+1, &readfds, NULL, NULL, NULL);

            // two cases: 1) has no instruction 2) has new instruction
            if(FD_ISSET(0, &readfds) == 0){
                continue;
            }

            memset(message, 0, sizeof(message));
            read(STDIN,message,sizeof(message));
            msg = message;

           	msg = msg.substr(0, msg.length() - 1);

            fflush(0);
            split_msg(msg, ' ', msg_vec);
            // for different instructions
            if(msg_vec[0] == "AUTHOR" || msg_vec[0] == "IP" || msg_vec[0] == "PORT"){
                log_GeneralInfo(msg_vec[0]);
            }else if(msg_vec[0] == "EXIT"){
                send(sockfd, (const char*)("EXIT " + myIP).c_str(), msg.length(), 0); 
                log_GeneralInfo(msg_vec[0]);
                exit(0);
            }else if(msg_vec[0] == "LOGIN"){
                // handle login exceptions
                if(!valid_ip(msg_vec[1]) || !valid_port(msg_vec[2])){
                    log_ERROR(msg_vec[0]);
                    continue;
                }

                if(myServerIP != msg_vec[1] || myServerPORT != msg_vec[2]){
                    myServerIP = msg_vec[1];
                    myServerPORT = msg_vec[2];

                    // server addrinfo
                    struct addrinfo *serverInfo, *p;

                    // if can't reach server, log_ERROR & continue
                    if(getaddrinfo(myServerIP.c_str(), myServerPORT.c_str(), &hints, &serverInfo) != 0){
                        myServerIP = "";
                        myServerPORT = "";
                        freeaddrinfo(serverInfo);   // 这里多添加了几次，在各种情况下都free
                        log_ERROR(msg_vec[0]);
                        continue;
                    }

                    // try to connect to server
                    for(p = serverInfo; p != NULL; p = p->ai_next){
                        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == 0){
                            break;
                        }else{
                            close(sockfd);
                        }
                    }

                    // no successful connection, log_ERROR & continue
                    if(p == NULL){
                        myServerIP = "";
                        myServerPORT = "";
                        freeaddrinfo(serverInfo);   // 这里多添加了几次，在各种情况下都free
                        log_ERROR(msg_vec[0]);
                        continue;
                    }
                    
                    // if connected successfully
                    freeaddrinfo(serverInfo);   // 这里多添加了几次，在各种情况下都free
                }

                // if connected, start to login
                msg = "LOGIN " + myHostname + " " + myIP + " " + myPort;
                send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                loged_in = true;

                // REFRESH client/socket list
                // if has messages in buffer, handle them
                memset(message, 0, sizeof(message));
                
                // split each line of message, save in msg_buf
                recv(sockfd, message, sizeof(message), 0);
                msg = message;
                split_msg(msg, '\n', msg_buf);

                // check & handle each message
                for(int i = 0; i < msg_buf.size(); i++){
                    split_msg(msg_buf[i], ' ', msg_vec);
                    if(msg_vec[0] == "REFRESH"){
                        socketlist.clear();
                        for(int j = 1; j < (msg_vec.size() - 1); j += 3){
                            if(getClient(-1, msg_vec[j+1], msg_vec[j+2]) == NULL){
                                Client clt(-2, msg_vec[j], msg_vec[j+1], msg_vec[j+2]);
                                socketlist.push_back(clt);
                            }
                        }
                    }else if(msg_vec[0] == "SEND"){
                        msg = msg_vec[3];
                        for(int j = 4; j < msg_vec.size(); j++){
                            msg += " " + msg_vec[j];
                        }
                        log_Client_EVENT(msg_vec[1], msg);
                    }else if(msg_vec[0] == "BROADCAST"){
                        msg = msg_vec[2];
                        for(int j = 3; j < msg_vec.size(); j++){
                            msg += " " + msg_vec[j];
                        }
                        log_Client_EVENT(msg_vec[1], msg);
                    }
                }

                // login success message
                log_GeneralInfo("LOGIN");
            }
        }
    }
}


//----------------------------------serverEnd---------------------------------------//
void serverEnd(string server_port){

    fd_set global_rdfs, current_rdfs;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;

    char charmsg[BUFSIZ];
    string msg;
    vector<string> msg_p;
    struct addrinfo *ai, *p;

    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    int bytes = 0;
    int maxfd;
    int fdtemp,n;
    char buf[BUFSIZ],str;
    len = sizeof(struct sockaddr_in);
    const char* charPort = server_port.c_str();

    initMyAddr(charPort);
    printf("My ip address is : %s\n",myIP.c_str());
    listen(sockfd,BACKLOG);
    listenfd = sockfd;

    FD_ZERO(&global_rdfs);
    FD_ZERO(&current_rdfs);
    FD_SET(listenfd, &global_rdfs);
    FD_SET(STDIN, &global_rdfs);
    maxfd = listenfd;
    
    while(1){
//        fflush(stdout);
        current_rdfs = global_rdfs;
        if(select(maxfd+1, &current_rdfs,NULL,NULL,NULL)<0){
            perror("select error.\n");
            exit(-1);
        }else if(select(maxfd+1, &current_rdfs,NULL,NULL,NULL) == 0 ){
            perror("Select time out.\n");
            exit(-1);
        }

        for(fdtemp = 0; fdtemp<= maxfd; fdtemp++){
            if(FD_ISSET(fdtemp,&current_rdfs)){

                //键盘输入
                if(STDIN == fdtemp){
                    memset(&charmsg[0], 0, sizeof(charmsg));
                    n = read(STDIN,charmsg,sizeof(charmsg));
                    fflush(STDIN);
                    string msg = "";
                    msg = charmsg;
                    msg = msg.substr(0, msg.length() - 1);
                    msg_p.clear();
                    split_msg(msg,' ',msg_p);
                    
                    if(msg_p[0] == "AUTHOR" || msg_p[0] == "LIST" || msg_p[0] == "STATISTICS" || msg_p[0] == "IP" || msg_p[0] == "PORT"){
                        log_GeneralInfo(msg_p[0]);
                        break;
                    }
                    if(msg_p[0] == "BLOCKED"){log_BLOCKED(msg_p[1]);break;}
                }

                //创造连接
                else if(listenfd == fdtemp){
                    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr,(socklen_t*)&len))<0){
                        perror("accept error.\n");
                        exit(-1);
                    }
                    // 这里改动了socklen_t
                    printf("receive from %s at Port %d\n",inet_ntop(AF_INET,&client_addr.sin_addr,&str, sizeof((socklen_t)str)),ntohs(client_addr.sin_port));
                    
                    FD_CLR(fdtemp, &current_rdfs);
                    maxfd = maxfd >connfd? maxfd:connfd;
                    FD_SET(connfd,&global_rdfs);
                }
                //信息交流
                else{
                    /*initialize buffer to receive message*/
                    memset(&charmsg[0], 0, sizeof(charmsg));
                    bytes = recv(fdtemp, charmsg, sizeof(charmsg), 0 );
                    if(bytes<0){
                        perror("recv error.\n");
                        exit(-1);
                    }
                    if(bytes == 0){//退出
                        FD_CLR(fdtemp, &global_rdfs);
                        close(fdtemp);
                        continue;
                    }
                    msg = charmsg;
                    split_msg(msg,' ',msg_p);
                    
                    if(msg_p[0] == "LOGIN"){
                        Client *hd = getClient(fdtemp);

                        if (hd == NULL)
                        {
                            Client hd(fdtemp, msg_p[1], msg_p[2], msg_p[3]);
                            socketlist.push_back(hd);
                        }
                        else
                        {
                            hd->status = "logged-in";
                            if (!hd->msgbuffer.empty())
                            {
                                for (vector<string>::iterator it = hd->msgbuffer.begin(); it < hd->msgbuffer.end(); it++)
                                {
                                    send(hd->cfd, (const char *)(*it).c_str(), (*it).length(), 0);

                                    split_msg(msg,' ',msg_p);
                                    string org_ip = msg_p[1];
                                    string tar_ip = (msg_p[0] == "BROADCAST") ? "255.255.255.255" : msg_p[2];
                                    string bufmsg = (msg_p[0] == "BROADCAST") ? msg_p[2] : msg_p[3];
                                    log_Server_EVENT(org_ip, bufmsg, tar_ip);
                                }
                                hd->msgbuffer.clear();
                            }
                        }
                        string message = "REFRESH";
                        for (int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].status != "logged-in"){
                                continue;
                            }
                            message += space + socketlist[i].hostname + space + socketlist[i].ip + space + socketlist[i].port;
                        }
                        
                        send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
                    }
                    
                    if(msg_p[0] == "LOGOUT"){
                        cout << "entered logout" << endl;
                        string ip_addr = msg_p[1];
                        cout << ip_addr << endl;
                        Client *hd = getClient(-1, ip_addr);
                        if (hd != NULL)
                        {
                            cout << "found this client" << endl;
                            hd->status = "logged-out";
                        }
                    }
                    
                    
                    if(msg_p[0] == "EXIT"){
                        for (int i = 0; i < socketlist.size(); i++)
                        {
                            if (socketlist[i].cfd == fdtemp){
                                socketlist.erase(socketlist.begin() + i);
                                i--;
                            }
                        }
                    }
                    
                    if(msg_p[0] == "REFRESH"){
                        string cur_msg = "REFRESH";
                        for (int i = 0; i < socketlist.size(); i++){
                            if (socketlist[i].status == "logged-in")
                            {
                                cur_msg += " " + socketlist[i].hostname;
                                cur_msg += " " + socketlist[i].ip + space + socketlist[i].port;
                            }
                        }
                        send(fdtemp, cur_msg.c_str(), strlen(cur_msg.c_str()), 0);
                    }
                    
                    if(msg_p[0] == "BROADCAST"){

                        string con = "255.255.255.255";

                        string from_ip = msg_p[1];
                        Client *hd2 = getClient(-1, from_ip);
                        if (hd2 == NULL)
                        {
                            continue;
                        }
                        
                        for (int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].ip == from_ip)
                            {
                                continue;
                            }
                            Client *hd = getClient(-1, socketlist[i].ip);
                            if (find(socketlist[i].blockeduser.begin(), socketlist[i].blockeduser.end(), from_ip) == socketlist[i].blockeduser.end())
                            {
                                if (socketlist[i].status == "logged-in")
                                {
                                    send(socketlist[i].cfd, (const char *)msg.c_str(), msg.length(), 0);
                                    socketlist[i].num_msg_rcv = socketlist[i].num_msg_rcv + 1;
                                    hd2->num_msg_sent = hd2->num_msg_sent + 1;
                                    string message;
                                    message = msg_p[2];
                                    for (int m = 3; m < msg_p.size(); m++)
                                    {
                                        message += space + msg_p[m];
                                    }
                                    
                                    log_Server_EVENT(from_ip, message, con);
                                }
                                else
                                {
                                    socketlist[i].msgbuffer.push_back(msg);
                                    socketlist[i].num_msg_rcv = socketlist[i].num_msg_rcv + 1;
                                    hd2->num_msg_sent = hd2->num_msg_sent + 1;
                                }
                            }
                        }
                    }
                    
                    if(msg_p[0] == "BLOCK"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        Client *hd = getClient(-1, from_ip);

                        if (valid_ip(to_ip) && (getClient(-1, to_ip) != NULL) && (find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip) == hd->blockeduser.end())){
                            hd->blockeduser.push_back(to_ip);
                        }else{
                            log_ERROR("BLOCK");
                        }
                    }
                    
                    if(msg_p[0] == "UNBLOCK"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        Client *hd = getClient(-1, from_ip);
                        if (!valid_ip(to_ip) || getClient(-1, to_ip) == NULL || (hd->blockeduser.end() == find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip))){
                            log_ERROR("UNBLOCK");
                        }
                        else
                        {
                            hd->blockeduser.erase(find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip));
                        }
                    }
                    
                    if(msg_p[0] == "SEND"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        Client *hd = getClient(-1, to_ip);

                        if (hd == NULL)
                        {
                            continue;
                        }

                        Client *hd2 = getClient(-1, from_ip);
                        
                        if (find(hd->blockeduser.begin(), hd->blockeduser.end(), from_ip) != hd->blockeduser.end())
                        {
                            continue;
                        }

                        if (hd->status == "logged-in"){
                            send(hd->cfd, (const char *)msg.c_str(), msg.length(), 0);//
                            hd->num_msg_rcv = hd->num_msg_rcv + 1;
                            hd2->num_msg_sent = hd2->num_msg_sent + 1;
                            string message;
                            message = msg_p[3];
                            for (int m = 4; m < msg_p.size(); m++)
                            {
                                message += space + msg_p[m];
                            }
                            log_Server_EVENT(msg_p[1], message, msg_p[2]);
                        }
                        else
                        {
                            hd->msgbuffer.push_back(msg);
                            hd->num_msg_rcv = hd->num_msg_rcv + 1;
                            hd2->num_msg_sent = hd2->num_msg_sent + 1;
                        }
                    }
                    

                    fflush(stdout);// not sure
                }
            }
        }
    }
    
}


//---------------------------------Main Entry--------------------------------------//
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
   	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if(*argv[1]=='s')
	{
	    serverEnd(argv[2]);
	}
	else if(*argv[1]=='c')
	{
	    clientEnd(argv[2]);
	}
	else{
	    printf("System out");
	    exit(-1);
	}

	return 0;
}
