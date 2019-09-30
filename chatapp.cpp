#include "header.h"

using namespace std;

#define SERV_PORT 6666
#define STDIN 0
#define RT_ERR -1
#define BACKLOG 5

void serverEnd(string server_port);
void clientEnd(string client_port);

//-----------------------------frequently used data---------------------------------//

string myHostname;
string myPort;
string myIP;
string blank = " ";
int sockfd;
struct addrinfo *myAddrInfo;
struct addrinfo hints;

fd_set masterfds;
fd_set readfds;
int fdmax;

//------------------------------data structure-------------------------------------//

struct SocketObject{
    int cfd;
    string hostname;
    string ip;
    string port;
    int num_msg_sent;
    int num_msg_rcv;
    string status;
    vector<string> blockeduser;
    vector<string> msgbuffer;

    bool operator<(const SocketObject &rhs) const {
        return atoi(port.c_str()) < atoi(rhs.port.c_str());
    }
};
vector<SocketObject> socketlist;

SocketObject* newSocketObject(int cfd, string hostname, string ip, string port){
    SocketObject* hd = new SocketObject;
    hd->cfd = cfd;
    hd->hostname = hostname;
    hd->ip = ip;
    hd->port = port;
    hd->num_msg_sent = 0;
    hd->num_msg_rcv = 0;
    hd->status = "logged-in";

    return hd;

}
SocketObject* InSetSocket(string ip, string port) {
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        SocketObject* hd = &socketlist[i];
        if (hd->ip == ip && hd->port == port) {
            return hd;
        }
    }
    return NULL;
}

SocketObject* InSetSocket(string ip) {
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        SocketObject* hd = &socketlist[i];
        if (hd->ip == ip) {
            return hd;
        }
    }
    return NULL;
}

SocketObject* InSetSocket(int cfd) {
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        SocketObject* hd = &socketlist[i];
        if (hd->cfd == cfd) {
            return hd;
        }
    }
    return NULL;
}


//------------------------------helper functions------------------------------------//
// initialization, for server & client
void initMyAddr(const char* port){
    // myPort
    myPort = port;

    // myHostname
    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    myHostname = hostname;

    // myIP
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

// string splitor to get message instructions

void split_msg(string &src, char spt, vector<string> &dest){
    stringstream sstrm(src);
    string tmp;
    dest.clear();
    while(getline(sstrm, tmp, spt)){
        dest.push_back(tmp);
    }
}


// 我在服务器上试了下，c++有stoi()，能够直接用，不需要再写函数了

bool valid_ip(string ip_test) {
    int num = count(ip_test.begin(),ip_test.end(),'.');
    if(num != 3) return false;
    vector<string> ip_parts;
    split_msg(ip_test,'.', ip_parts);
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < ip_parts[i].length(); ++j){
            if(ip_parts[i][j] > '9' || ip_parts[i][j] < '0') return false;
        }
    }
    for(int i = 0; i < 4; ++i){
        if(stoi(ip_parts[i]) > 255) return false;
    }
    return true;
}

