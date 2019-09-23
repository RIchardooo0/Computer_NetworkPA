#include "header.h"

using namespace std;

#define SERV_PORT 6666
#define STDIN 0
#define RT_ERR -1


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


//------------------------------helper functions------------------------------------//
// initialization, for server & client
void initMyAddr(const char *port){
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
        inet_ntop(AF_INET, addr, ip, sizeof(ip));
        myIP = ip;
    }

    // hints & myAddrInfo & sockfd
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, (const char*)myPort.c_str(), &myAddrInfo); // here change myPort back to c style string
    sockfd = socket(myAddrInfo-ai_family, myAddrInfo->ai_socktype, myAddrInfo->ai_protocol);
    bind(sockfd, myAddrInfo->ai_addr, myAddrInfo->ai_addrlen);
    freeaddrinfo(myAddrInfo);
}


//----------------------------cse4589_print_and_log---------------------------------//
// packaged functions for log
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
	string ubit_name_2 = "xingyuya";
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",  ubit_name_2.c_str());
	cse4589_print_and_log("[%s:END]\n", command);
}
void log_PORT() {
	const char* command = "PORT";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("PORT:%d\n", myPORT);
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
	const char* command = "EVENT";
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
	const char* command = "EVENT";
	cse4589_print_and_log("[%s:SUCCESS]\n", command);
	cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from_ip, to_ip, msg);
	cse4589_print_and_log("[%s:END]\n", command);
}
void log_BLOCKED(string cli_ip) {
		string cmd = "BLOCKED";
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


//----------------------------------clientEnd---------------------------------------//
void clientEnd(char *port){
    // client data
    bool loged_in = false;
    FD_ZERO(&masterfds);
    FD_SET(0, &masterfds);
    fdmax = 0;

    // initialization
    initMyAddr(port);
    
    // main loop handling instructions
    while(true){
        readfds = masterfds;

        // two cases: loged in or not
        if(loged_in){
            cout << "Handle Loged In!" << endl;
        }else{
            select(fdmax+1, &readfds, NULL, NULL, NULL);
            if(FD_ISSET(0, &readfds)){
            }
            cout << "Please Login First!" << endl;
        }
    }
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
int create_serv_socket(int port);

int main(int argc, char **argv){
    if(argc != 3)
    {
    cout<<"Please enter c/s and Port number"<<endl;
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
        cout<<"System out"<<endl;
        exit(-1);
    }
    return 0;
}

void clientEnd(int client_port){
    
    printf("hello %d",client_port);
    
//    while(1){
//        cin.clear();
//        cin.sync();
//    }
}


void serverEnd(int server_port){
    
    fd_set global_rdfs, current_rdfs;
    
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;
    int bytes = 0;
    int maxfd;
    int i;
    char buf[BUFSIZ],str;
    len = sizeof(struct sockaddr_in);
    
    //对准备maintain的struct初始化
    
    listenfd = create_serv_socket(server_port);
    FD_ZERO(&global_rdfs);
    FD_ZERO(&current_rdfs);
    FD_SET(listenfd, &global_rdfs);
    FD_SET(STDIN, &global_rdfs);
    maxfd = listenfd;
    
    while(1){
        cin.clear();
        cin.sync();
        current_rdfs = global_rdfs;
        if(select(maxfd+1, &current_rdfs,NULL,NULL,NULL)<0){
            perror("select error.\n");
            exit(-1);
        }
        for(i = 0;i <= maxfd; i++){
            if(FD_ISSET(i,&current_rdfs)){
                if(listenfd == i){
                    if((connfd = accept(listenfd, (struct sockaddr*)&client_addr,(socklen_t*)&len))<0){
                        perror("accept error.\n");
                        exit(-1);
                    }
                    printf("receive from %s at Port %d\n", inet_ntop(AF_INET, &client_addr.sin_addr,&str, sizeof(str)),
                           ntohs(client_addr.sin_port));
                    FD_CLR(i, &current_rdfs);
                    maxfd = maxfd >connfd? maxfd:connfd;
                    FD_SET(connfd,&global_rdfs);
                }else{
                    
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
                    printf("buf:%s\n", buf);
                    send(i, buf, strlen(buf),0);
                }
            }
        }
    }
    
}



int create_serv_socket(int port){
    int fd;
    struct sockaddr_in my_addrs;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket error.\n");
        exit(-1);
    }else{
        printf("Socket created");
    }
    bzero(&my_addrs, sizeof(my_addrs));
    my_addrs.sin_family = AF_INET;
    my_addrs.sin_port = htons(port);
    my_addrs.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(fd, (struct sockaddr*)&my_addrs, sizeof(struct sockaddr_in)) != 0){
        perror("Binding error.\n");
        exit(-1);
    }else{
        printf("Binding successful");
    }
    if(listen(fd, 128)<0){
        perror("listen failed");
    }else{
        printf("listen created");
    }
    
    return fd;
}

int create(word){
    printf(word);
}
/*

int main(void){
    
    int lfd, cfd;
    int client[FD_SETSIZE] //1024
    struct sockaddr_in serv_addr, clie_addr;
    socklen_t clie_addr_len, clie_IP_len;
    char buf[BUFSIZ], clie_IP[BUFSIZ],str[INET_ADDRSTRLEN];
    int i,n,ret;
    
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1){
        perror("socket error");
        exit(1);
    }
    
    
    bzero(&serv_addr,sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    ret = bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(ret == -1){
        perror("bind error");
        exit(1);
    }
    ret = listen(lfd, 128);
    if(ret == -1){
        perror("listen error");
        exit(1);
    }
    clie_addr_len = sizeof(clie_addr);
    cfd = accept(lfd, (struct sockaddr *)&clie_addr,&clie_addr_len);
    if(cfd == -1){
        perror("accept error");
        exit(1);
    }
    
    printf("client IP: %s, client port: %d\n",
           inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr,clie_IP, sizeof(clie_IP_len)),
           ntohs(clie_addr.sin_port));
    
    while(1){
        n = read(cfd, buf, sizeof(buf));
        for (i = 0; i <n ; i++)
        buf[i] = toupper(buf[i]);
        write(cfd, buf, n);
        
    }
    
    close(lfd);
    close(cfd);
    
    return 0;
}
*/
