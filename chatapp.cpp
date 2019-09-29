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
int sockfd;
struct addrinfo *myAddrInfo;
struct addrinfo hints;

fd_set masterfds;
fd_set readfds;
int fdmax;

/*
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

SocketObject* setSocketObject(int cfd, string hostname, string ip, string port){
    SocketObject* info = new SocketObject;
    info->cdf = cfd;
    info->hostname = hostname;
    info->ip = ip;
    info->port = port;
    info->num_msg_sent = 0;
    info->num_msg_rcv = 0;
    info->status = "logged-in";

    return info;

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
*/

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
    bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen);
    
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

/*
 
//-----------------------------string processing------------------------------------//



//----------------------------------logger------------------------------------------//
void log_ERROR(string cmd) {
    cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_SUCCESS(string cmd) {
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_IP(){
    const char* command = "IP";
    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    cse4589_print_and_log("IP:%s\n", myIP.c_str());
    cse4589_print_and_log("[%s:END]\n", command);
}
void log_AUTHOR() {
    const char* command = "AUTHOR";
    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    //string ubit_name_1 = "";
    string ubit_name_2 = "";
    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",  ubit_name_2.c_str());
    cse4589_print_and_log("[%s:END]\n", command);
}
void log_PORT() {
    const char* command = "PORT";
    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    cse4589_print_and_log("PORT:%d\n", str_to_int(myPORT));
    cse4589_print_and_log("[%s:END]\n", command);
}
void log_LIST() {
    string cmd = "LIST";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    sort(socketlist.begin(), socketlist.end());
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1,
                              socketlist[i].hostname.c_str(),
                              socketlist[i].ip.c_str(), str_to_int(socketlist[i].port));
    }
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
void log_EVENT(string client_ip, string msg) {
    const char* command = "RECEIVED";
    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip.c_str(), msg.c_str());
    cse4589_print_and_log("[%s:END]\n", command);
}
void log_STATISTICS() {
    string command = "STATISTICS";
    cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
    sort(socketlist.begin(), socketlist.end());
    for (unsigned int i = 0; i < socketlist.size(); ++i) {
        cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i + 1,
                              socketlist[i].hostname.c_str(),
                              socketlist[i].num_msg_sent, socketlist[i].num_msg_rcv,
                              socketlist[i].status.c_str());
    }
    cse4589_print_and_log("[%s:END]\n", command.c_str());
}
void log_EVENTS(string from_ip, string msg, string to_ip) {
    const char* command = "RELAYED";
    cse4589_print_and_log("[%s:SUCCESS]\n", command);
    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", (const char*)from_ip.c_str(), (const char*)to_ip.c_str(), (const char*)msg.c_str());
    cse4589_print_and_log("[%s:END]\n", command);
}
void log_BLOCKED(string cli_ip) {
    string cmd = "BLOCED";
    if (!valid_ip(cli_ip) < 0 || InSetSocket(cli_ip) == NULL) {
        log_Error(cmd);
        return;
    }
    SocketObject* hd = InSetSocket(cli_ip);
    
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd.c_str());
    for (int i = 0; i < hd->blockeduser.size(); ++i) {
        SocketObject* new_hd = InSetSocket(hd->blockeduser[i]);
        cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", i + 1, new_hd->hostname.c_str(),
                              new_hd->ip.c_str(), new_hd->port.c_str());
    }
    cse4589_print_and_log("[%s:END]\n", cmd.c_str());
}
// 我添加了一个函数，只负责log，并不实现EXIT中send() 给server message那部分
void log_EXIT(){
    cse4589_print_and_log("[%s:SUCCESS]\n", "EXIT");
    cse4589_print_and_log("[%s:END]\n", "EXIT");
}
//###############################################################
*/