//----------------------------------logger------------------------------------------//
void log_ERROR(string cmd) {
    printf("[%s:ERROR]\n", cmd.c_str());
    printf("[%s:END]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_SUCCESS(string cmd) {
    printf("[%s:SUCCESS]\n", cmd.c_str());
    printf("[%s:END]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_IP(){
    const char* command = "IP";
    printf("[%s:SUCCESS]\n", command);
    printf("IP:%s\n",myIP.c_str());
    printf("[%s:END]\n", command);

    
//    cse4589_print_and_log("[%s:SUCCESS]\n", command);
//    cse4589_print_and_log("IP:%s\n", myIP.c_str());
//    cse4589_print_and_log("[%s:END]\n", command);
}
void log_AUTHOR() {
    const char* command = "AUTHOR";
    printf("[%s:SUCCESS]\n", command);
//    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    string ubit_name_2 = "";
    printf("I, %s, have read and understood the course academic integrity policy.\n", ubit_name_2.c_str());
//    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",  ubit_name_2.c_str());
    printf("[%s:END]\n", command);
//    cse4589_print_and_log("[%s:END]\n", command);
}
void log_PORT() {
    const char* command = "PORT";
    printf("[%s:SUCCESS]\n", command);
    printf("PORT:%d\n",stoi(myPort));
    printf("[%s:END]\n", command);
//    cse4589_print_and_log("[%s:SUCCESS]\n", command);
//    cse4589_print_and_log("PORT:%d\n", str_to_int(myPort));
//    cse4589_print_and_log("[%s:END]\n", command);
}
void log_LIST() {
    string cmd = "LIST";
    printf("[%s:SUCCESS]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    sort(socketlist.begin(), socketlist.end());
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        printf("%-5d%-35s%-20s%-8d\n", i + 1,
               socketlist[i].hostname.c_str(),
               socketlist[i].ip.c_str(), stoi(socketlist[i].port));
//        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1,
//                              socketlist[i].hostname.c_str(),
//                              socketlist[i].ip.c_str(), str_to_int(socketlist[i].port));
    }
    printf("[%s:END]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_EVENT(string client_ip, string msg) {
    const char* command = "RECEIVED";
    printf("[%s:SUCCESS]\n", command);
    printf("msg from:%s\n[msg]:%s\n", client_ip.c_str(), msg.c_str());
    printf("[%s:END]\n", command);
//    cse4589_print_and_log("[%s:SUCCESS]\n", command);
//    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip.c_str(), msg.c_str());
//    cse4589_print_and_log("[%s:END]\n", command);
}
void log_STATISTICS() {
    string command = "STATISTICS";
    printf("[%s:SUCCESS]\n", command.c_str());
//    cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
    sort(socketlist.begin(), socketlist.end());
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        printf("%-5d%-35s%-8d%-8d%-8s\n", i + 1,
                                      socketlist[i].hostname.c_str(),
                                      socketlist[i].num_msg_sent, socketlist[i].num_msg_rcv,
                                      socketlist[i].status.c_str());
//        cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i + 1,
//                              socketlist[i].hostname.c_str(),
//                              socketlist[i].num_msg_sent, socketlist[i].num_msg_rcv,
//                              socketlist[i].status.c_str());
    }
    printf("[%s:END]\n", command.c_str());
//    cse4589_print_and_log("[%s:END]\n", command.c_str());
}
void log_EVENTS(string from_ip, string msg, string to_ip) {
    const char* command = "RELAYED";
    printf("[%s:SUCCESS]\n", command);
    printf("msg from:%s, to:%s\n[msg]:%s\n", (const char*)from_ip.c_str(), (const char*)to_ip.c_str(), (const char*)msg.c_str());
    printf("[%s:END]\n", command);
//    cse4589_print_and_log("[%s:SUCCESS]\n", command);
//    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", (const char*)from_ip.c_str(), (const char*)to_ip.c_str(), (const char*)msg.c_str());
//    cse4589_print_and_log("[%s:END]\n", command);
}
void log_BLOCKED(string cli_ip) {
    string cmd = "BLOCED";
    if (!valid_ip(cli_ip) || InSetSocket(cli_ip) == NULL) {
        log_ERROR(cmd);
        return;
    }
    SocketObject* hd = InSetSocket(cli_ip);
    printf("[%s:SUCCESS]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    for (int i = 0; i < hd->blockeduser.size(); ++i) {
        SocketObject* new_hd = InSetSocket(hd->blockeduser[i]);
        printf("%-5d%-35s%-20s%-8s\n", i + 1, new_hd->hostname.c_str(),
                                             new_hd->ip.c_str(), new_hd->port.c_str());
//        cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1, new_hd->hostname.c_str(),
//                              new_hd->ip.c_str(), new_hd->port.c_str());
    }
    printf("[%s:END]\n", cmd.c_str());
//    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
// 我添加了一个函数，只负责log，并不实现EXIT中send() 给server message那部分
void log_EXIT(){
    printf("[%s:SUCCESS]\n", "EXIT");
    printf("[%s:END]\n", "EXIT");
//    cse4589_print_and_log("[%s:SUCCESS]\n", "EXIT");
//    cse4589_print_and_log("[%s:END]\n", "EXIT");
}


//----------------------------------clientEnd---------------------------------------//

void clientEnd(char *port){
    // client status
    bool loged_in = false;

    // client socket
    FD_ZERO(&masterfds);
    FD_SET(0, &masterfds);
    fdmax = 2;
    
    // my server/ex-server info
    string myServerIP;
    string myServerPORT;

    // save received message
    char message[BUFSIZ];
    string msg;
    vector<string> msg_buf;
    vector<string> msg_vec;

    // initialization
    initMyAddr(port);
    
    // main loop handling instructions
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
                fflush(0); // 这里不flush的话，会有bug吗？
                split_msg(msg, ' ', msg_vec);

                // for different instructions
                if(msg_vec[0] == "AUTHOR"){
                    log_AUTHOR();
                }else if(msg_vec[0] == "IP"){
                    log_IP();
                }else if(msg_vec[0] == "PORT"){
                    log_PORT();
                }else if(msg_vec[0] == "LOGOUT"){
                    msg = "LOGOUT " + myIP;
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    loged_in = false;
                    log_SUCCESS("LOGOUT");
                }else if(msg_vec[0] == "EXIT"){
                    send(sockfd, (const char*)("EXIT " + myIP).c_str(), msg.length(), 0); 
                    log_EXIT();
                    exit(0);
                }else if(msg_vec[0] == "REFRESH"){
                    send(sockfd, (const char*)("REFRESH " + myIP).c_str(), msg.length(), 0); 
                    log_SUCCESS("REFRESH");
                }else if(msg_vec[0] == "LIST"){
                    log_LIST();
                }else if(msg_vec[0] == "BLOCK"){
                    // if block user not valid or not in the list
                    if (!valid_ip(msg_vec[1]) || InSetSocket(msg_vec[1]) == NULL)
                    {
                        log_ERROR("BLOCK");
                        continue;
                    }

                    SocketObject *hd = InSetSocket(myIP);
                    if (hd == NULL)
                    {
                        msg = "BLOCK " + myIP + " " + msg_vec[1];
                        send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                        log_SUCCESS("BLOCK");
                        continue;
                    }
                    vector<string>::iterator ret;
                    ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_vec[1]);
                    if (ret != hd->blockeduser.end())
                    {
                        continue;
                    }
                    hd->blockeduser.push_back(msg_vec[1]);
                    msg = "BLOCK " + myIP + " " + msg_vec[1];
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_SUCCESS("BLOCK");
                }else if(msg_vec[0] == "UNBLOCK"){
                    if (InSetSocket(msg_vec[1]) == NULL){
                        log_ERROR("UNBLOCK");
                        continue;
                    }
                    SocketObject *hd = InSetSocket(sockfd);
                    if (hd == NULL){
                        msg = "UNBLOCK " + myIP + " " + msg_vec[1];
                        send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                        log_SUCCESS("UNBLOCK");
                    }else{
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), msg_vec[1]);
                        if (ret == hd->blockeduser.end()){
                            continue;
                        }
                        msg = "UNBLOCK " + myIP + " " + msg_vec[1];
                        send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                        log_SUCCESS("UNBLOCK");
                    }
                }else if(msg_vec[0] == "SEND"){
                    if(!valid_ip(msg_vec[1]) || InSetSocket(msg_vec[1]) == NULL){
                        log_ERROR("SEND");
                        continue;
                    }
                    msg = "SEND " + myIP + " " + msg_vec[1];
                    for(int i = 2; i < msg_vec.size(); i++){
                        msg += " " + msg_vec[i];
                    }
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_SUCCESS("SEND");
                }else if(msg_vec[0] == "BROADCAST"){
                    msg = "BROADCAST " + myIP;
                    for(int i = 1; i < msg_vec.size(); i++){
                        msg += " " + msg_vec[i];
                    }
                    send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                    log_SUCCESS("BROADCAST");
                }
            }else if(FD_ISSET(sockfd, &readfds)){
                // receive message from server
                memset(message, 0, sizeof(message));
                if(recv(sockfd, message, sizeof(message), 0) == 0){
                    // this client is closed by server
                    close(sockfd);
                    fdmax = 0; // 这里跟狗蛋写的不一样
                    loged_in = false;
                    continue;
                }

                msg = message;
                split_msg(msg, ' ', msg_vec);
                if(msg_vec[0] == "REFRESH"){
                    socketlist.clear();
                    for(int j = 1; j < (msg_vec.size() - 1); j += 3){
                        if(InSetSocket(msg_vec[j+1], msg_vec[j+2]) == NULL){
                            socketlist.push_back(*newSocketObject(-2, msg_vec[j], msg_vec[j+1], msg_vec[j+2]));
                        }
                    }
                }else if(msg_vec[0] == "SEND"){
                    msg = msg_vec[3];
                    for(int j = 4; j < msg_vec.size(); j++){
                        msg += " " + msg_vec[j];
                    }
                    log_EVENT(msg_vec[1], msg);
                }else if(msg_vec[0] == "BROADCAST"){
                    msg = msg_vec[2];
                    for(int j = 3; j < msg_vec.size(); j++){
                        msg += " " + msg_vec[j];
                    }
                    log_EVENT(msg_vec[1], msg);
                }
            }
        }else{
            // if not loged in, listen to "stdin", for instructions
            select(fdmax+1, &readfds, NULL, NULL, NULL);

            // two cases: 1) has no instruction 2) has new instruction
            // 1) no instructions, continue to listening
            if(FD_ISSET(0, &readfds) == 0){
                continue;
            }
            // 2) has new instruction
            memset(message, 0, sizeof(message));
            read(STDIN,message,sizeof(message));
            cout << message << endl;
            msg = message;

           	msg = msg.substr(0, msg.length() - 1);

            fflush(0); // 这里不flush的话，会有bug吗？
            split_msg(msg, ' ', msg_vec);
            // for different instructions
            if(msg_vec[0] == "AUTHOR"){
                log_AUTHOR();
            }else if(msg_vec[0] == "IP"){
                log_IP();
            }else if(msg_vec[0] == "PORT"){
                log_PORT();
            }else if(msg_vec[0] == "EXIT"){
                send(sockfd, (const char*)("EXIT " + myIP).c_str(), msg.length(), 0); 
                log_EXIT();
                exit(0);
            }else if(msg_vec[0] == "LOGIN"){
                // if ip is not valid, skip other operations
                if(!valid_ip(msg_vec[1])){
                    log_ERROR(msg_vec[0]);
                    continue;
                }
                
                // if ip is valid, try to connect to server
                //      1) if it is the first time, will need to connect
                //      2) if has connected to the same server, skip
                //      3) if has connected to a different server, need to re-connect
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
                            if(InSetSocket(msg_vec[j+1], msg_vec[j+2]) == NULL){
                                socketlist.push_back(*newSocketObject(-2, msg_vec[j], msg_vec[j+1], msg_vec[j+2]));
                            }
                        }
                    }else if(msg_vec[0] == "SEND"){
                        msg = msg_vec[3];
                        for(int j = 4; j < msg_vec.size(); j++){
                            msg += " " + msg_vec[j];
                        }
                        log_EVENT(msg_vec[1], msg);
                    }else if(msg_vec[0] == "BROADCAST"){
                        msg = msg_vec[2];
                        for(int j = 3; j < msg_vec.size(); j++){
                            msg += " " + msg_vec[j];
                        }
                        log_EVENT(msg_vec[1], msg);
                    }
                }

                // login success message
                log_SUCCESS("LOGIN");
            }
        }
    }
}


