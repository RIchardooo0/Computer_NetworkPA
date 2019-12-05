#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <iostream>

int get_ctrl_sock(uint16_t ctrl_port_num);
