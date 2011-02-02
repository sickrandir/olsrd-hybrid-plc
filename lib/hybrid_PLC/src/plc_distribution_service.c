#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "olsr.h"
#include "ipcalc.h"
#include "net_olsr.h"
#include "mantissa.h"
#include "scheduler.h"
#include "parser.h"

#include "plc_distribution_service.h"
#include "linklayer_plc_data.h"

int distribution_service_init(void);
bool olsr_parse_plc_msg(union olsr_message *, struct interface *, union olsr_ip_addr *);
void olsr_send_plc_msg();
bool olsr_parse_plc_msg();

uint8_t *mac;
static double my_timeout = PLC_VALID_TIME;




int distribution_service_init(void) {
  olsr_printf(3, "\nPLC DISTRIBUTION SERVICE: init\n");
  /* register functions with olsrd */
  olsr_parser_add_function(&olsr_parse_plc_msg, PARSER_TYPE);
  /* periodic message generation */
  olsr_start_timer(EMISSION_INTERVAL * MSEC_PER_SEC, EMISSION_JITTER, OLSR_TIMER_PERIODIC, &olsr_send_plc_msg, NULL, 0);
  mac = get_own_plc_mac();
  olsr_printf(3, "\nOWN MAC PLC:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return 1;
}

void olsr_send_plc_msg() {
   olsr_printf(3, "\nPLC DISTRIBUTION SERVICE: olsr_send_plc_msg\n");
   unsigned char buffer[2000];
   union olsr_message *message = (union olsr_message *)buffer;
   struct interface *ifn;
   int size;

   /* fill message */
   if (olsr_cnf->ip_version == AF_INET) {
     /* IPv4 */
     message->v4.olsr_msgtype = MESSAGE_TYPE;
     message->v4.olsr_vtime = reltime_to_me(my_timeout * MSEC_PER_SEC);
     memcpy(&message->v4.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
     message->v4.ttl = 1;
     message->v4.hopcnt = 0;
     message->v4.seqno = htons(get_msg_seqno());

     memcpy(ARM_NOWARN_ALIGN(&message->v4.message), mac, 8);
     size = 8 + sizeof(struct olsrmsg);
     message->v4.olsr_msgsize = htons(size);
   } else {
     /* IPv6 */
     message->v6.olsr_msgtype = MESSAGE_TYPE;
     message->v6.olsr_vtime = reltime_to_me(my_timeout * MSEC_PER_SEC);
     memcpy(&message->v6.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
     message->v6.ttl = MAX_TTL;
     message->v6.hopcnt = 0;
     message->v6.seqno = htons(get_msg_seqno());

     memcpy(ARM_NOWARN_ALIGN(&message->v6.message), mac, 8);
     size = 8 + sizeof(struct olsrmsg6);
     message->v6.olsr_msgsize = htons(size);
   }

   /* looping trough interfaces */
   for (ifn = ifnet; ifn; ifn = ifn->int_next) {
     OLSR_PRINTF(3, "\nPLC DISTRIBUTION SERVICE: Generating packet\n");
     if (ifn->mode != IF_MODE_PLC)
       continue;
     if (net_outbuffer_push(ifn, message, size) != size) {
       /* send data and try again */
       net_output(ifn);
       if (net_outbuffer_push(ifn, message, size) != size) {
         OLSR_PRINTF(3, "\nPLC DISTRIBUTION SERVICE: could not send on interface: %s\n", ifn->int_name);
       }
     }
   }

}


  /**
 * Parse name olsr message of NAME type
 */
bool olsr_parse_plc_msg(union olsr_message *m, struct interface *in_if __attribute__ ((unused)), union olsr_ip_addr *ipaddr) {
  union olsr_ip_addr originator;
  olsr_reltime vtime;
  int size;
  uint16_t seqno;
  void * message;
  uint8_t recv_mac[6];
  struct ipaddr_str ipbuf;

  /* Fetch the originator of the messsage */
  if (olsr_cnf->ip_version == AF_INET) {
    memcpy(&originator, &m->v4.originator, olsr_cnf->ipsize);
    seqno = ntohs(m->v4.seqno);
  } else {
    memcpy(&originator, &m->v6.originator, olsr_cnf->ipsize);
    seqno = ntohs(m->v6.seqno);
  }

  /* Fetch the message based on IP version */
  if (olsr_cnf->ip_version == AF_INET) {
    vtime = me_to_reltime(m->v4.olsr_vtime);
    size = ntohs(m->v4.olsr_msgsize);
    message = ARM_NOWARN_ALIGN(&m->v4.message);
    memcpy(recv_mac, message, 6);
  } else {
    vtime = me_to_reltime(m->v6.olsr_vtime);
    size = ntohs(m->v6.olsr_msgsize);
    message = ARM_NOWARN_ALIGN(&m->v6.message);
    memcpy(recv_mac, message, 6);
  }

  olsr_printf(3, "\nInserisco in linklayer_plc_data:\n");
  olsr_printf(3, "\nIndirizzo ip: %s\n", olsr_ip_to_string(&ipbuf, &originator));
  olsr_printf(3, "\nMAC PLC:%02x:%02x:%02x:%02x:%02x:%02x\n", recv_mac[0], recv_mac[1], recv_mac[2], recv_mac[3], recv_mac[4], recv_mac[5]);

  insert_plc_peer_neighbor(&originator, recv_mac);
  return true;
}