//----------------------------------clientEnd---------------------------------------//
void clientEnd(char *port){
    
    // client status
    bool loged_in = false;

    // client socket
    FD_ZERO(&masterfds);
    FD_SET(0, &masterfds);
    fdmax = 0;
    
    // my server/ex-server info
    string myServerIP;
    string myServerPORT;

    // save received message
    char message[BUFSIZ];
    string msg;
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
            cout << "Handle Loged In!" << endl;
        }else{
            // if not loged in, listen to "stdio", for instructions
            select(fdmax+1, &readfds, NULL, NULL, NULL);

            // two cases: 1) has no instruction 2) has new instruction
            // 1) no instructions, continue to listening
            if(FD_ISSET(0, &readfds) == 0){
                continue;
            }

            // 2) has new instruction
            recv(0, message, BUFSIZ, 0);
            msg = message; // 这里到底有没有特殊符号？到底要不要截取？
            split_msg(msg, ' ', msg_vec);
            fflush(0); // 这里不flush的话，会有bug吗？
            
            // for different instructions
            if(msg_vec[0] == "AUTHOR"){
                log_AUTHOR();
            }else if(msg_vec[0] == "IP"){
                log_IP();
            }else if(msg_vec[0] == "PORT"){
                log_PORT;
            }else if(msg_vec[0] == "EXIT"){
                send(sockfd, (const char*)("EXIT " + myIP).c_str(), msg.length(), 0); 
                log_EXIT();
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

                    // if can't reach server, continue
                    if(getaddrinfo(myServerIP, myServerPORT, &hints, &serverInfo) != 0){
                        myServerIP = "";
                        myServerPORT = "";
                        freeaddrinfo(serverInfo);   // 这里多添加了几次，在各种情况下都free
                        log_ERROR(msg_vec[0]);
                        continue;
                    }

                    // try to connect to server
                    for(p = serverInfo; p != null; p = p->ai_next){
                        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == 0){
                            break;
                        }else{
                            close(sockfd);
                        }
                    }

                    // no successful connection, continue
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
                msg = "LOGIN " + myHostname + " " + myIP + " " + myPORT;
                send(sockfd, (const char *)msg.c_str(), msg.length(), 0);
                loged_in = true;
                log_SUCCESS(msg_vec[0]);
                
                // loged in, but receive new messages at here???
                //    or do it in new loop???
                //
                // do REFRESH as a single function, and use it here?
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
    int i,n;
    char buf[BUFSIZ],str;
    len = sizeof(struct sockaddr_in);
    const char* charPort = server_port.c_str();
    //对准备maintain的struct初始化
    initMyAddr(charPort);
    listen(sockfd,BACKLOG);
    listenfd = sockfd;

    FD_ZERO(&global_rdfs);
    FD_ZERO(&current_rdfs);
    FD_SET(listenfd, &global_rdfs);
    FD_SET(STDIN, &global_rdfs);
    maxfd = listenfd;
    
    while(1){
        fflush(stdout);
        current_rdfs = global_rdfs;
        if(select(maxfd+1, &current_rdfs,NULL,NULL,NULL)<0){
            perror("select error.\n");
            exit(-1);
        }else if(select(maxfd+1, &current_rdfs,NULL,NULL,NULL) == 0 ){
            perror("Select time out.\n");
            exit(-1);
        }

        for(i = 0;i <= maxfd; i++){
            if(FD_ISSET(i,&current_rdfs)){

                //键盘输入
                if(STDIN == i){
                    memset(&charmsg[0], 0, sizeof(charmsg));
                    n = read(STDIN,charmsg,sizeof(charmsg));
                    fflush(STDIN);
                    string msg ;
                    msg = charmsg;
                    
//                    msg = msg.substr(0, msg.length() - 1); 不确定EoF是否需要删除
//                    for(i = 0;i<n;i++)
//                        charmsg[i] = toupper(charmsg[i]);
//                    printf("%s",charmsg);
//                    int choice;
                    split_msg(msg,' ',msg_p);
                    for (int i = 0; i<msg_p.size();i++){
                        cout<< i << endl;
                    }
                    
//                    if(msg_p[0] == "LIST"){log_LIST();break;}
//                    if(msg_p[0] == "STATISTICS"){log_STATISTICS();break;}
//                    if(msg_p[0] == "IP"){log_IP();break;}
//                    if(msg_p[0] == "AUTHOR"){log_AUTHOR();break;}
//                    if(msg_p[0] == "PORT"){log_PORT();break;}
//                    if(msg_p[0] == "BLOCKED"){log_BLOCKED(msg_p[1]);break;}

                }

                //创造连接
                else if(listenfd == i){
                    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr,(socklen_t*)&len))<0){
                        perror("accept error.\n");
                        exit(-1);
                    }
                    printf("receive from %s at Port %d\n",inet_ntop(AF_INET,&client_addr.sin_addr,&str, sizeof(str)),ntohs(client_addr.sin_port));
                    
                    
                    FD_CLR(i, &current_rdfs);
                    maxfd = maxfd >connfd? maxfd:connfd;
                    FD_SET(connfd,&global_rdfs);
                    recv(connfd, charmsg, sizeof(charmsg),0);
                    msg = charmsg;
//                    split_msg(msg," ",msg_p);
                    
//                    switch(str_to_int(msg_p[0])){
//                        case 1:{
//
//                        }
//                        case 2:{
//
//                        }
//                    }
                    
                }
                //信息交流
                else{
                    /*initialize buffer to receive message*/
                    bytes = recv(i, buf, BUFSIZ, 0 );
                    if(bytes<0){
                        perror("recv error.\n");
                        exit(-1);
                    }
                    if(bytes == 0){//退出
                        FD_CLR(i, &global_rdfs);
                        close(i);
                        continue;
                    }
                    printf("Client sent me buf:%s\n", buf);
                    printf("Echoing it backt to the remote host ...");
                    if(send(i, buf, strlen(buf),0) == strlen(buf)){
                        cout<<"done!\n"<<endl;
                    }
                    fflush(stdout);
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
