#include "../include/simulator.h"
#include <numeric>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
using namespace std;

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
// data for A side
int seqnum;
int acknum;
bool can_send = true;

// data for B side
int waitnum;

// parameter for protocol
float rtt = 22;
vector<pkt> sending_list;

// helper functions
int get_check_sum(struct pkt* packet){
	int s = packet->acknum + packet->seqnum;
	for (int i = 0; i < 20;i++) {
		s += packet->payload[i];
	}
	return s;
}

static pkt* get_packet(int seqnum,int acknum, char* data){   
	struct pkt* packet = new pkt();
    packet->acknum = acknum;
    packet->seqnum = seqnum;
	if(data != NULL){
    	strncpy(packet->payload, data, 20);
	}else{
    	memset(packet->payload, 0, 20);
	}
    packet->checksum = get_check_sum(packet);
    return packet;
}

void send_pkt(){
	tolayer3(0, sending_list.front());
	starttimer(0, rtt);
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	struct pkt* packet = get_packet(seqnum, acknum, message.data);
	sending_list.push_back(*packet);
	seqnum++;
	if(can_send){
		can_send = false;
		send_pkt();
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(packet.checksum == get_check_sum(&packet)
		&& packet.acknum == acknum && sending_list.front().seqnum == packet.seqnum){
		stoptimer(0);
		sending_list.erase(sending_list.begin());
		if(sending_list.size() > 0){
			send_pkt();
		}
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	send_pkt();
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	seqnum = 0;
	acknum = 1;
	sending_list.clear();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	if(packet.checksum != get_check_sum(&packet)){
		return;
	}
	struct pkt *ack = get_packet(packet.seqnum, 1, NULL);
	tolayer3(1, *ack);
	if(packet.seqnum == waitnum){
		tolayer5(1, packet.payload);
		waitnum++;
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    waitnum = 0;
}
