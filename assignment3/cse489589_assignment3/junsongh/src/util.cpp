#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// for connection_manager
int get_ctrl_sock(uint16_t ctrl_port_num){
    int sk;
    struct sockaddr_in ctrl_addr;
    socklen_t addr_len = sizeof(ctrl_addr);

    sk = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    bzero(&ctrl_addr, sizeof ctrl_addr);

    ctrl_addr.sin_family = AF_INET;
    ctrl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ctrl_addr.sin_port = htons(ctrl_port_num);
    
    return sk;
}
