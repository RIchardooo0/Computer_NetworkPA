#include <util.h>

#include <vector>
#include <iostream>

using namespace std;


/* Define Variables */
int max_fd;
fd_set master_set;
fd_set listen_set;


/* Start & Run*/
void start(uint16_t ctrl_port_num){
    /* clean socket set */
    FD_ZERO(&master_set);
    FD_ZERO(&listen_set);

    /* get connection socket */
    ctrl_sock = get_ctrl_sock(ctrl_port_num);
    FS_SER(ctrl_port_num, &master_set);
    max_fd = ctrl_port_num;

    /* enterning main loop */

}