//----------------------------------serverEnd---------------------------------------//
void serverEnd(string server_port){
//初始化结构体
//    for(int i = 0; i<5; i++){
//        socketlist[i] =
//    }

    // variable
    fd_set global_rdfs, current_rdfs;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;

    char charmsg[BUFSIZ];
    string msg;
    vector<string> msg_p;
    struct addrinfo *ai, *p;
    //
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    int bytes = 0;
    int maxfd;
    int fdtemp,n;
    char buf[BUFSIZ],str;
    len = sizeof(struct sockaddr_in);
    const char* charPort = server_port.c_str();
    //对准备maintain的struct初始化
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
                    string msg ;
                    msg = charmsg;
//                    msg = msg.substr(0, msg.length() - 1); 不确定EoF是否需要删除
                    msg = msg.substr(0, msg.length() - 1);
                    split_msg(msg,' ',msg_p);
                    for (int i = 0; i<msg_p.size();i++){
                        cout<< msg_p[i] << endl;
                    }
                    
                    if(msg_p[0] == "LIST"){log_LIST();break;}
                    if(msg_p[0] == "STATISTICS"){log_STATISTICS();break;}
                    if(msg_p[0] == "IP"){log_IP();break;}
                    if(msg_p[0] == "AUTHOR"){log_AUTHOR();break;}
                    if(msg_p[0] == "PORT"){log_PORT();break;}
                    if(msg_p[0] == "BLOCKED"){log_BLOCKED(msg_p[1]);break;}

                }

                //创造连接
                else if(listenfd == fdtemp){
                    cout<< "entered"<<endl;
                    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr,(socklen_t*)&len))<0){
                        perror("accept error.\n");
                        exit(-1);
                    }
                    printf("receive from %s at Port %d\n",inet_ntop(AF_INET,&client_addr.sin_addr,&str, sizeof(str)),ntohs(client_addr.sin_port));
                    
                    
                    FD_CLR(fdtemp, &current_rdfs);
                    maxfd = maxfd >connfd? maxfd:connfd;
                    FD_SET(connfd,&global_rdfs);
                    
                    
                }
                //信息交流
                else{
                    /*initialize buffer to receive message*/
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
                        string host = msg_p[1];
                        string host_ip = msg_p[2];
                        string port = msg_p[3];
                        SocketObject *hd = InSetSocket(fdtemp);
                        
                        if (hd == NULL)
                        {
                            hd = newSocketObject(fdtemp, host, host_ip, port);
                            socketlist.push_back(*hd);
                        }
                        else
                        {
                            hd->status = "logged-in";
                            if (!hd->msgbuffer.empty())
                            {
                                for (vector<string>::iterator it = hd->msgbuffer.begin(); it < hd->msgbuffer.end(); it++)
                                {
                                    string mgs = *it;
                                    send(hd->cfd, (const char *)mgs.c_str(), mgs.length(), 0);
                                }
                                hd->msgbuffer.clear();
                            }
                        }
                        string message = "REFRESH";
                        for (unsigned int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].status == "logged-in")
                            {
                                message = message + blank + socketlist[i].hostname + blank + socketlist[i].ip + blank + socketlist[i].port;
                            }
                        }
                        
                        send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
                        break;}
                    
                    if(msg_p[0] == "LOGOUT"){
                        string ip_addr = msg_p[1];
                        SocketObject *hd = InSetSocket(ip_addr);
                        if (hd != NULL)
                        {
                            hd->status = "logged-out";
                        }
                        break;}
                    
                    
                    if(msg_p[0] == "EXIT"){
                        for (int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].cfd == fdtemp)
                            {
                                socketlist.erase(socketlist.begin() + i--);
                            }
                        }
                        break;}
                    
                    if(msg_p[0] == "REFRESH"){
                        string message = "REFRESH";
                        for (unsigned int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].status == "logged-in")
                            {
                                message += blank + socketlist[i].hostname + blank + socketlist[i].ip + blank +
                                socketlist[i].port;
                            }
                        }
                        send(fdtemp, message.c_str(), strlen(message.c_str()), 0);
                        break;}
                    
                    if(msg_p[0] == "BROADCAST"){
                        string from_ip = msg_p[1];
                        SocketObject *hd2 = InSetSocket(from_ip);
                        if (hd2 == NULL)
                        {
                            break;
                        }
                        string con = "255.255.255.255";
                        
                        for (int i = 0; i < socketlist.size(); ++i)
                        {
                            if (socketlist[i].ip == from_ip)
                            {
                                continue;
                            }
                            vector<string>::iterator ret;
                            ret = find(socketlist[i].blockeduser.begin(), socketlist[i].blockeduser.end(), from_ip);
                            if (ret == socketlist[i].blockeduser.end())
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
                                        message = message + blank + msg_p[m];
                                    }
                                    
                                    log_EVENTS(from_ip, message, con);
                                }
                            }
                        }
                        break;}
                    
                    if(msg_p[0] == "BLOCK"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        SocketObject *hd = InSetSocket(from_ip);
                        if (!valid_ip(to_ip) || InSetSocket(to_ip) == NULL)
                        {
                            log_ERROR("BLOCK");
                        }
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
                        if (ret == hd->blockeduser.end())
                        {
                            hd->blockeduser.push_back(to_ip);
                        }
                        else
                        {
                            log_ERROR("BLOCK");
                        }
                        break;}
                    
                    if(msg_p[0] == "UNBLOCK"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        SocketObject *hd = InSetSocket(from_ip);
                        if (!valid_ip(to_ip) || InSetSocket(to_ip) == NULL)
                        {
                            log_ERROR("UNBLOCK");
                        }
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
                        if (ret == hd->blockeduser.end())
                        {
                            log_ERROR("UNBLOCK");
                        }
                        else
                        {
                            hd->blockeduser.erase(ret);
                        }
                        break;}
                    
                    if(msg_p[0] == "SEND"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        //cout<< msg;
                        SocketObject *hd = InSetSocket(to_ip);
                        SocketObject *hd2 = InSetSocket(from_ip);
                        
                        if (hd == NULL)
                        {
                            break;
                        }
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), from_ip);
                        if (ret == hd->blockeduser.end())
                        {
                            if (hd->status == "logged-in")
                            {
                                //cout<< msg;
                                send(hd->cfd, (const char *)msg.c_str(), msg.length(), 0);//
                                hd->num_msg_rcv = hd->num_msg_rcv + 1;
                                hd2->num_msg_sent = hd2->num_msg_sent + 1;
                                string message;
                                message = msg_p[3];
                                for (int m = 4; m < msg_p.size(); m++)
                                {
                                    message = message + blank + msg_p[m];
                                }
                                log_EVENTS(from_ip, message, to_ip);
                            }
                            else
                            {
                                hd->msgbuffer.push_back(msg);
                                hd->num_msg_rcv = hd->num_msg_rcv + 1;
                                hd2->num_msg_sent = hd2->num_msg_sent + 1;
                                string message;
                                message = msg_p[3];
                                for (int m = 4; m < msg_p.size(); m++)
                                {
                                    message = message + blank + msg_p[m];
                                }
                                log_EVENTS(from_ip, message, to_ip);
                            }
                            break;
                        }
                    }
                    

                    fflush(stdout);// not sure
                }
            }
        }
    }
    
}


//---------------------------------Main Entry--------------------------------------//
int main(int argc, char **argv){
    if(argc != 3)
    {
        printf("Please enter c/s and Port number");
        exit(-1);
    }
    if(*argv[1]=='s')
    {
        string port = argv[2];
        serverEnd(port);
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
