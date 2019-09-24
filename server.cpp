//#include "header.h"


using namespace std;

#define SERV_PORT 6666
#define STDIN 0
#define RT_ERR -1
#define BACKLOG 5
//-----------------------------frequently used data---------------------------------//
string myHostname;
string myPort;
string myIP;
int sockfd;
struct addrinfo *myAddrInfo;
struct addrinfo hints;


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
    struct hostent *ht = gethostbyname(myHostname);
    for(int i = 0; ht->h_addr_list[i] != 0; i++){
        void *addr;
        char ip[INET_ADDRSTRLEN];
        addr = &(ht->h_addr_list[i]);
        inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);// 16 定义在netinet/in.h头文件中
        myIP = ip;
    }

    // hints & myAddrInfo & sockfd
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, (const char*)myPort.c_str(), &hints,&myAddrInfo); // here change myPort back to c style string
    sockfd = socket(myAddrInfo->ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
    bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen);
    freeaddrinfo(myAddrInfo);
}


//struct block_list{
//
//} ;
//struct block_list client1;
//
//vector<struct block_list> bl_list;
/*
 初始完值过后用bl_list.push_back(client_block)
 */
void serverEnd(int server_port);
void clientEnd(int client_port);
//int create_serv_socket(int port);

//struct SocketObject{
//    int cfd;
//    string hostname;
//    string ip;
//    string port;
//    int num_msg_sent;
//    int num_msg_rcv;
//    string status;
//    vector<string> blockeduser;
//    vector<string> msgbuffer;
//
//    bool operator<(const SocketObject &rhs) const {
//        return atoi(port.c_str()) < atoi(rhs.port.c_str());
//    }
//};
//vector<SocketObject> socketlist;
//
//SocketObject* setSocketObject(int cfd, string hostname, string ip, string port){
//    SocketObject* info = new SocketObject;
//    info->cdf = cfd;
//    info->hostname = hostname;
//    info->ip = ip;
//    info->port = port;
//    info->num_msg_sent = 0;
//    info->num_msg_rcv = 0;
//    info->status = "logged-in";
//
//    return info;
//
//}

void clientEnd(int client_port){

    printf("hello %d",client_port);

//    while(1){

//    }
}


void serverEnd(int server_port){
//初始化结构体
//    for(int i = 0; i<5; i++){
//        socketlist[i] =
//    }

    // variable
    fd_set global_rdfs, current_rdfs;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;

    string charmsg;
    string msg;
    vector<string> msg_p;
    struct addrinfo *ai, *p;
    //
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    int bytes = 0;
    int maxfd;
    int i;
    char buf[BUFSIZ],str;
    len = sizeof(struct sockaddr_in);
    
    //对准备maintain的struct初始化
    initMyAddr(server_port);
    listen(sockfd,BACKLOG)<0);
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
                    read(STDIN,charmsg,sizeof(charmsg));
                    fflush(STDIN);
                    string msg ;
                    msg = charmsg;
                    int choice;
                    if(msg_p[0] == "LIST"){choice = 1;}
                    if(msg_p[0] == "STATISTICS"){choice = 2;}
                    if(msg_p[0] == "IP"){choice = 3;}
                    if(msg_p[0] == "AUTHOR"){choice = 4;}
                    if(msg_p[0] == "PORT"){choice = 5;}
                    if(msg_p[0] == "BLOCKED"){choice = 6;}
                    switch(choice){
                        case 1:{}
                        case 2:{}
                        case 3:{}
                        case 4:{}
                        case 5:{}
                        case 6:{}

                    }
//                    cout<<msg<<endl;

                }

                //创造连接
                else if(listenfd == i){
                    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr,(socklen_t*)&len))<0){
                        perror("accept error.\n");
                        exit(-1);
                    }
                        printf("receive from %s at Port %d\n",inet_ntop(AF_INET, &client_addr.sin_addr,&str, sizeof(str)),
                           ntohs(client_addr.sin_port));
                    FD_CLR(i, &current_rdfs);
                    maxfd = maxfd >connfd? maxfd:connfd;
                    FD_SET(connfd,&global_rdfs);
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



//int create_serv_socket(int port){
//
//    int fd;
//    struct sockaddr_in my_addrs;
//
//
//    // myHostname
//    char hostname[1024];
//    gethostname(&hostname, sizeof(hostname));
//    myHostname = hostname;
//
//    // myIP
//    struct hostent *ht = gethostbyname(myHostname);
//    for(int i = 0; ht->h_addr_list[i] != 0; i++){
//        void *addr;
//        char ip[INET_ADDRSTRLEN];
//        addr = &(ht->h_addr_list[i]);
//        inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);// 16 定义在netinet/in.h头文件中
//        myIP = ip;
//    }
//    printf("this is my ip address %s",myIP);
//    fd = socket(AF_INET, SOCK_STREAM, 0);
//    if(fd < 0){
//        perror("socket error.\n");
//        exit(-1);
//    }else{
//        printf("Socket created\n");
//    }
//    bzero(&my_addrs, sizeof(my_addrs));
//    my_addrs.sin_family = AF_INET;
//    my_addrs.sin_port = htons(port);
////    my_addrs.sin_addr.s_addr = htonl(INADDR_ANY);
//    inet_pton(AF_INET, myIP, &my_addrs.sin_addr.s_addr);
//
//    if(bind(fd, (struct sockaddr*)&my_addrs, sizeof(struct sockaddr_in)) != 0){
//        perror("Binding error.\n");
//        exit(-1);
//    }else{
//        printf("Binding successful\n");
//    }
//    if(listen(fd, BACKLOG)<0){
//        perror("listen failed\n");
//    }else{
//        printf("listen created\n");
//    }
//
//    return fd;
//}

int main(int argc, char **argv){
    if(argc != 3)
    {
        printf("Please enter c/s and Port number");
        exit(-1);
    }
    if(*argv[1]=='s')
    {
        serverEnd(atoi(argv[2]));
    }
    else if(*argv[1]=='c')
    {
        clientEnd(atoi(argv[2]));
    }
    else{
        printf("System out");
        exit(-1);
    }
    return 0;
}

