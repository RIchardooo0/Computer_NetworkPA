#include "../include/simulator.h"
#include <numeric>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
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
// data for B side
int waitnum;
int windowsize = 1;
// set & map for A & B
//    in_window saves the seqnum in the sending window
//    if A received the ack of pkt with seqnum, then remove seqnum from in_window
//    when resend pkts, only send pkt that has seqnum inside this set
unordered_set<int> in_window;
unordered_map<int, struct pkt> rev_buff;

// parameter for protocol
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

void send_pkt(int index, int tm){
    tolayer3(0, sending_list[index]);
    in_window.insert(sending_list[index].seqnum);
    if(index == 0 || index == tm){
        starttimer(0, rtt);
    }
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    struct pkt* packet = get_packet(seqnum, acknum, message.data);
    sending_list.push_back(*packet);
    seqnum++;
    acknum++;
    if(0 < sending_list.size() && sending_list.size() <= windowsize){
        send_pkt(sending_list.size()-1, -1);
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(packet.checksum != get_check_sum(&packet)){
	return;
    }

    // struct pkt* re_pkt = inSendingWindow(packet.seqnum);
    if(packet.acknum == sending_list.front().acknum){
        stoptimer(0);
	in_window.erase(packet.seqnum);
	int count = 0;
	for(int i = 0; i < windowsize; i++){
	    if(sending_list.size() > 0 && in_window.find(sending_list.front().seqnum) == in_window.end()){
		count++;
        	sending_list.erase(sending_list.begin());
	    }
	}

	for(int i = 0; i < windowsize; i++){
	    // windowsize - count is the left pkt from erase in above for loop
	    //   we should skip them to avoid resend
	    if(i >= windowsize - count && i < sending_list.size()){
		send_pkt(i, windowsize - count);
	    }
	}
    }else if(packet.acknum > sending_list.front().acknum){
	    in_window.erase(sending_list.front().acknum);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    int i = 0;
    while( i < windowsize){
	if(in_window.find(sending_list.front().seqnum) != in_window.end()){
            send_pkt(i++, -1);
	}
    }
    
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    seqnum = 0;
    acknum = 0;
    
    sending_list.clear();
    in_window.clear();
    windowsize = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(packet.checksum != get_check_sum(&packet)){
        return;
    }
    pkt_comp com_packet = *newpkt_comp(packet.seqnum, packet.acknum, packet.payload);
    if(packet.seqnum == waitnum){
        struct pkt *ack = get_packet(packet.seqnum, packet.acknum, NULL);
        tolayer3(1, *ack);
        receiving_list.push_back(com_packet);
        sort(receiving_list.begin(), receiving_list.end());
        for(int i = 0; i < receiving_list.size(); i++){
        tolayer5(1, receiving_list[i].payload);
        waitnum++;
            }
        receiving_list.clear();
    }else if(waitnum < packet.seqnum){
        receiving_list.push_back(com_packet);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    receiving_list.clear();
    waitnum = 0;
    windowsize = getwinsize();
}
