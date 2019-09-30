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

/*
 
//-----------------------------string processing------------------------------------//



//----------------------------------logger------------------------------------------//
void log_ERROR(string cmd) {
    cse4589_print_and_log("[%s:ERROR]\n", cmd.c_str());
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
    
    // save received message
    char message[BUFSIZ];
    string msg;
    vector<string> msg_vec;

    // initialization
    initMyAddr(port);
    
    // main loop handling instructions
    // while(true){
        // copy fds
        readfds = masterfds;

        // two cases: loged in or not
        if(loged_in){
            // if already loged in
            cout << "Handle Loged In!" << endl;
        }else{
            // if not loged in, listen to "stdio", for instructions
            select(fdmax+1, &readfds, NULL, NULL, NULL);

            // two cases: 1) has new instruction 2) no instruction
            if(FD_ISSET(0, &readfds)){
                // 1) has new instruction
                recv(0, message, BUFSIZ, 0);
                msg = message; // 这里到底有没有特殊符号？到底要不要截取？
                split_msg(msg, ' ', msg_vec);
                fflush(0); // 这里不flush的话，会有bug吗？
                
                // for different instructions
                if(msg_vec[0] == "AUTHOR"){
                    log_AUTHOR();
                }
                if(msg_vec[0] == "IP"){
                    log_IP();
                }
                if(msg_vec[0] == "PORT"){
                    log_PORT;
                }
                if(msg_vec[0] == "EXIT"){
                    send(sockfd, (const char*)("EXIT " + myIP).c_str(), msg.length(), 0); 
                    log_EXIT();
                }
                if(msg_vec[0] == "LOGIN"){

                }
            }else{
                // 2) no instructions, continue to listening
                continue;
            }
            cout << "Please Login First!" << endl;
        }
    // }
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

                    split_msg(msg,' ',msg_p);
                    for (int i = 0; i<msg_p.size();i++){
                        cout<< msg_p[i] << endl;
                    }
                    
//                    if(msg_p[0] == "LIST"){log_LIST();break;}
//                    if(msg_p[0] == "STATISTICS"){log_STATISTICS();break;}
//                    if(msg_p[0] == "IP"){log_IP();break;}
//                    if(msg_p[0] == "AUTHOR"){log_AUTHOR();break;}
//                    if(msg_p[0] == "PORT"){log_PORT();break;}
//                    if(msg_p[0] == "BLOCKED"){log_BLOCKED(msg_p[1]);break;}

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
                        FD_CLR(i, &global_rdfs);
                        close(i);
                        continue;
                    }
                    printf("Client sent me buf:%s\n", msg);
                    printf("Echoing it backt to the remote host ...");
                    msg = charmsg;
                    split_msg(msg," ",msg_p);
                    
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
                            log_Error("BLOCK");
                        }
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
                        if (ret == hd->blockeduser.end())
                        {
                            hd->blockeduser.push_back(to_ip);
                        }
                        else
                        {
                            log_Error("BLOCK");
                        }
                        break;}
                    
                    if(msg_p[0] == "UNBLOCK"){
                        string from_ip = msg_p[1];
                        string to_ip = msg_p[2];
                        SocketObject *hd = InSetSocket(from_ip);
                        if (!valid_ip(to_ip) || InSetSocket(to_ip) == NULL)
                        {
                            log_Error("UNBLOCK");
                        }
                        vector<string>::iterator ret;
                        ret = find(hd->blockeduser.begin(), hd->blockeduser.end(), to_ip);
                        if (ret == hd->blockeduser.end())
                        {
                            log_Error("UNBLOCK");
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
