#include "../include/simulator.h"
#include <numeric>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
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
int seqnum = 0;
int acknum = 0;
int waitnum = 0;
int WINDOWSIZE = 1;
float RTT = 15;

vector<pkt> waiting_list;
vector<pkt> received_list;
vector<pkt> sending_list;

int get_check_sum(struct pkt* packet){
    int s = packet->acknum + packet->seqnum;
    for (int i = 0; i < 20;i++) {
        s += packet->payload[i];
    }
    return s;
}

static pkt* in_list(int seqnum, vector<pkt> &list) {
  for (unsigned int i = 0; i < list.size(); ++i) {
    pkt* cur = &list[i];
    if (cur->seqnum == seqnum) {
      return cur;
    }
  }
  return NULL;
}

void remove_pkt(int seqnum, vector<pkt> &list) {
    vector<pkt>::iterator cur;
    for (cur = list.begin(); cur != list.end(); cur++)
    {
        if (seqnum == cur->seqnum)
        {
            list.erase(cur); 
            return;  
        }
    }
}

void send_next_pkt(){
	struct pkt cur = waiting_list.back();
    sending_list.push_back(cur);
    tolayer3(0, cur);
    waiting_list.pop_back();
    starttimer(0,RTT);
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

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message){
   waiting_list.insert(waiting_list.begin(),*get_packet(seqnum,acknum,message.data));
   seqnum = seqnum + 1;
   if(sending_list.size() < WINDOWSIZE && waiting_list.size() >0){
   		send_next_pkt();
   }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet){
	struct pkt* cur = in_list(packet.seqnum, sending_list);
	if(packet.checksum == get_check_sum(&packet) && cur){
        remove_pkt(packet.seqnum, sending_list);
        if(sending_list.size() == 0 && waiting_list.size() == 0){
            stoptimer(0);
        }
        if(sending_list.size() < WINDOWSIZE && waiting_list.size() > 0){
            send_next_pkt();
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(){
    tolayer3(0, sending_list.front());
    starttimer(0, RTT);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(){
    seqnum = 0;
    acknum = 1;
    waiting_list.clear();
    sending_list.clear();
    WINDOWSIZE= getwinsize();
}


/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet){
  if (packet.checksum != get_check_sum(&packet)){
    return;
  }
 
  struct pkt *ack = get_packet(packet.seqnum, 1, NULL);
  tolayer3(1, *ack);

  if(in_list(packet.seqnum, received_list) || packet.seqnum < waitnum){
    return;
  }

  received_list.push_back(packet);

  while(!received_list.empty()){
    vector<pkt>::iterator cur;
    int minseq = received_list.begin()->seqnum;
    for (cur = received_list.begin(); cur != received_list.end(); cur++)
    {
        minseq = cur->seqnum < minseq ? cur->seqnum : minseq;
    }
    if(minseq == waitnum){
        struct pkt* rmpkt = in_list(minseq, received_list);
        tolayer5(1, rmpkt->payload);
        remove_pkt(minseq, received_list);
        waitnum += 1;
    }else{
        break;
    }
  }
}


/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(){
    waitnum = 0;
    received_list.clear();
}

