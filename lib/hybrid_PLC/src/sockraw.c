#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <net/ethernet.h>

//#include <linux/if_packet.h>
//#include <linux/if_ether.h>
//#include <linux/if_arp.h>
#include <pcap.h>

#include "/usr/local/include/faifa/homeplug.h"
#include "/usr/local/include/faifa/homeplug_av.h"
#include "/usr/local/include/faifa/endian.h"

#include "sockraw.h"
#include "linklayer_plc_data.h"

int init_plc_communication(char *);
static int send_frame_pcap(uint8_t *, int);
static int recv_frame_pcap(void);
static int ether_init_header(void *, int, uint8_t *, uint8_t *, u_int16_t);
static int init_frame(uint8_t *, int, u_int16_t);
static void hpav_cast_frame(uint8_t *, int, struct ether_header *);
static void save_plc_mac(void);
static void save_plc_data(void);
int update_plc_data(void);

uint8_t hpav_intellon_oui[3] = { 0x00, 0xB0, 0x52 };
uint8_t hpav_intellon_macaddr[ETHER_ADDR_LEN] =
    { 0x00, 0xB0, 0x52, 0x00, 0x00, 0x01 };

uint8_t plc_mac[6];

struct station_data {
  struct sta_info sta_info;
  float raw_rate; //raw rate obtained with modulation data from A071 frame
};

struct network_data {
  int station_count;
  uint8_t network_id[7];
  uint8_t sta_mac[6];
  uint8_t sta_role;
  uint8_t cco_mac[6];
  float tx_mpdu_collision_perc;
  float tx_mpdu_failure_perc;
  float tx_pb_failure_perc;
  float rx_mpdu_failure_perc;
  float rx_pb_failure_perc;
  float rx_tbe_failure_perc;
  struct station_data sta_data[];
};


//Global variables

struct plc_data *p_data;
uint8_t p_size_t;
//int sockraw;
//struct sockaddr_ll socket_address;
pcap_t *pcap;

//int init_socket() {
//  sockraw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
//  if (sockraw == -1) {
//    olsr_printf(3, "problema con il socket");
//  }
//  /*prepare sockaddr_ll*/
//
//  /*RAW communication*/
//  socket_address.sll_family = PF_PACKET;
//  /*we don't use a protocoll above ethernet layer
//   ->just use anything here*/
//  socket_address.sll_protocol = htons(ETH_P_IP);
//  /*index of the network device
//   see full code later how to retrieve it*/
//  socket_address.sll_ifindex = 2;
//  /*ARP hardware identifier is ethernet*/
//  socket_address.sll_hatype = ARPHRD_ETHER;
//  /*target is another host*/
//  socket_address.sll_pkttype = PACKET_OTHERHOST;
//  /*address length*/
//  socket_address.sll_halen = ETH_ALEN;
//  /*MAC - begin*/
//  //48:5b:39:18:3a:5c
//  //  socket_address.sll_addr[0] = 0x48;
//  //  socket_address.sll_addr[1] = 0x5b;
//  //  socket_address.sll_addr[2] = 0x38;
//  //  socket_address.sll_addr[3] = 0x18;
//  //  socket_address.sll_addr[4] = 0x3a;
//  //  socket_address.sll_addr[5] = 0x5c;
//  //  /*MAC - end*/
//  //  socket_address.sll_addr[6] = 0x00;/*not used*/
//  //  socket_address.sll_addr[7] = 0x00;/*not used*/
//}


int init_plc_communication(char *device) {

  char pcap_errbuf[PCAP_ERRBUF_SIZE];
  int pcap_snaplen = ETHER_MAX_LEN;
  struct bpf_program pcap_filter;
  char filter_exp[] = "ether proto 0x88e1";

  if (!pcap_lookupdev(pcap_errbuf)) {
    olsr_printf(1, "pcap_lookupdev: can't find device %s", device);
  }

  /* Use open_live on Unixes */
  pcap = pcap_open_live(device, pcap_snaplen, 1, 100, pcap_errbuf);
  if (pcap == NULL) {
    olsr_printf(1, "Couldn't open device %s: %s\n", device, pcap_errbuf);
    return 0;
  }
  if (pcap_compile(pcap, &pcap_filter, filter_exp, 0, 0) == -1) {
    olsr_printf(1, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(pcap));
    return 0;
  }
  if (pcap_setfilter(pcap, &pcap_filter) == -1) {
    olsr_printf(1, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(pcap));
    return 0;
  }
  return 1;
}

static int send_frame_pcap(uint8_t *frame_buf, int frame_len) {
  /*send the packet*/
  int send_result;
  if (frame_len < ETH_ZLEN)
      frame_len = ETH_ZLEN;
  send_result = pcap_sendpacket(pcap, frame_buf, frame_len);
  if (send_result == -1) {
    olsr_printf(1, "pcap_inject: %s", pcap_geterr(pcap));
  }
  return send_result;
}

static int recv_frame_pcap() {
  uint8_t *frame_buf = malloc(ETH_FRAME_LEN); /*Buffer for ethernet frame*/
  struct pcap_pkthdr *pcap_header;
  u_char *pcap_data;
  u_int16_t *eth_type;
  int n;

  do {
    n = pcap_next_ex(pcap, &pcap_header, (const u_char **) &pcap_data);
    if (n < 0) {
      olsr_printf(3, "pcap_next_ex: %s", pcap_geterr(pcap));
    }
    if ((uint32_t) n > (uint32_t) (pcap_header->caplen)) {
      n = pcap_header->caplen;
    }
    //olsr_printf(3, "Received length: %d\n", length);
    //olsr_printf(3, "buffer byte 7-12: %02x:%02x:%02x:%02x:%02x:%02x\n", buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
    struct ether_header *eth_header = (struct ether_header *) pcap_data;
    eth_type = &(eth_header->ether_type);
    memcpy(&plc_mac, eth_header->ether_shost, sizeof(eth_header->ether_shost));
    uint8_t *frame_ptr = (uint8_t *) pcap_data;
    uint8_t *payload_ptr;
    int frame_len = n;
    int payload_len;
    payload_ptr = frame_ptr + sizeof(*eth_header);
    payload_len = frame_len - sizeof(*eth_header);
    //if((*eth_type == ntohs(ETHERTYPE_HOMEPLUG)) || (*eth_type == ntohs(ETHERTYPE_HOMEPLUG_AV))) {
    if ((*eth_type == ntohs(ETHERTYPE_HOMEPLUG_AV))) {
      hpav_cast_frame(payload_ptr, payload_len, eth_header);
      //print_blob(frame_ptr, frame_len);
    }
  } while (!(*eth_type == ntohs(ETHERTYPE_HOMEPLUG)) && !(*eth_type
      == ntohs(ETHERTYPE_HOMEPLUG_AV)));
}

int dump_hex(void *buf, int len, char *sep) {
  int avail = len;
  uint8_t *p = buf;

  while (avail > 0) {
    printf("%02hX%s", *p, (avail > 1) ? sep : "");
    p++;
    avail--;
  }
  return len;
}

#define HEX_BLOB_BYTES_PER_ROW  16

void print_blob(u_char *buf, int len) {
  u_int32_t i, d, m = len % HEX_BLOB_BYTES_PER_ROW;
  printf("Binary Data, %lu bytes", (unsigned long int) len);
  for (i = 0; i < len; i += HEX_BLOB_BYTES_PER_ROW) {
    d = (len - i) / HEX_BLOB_BYTES_PER_ROW;
    printf("\n%08lu: ", (unsigned long int) i);
    dump_hex((uint8_t *) buf + i, (d > 0) ? HEX_BLOB_BYTES_PER_ROW : m, " ");
  }
  printf("\n");
}

void print_plc_data() {
  int i;
  for (i = 0; i < p_size_t; i++) {
    printf("Station MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", p_data[i].mac[0], p_data[i].mac[1], p_data[i].mac[2], p_data[i].mac[3], p_data[i].mac[4], p_data[i].mac[5]);
    printf("TX rate: %d\n", p_data[i].tx_rate);
    printf("RX rate: %d\n\n", p_data[i].rx_rate);
  }
}

static int get_bits_per_carrier(short unsigned int modulation) {
  switch (modulation) {
  case NO:
    return 0;
    break;
  case BPSK:
    return 1;
    break;
  case QPSK:
    return 2;
    break;
  case QAM_8:
    return 3;
    break;
  case QAM_16:
    return 4;
    break;
  case QAM_64:
    return 6;
    break;
  case QAM_256:
    return 8;
    break;
  case QAM_1024:
    return 10;
    break;
  default:
    return 0;
    break;
  }
}

/**
 * ether_init_header - initialize the Ethernet frame header
 * @buf:      buffer to initialize
 * @len:      size of the buffer
 * @da:        destination MAC address (should not be NULL)
 * @sa:        destination MAC address (should not be NULL)
 * @ethertype: ethertype (between HomePlug 1.0/AV)
 * @return:    number of bytes set in buffer
 */
static int ether_init_header(void *buf, int len, uint8_t *da, uint8_t *sa, u_int16_t ethertype) {
  int avail = len;
  struct ether_header *header = (struct ether_header *) buf;

  /* set destination eth addr */
  memcpy(header->ether_dhost, da, ETHER_ADDR_LEN);

  /* set source eth addr */
  if (sa != NULL) {
    memcpy(header->ether_shost, sa, ETHER_ADDR_LEN);
  } else if (ethertype == ETHERTYPE_HOMEPLUG) {
    memset(header->ether_shost, 0xFF, ETHER_ADDR_LEN);
  } else if (ethertype == ETHERTYPE_HOMEPLUG_AV) {
    memset(header->ether_shost, 0x00, ETHER_ADDR_LEN);
  }

  /* Set the ethertype */
  header->ether_type = htons(ethertype);

  avail -= sizeof(*header);

  return (len - avail);
}

static int init_frame(uint8_t *frame_buf, int frame_len, u_int16_t mmtype) {
  struct hpav_frame *frame;
  uint8_t *frame_ptr = frame_buf;
  uint8_t *da;
  uint8_t *sa = NULL;
  bzero(frame_buf, frame_len);
  da = hpav_intellon_macaddr;

  /* Set the ethernet frame header */
  int n;
  n = ether_init_header(frame_ptr, frame_len, da, sa, ETHERTYPE_HOMEPLUG_AV);
  frame_ptr += n;
  frame = (struct hpav_frame *) frame_ptr;
  n = sizeof(frame->header);
  frame->header.mmtype = STORE16_LE(mmtype);
  if ((mmtype & HPAV_MM_CATEGORY_MASK) == HPAV_MM_VENDOR_SPEC) {
    frame->header.mmver = HPAV_VERSION_1_0;
    memcpy(frame->payload.vendor.oui, hpav_intellon_oui, 3);
    n += sizeof(frame->payload.vendor);
  } else {
    frame->header.mmver = HPAV_VERSION_1_1;
    n += sizeof(frame->payload.public);
  }
  frame_ptr += n;
  frame_len = frame_ptr - (uint8_t *) frame_buf;
  return frame_len;
}

//int send_frame(uint8_t *frame_buf, int frame_len) {
//  /*send the packet*/
//  int send_result;
//  if (frame_len < ETH_ZLEN)
//      frame_len = ETH_ZLEN;
//  send_result = sendto(sockraw, frame_buf, frame_len, 0, (struct sockaddr*) &socket_address, sizeof(socket_address));
//  return send_result;
//}

//int send_6048(uint8_t *frame_buf, int frame_len, int cursor) {
//  frame_len = faifa_send(faifa, frame_buf, frame_len);
//  if (frame_len == -1)
//    olsr_printf(3, "Init: error sending frame (%s)\n", faifa_error(faifa));
//  return frame_len;
//}
//
//int send_A038(uint8_t *frame_buf, int frame_len, int cursor) {
//  frame_len = faifa_send(faifa, frame_buf, frame_len);
//  if (frame_len == -1)
//    olsr_printf(3, "Init: error sending frame (%s)\n", faifa_error(faifa));
//  return frame_len;
//}
//
//int send_A070(uint8_t *frame_buf, int frame_len, int cursor, uint8_t macaddr[], uint8_t tsslot) {
//  uint8_t *frame_ptr = frame_buf;
//  int i, n;
//  frame_ptr += cursor;
//  struct get_tone_map_charac_request *mm =
//      (struct get_tone_map_charac_request *) frame_ptr;
//  for (i = 0; i < 6; i++)
//    mm->macaddr[i] = macaddr[i];
//  n = sizeof(*mm);
//  frame_len = frame_ptr - (uint8_t *) frame_buf;
//  if (frame_len < ETH_ZLEN)
//    frame_len = ETH_ZLEN;
//  frame_len = faifa_send(faifa, frame_buf, frame_len);
//  if (frame_len == -1)
//    olsr_printf(3, "Init: error sending frame (%s)\n", faifa_error(faifa));
//  return frame_len;
//}

static void hpav_cast_frame(uint8_t *frame_ptr, int frame_len, struct ether_header *hdr) {
  struct hpav_frame *frame = (struct hpav_frame *) frame_ptr;
  if ((frame->header.mmtype & HPAV_MM_CATEGORY_MASK) == HPAV_MM_VENDOR_SPEC) {
    frame_ptr = frame->payload.vendor.data;
    frame_len -= sizeof(frame->payload.vendor);
  } else {
    frame_ptr = frame->payload.public.data;
    frame_len -= sizeof(frame->payload.public);
  }
  switch (frame->header.mmtype) {
  case 0xA039: {
    struct network_info_confirm *mm = (struct network_info_confirm *) frame_ptr;
    int n_stas = mm->num_stas;
    p_size_t = n_stas;
    p_data = (struct plc_peer *) malloc(n_stas * sizeof(*p_data));
    //**nd = (struct network_data *) malloc(sizeof(struct network_data)+ sizeof(struct station_data[n_stas]));
    //(**nd)->station_count = n_stas;
    //memcpy((**nd)->network_id, mm->nid, 7);
    //memcpy((**nd)->sta_mac, hdr->ether_shost, ETHER_ADDR_LEN);
    //(**nd)->sta_role = mm->sta_role;
    //memcpy((**nd)->cco_mac, mm->cco_macaddr, ETHER_ADDR_LEN);
    int i;
    //struct station_data sta_d[n_stas];

    for (i = 0; i < n_stas; i++) {
      memcpy(&p_data[i].mac, mm->stas[i].sta_macaddr, sizeof(mm->stas[i].sta_macaddr));
      p_data[i].tx_rate = mm->stas[i].avg_phy_tx_rate;
      p_data[i].rx_rate = mm->stas[i].avg_phy_rx_rate;
      //sta_d[i].sta_info = mm->stas[i];
      //memcpy(&sta_d[i].sta_info, &mm->stas[i], sizeof(struct sta_info));
      //(**nd)->sta_data[i] = sta_d[i];
    }
    break;
  }
  case 0xA031:

    break;
  case 0xA071: {
    /*faifa_printf(out_stream, "ricevo A071\n");
     struct get_tone_map_charac_confirm *mm =
     (struct get_tone_map_charac_confirm *) frame_ptr;
     if (mm->mstatus != 0x00) {
     faifa_printf(out_stream, "A070-A071 error\n");
     break;
     }
     unsigned int total_bits = 0;
     int i;
     for (i = 0; i < MAX_CARRIERS; i++) {
     //faifa_printf(out_stream, "Total bits: %d\n", total_bits);
     total_bits += get_bits_per_carrier(mm->carriers[i].mod_carrier_lo);
     total_bits += get_bits_per_carrier(mm->carriers[i].mod_carrier_hi);
     }
     //faifa_printf(out_stream, "Total bits: %d\n", total_bits);
     float bits_per_second = (float) total_bits / 0.0000465;
     faifa_printf(out_stream, "Modulation rate: %4.2f bit/s\n",
     bits_per_second);
     break;*/
  }
  case 0x6049: {
    struct cm_get_network_stats_confirm *mm =
        (struct cm_get_network_stats_confirm *) frame_ptr;
    memcpy(&plc_mac, mm->sta.infos[0].macaddr, sizeof(mm->sta.infos[0].macaddr));
  }
  }
}

//void receive_frame(void) {
//  uint8_t *buffer = malloc(ETH_FRAME_LEN); /*Buffer for ethernet frame*/
//  int length = 0; /*length of the received frame*/
//  u_int32_t l = 1518;
//  u_int16_t *eth_type;
//  do {
//    length = recvfrom(sockraw, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
//    if (length == -1) {
//      olsr_printf(3, "\nNon ricevo\n");
//    }
//    //olsr_printf(3, "Received length: %d\n", length);
//    //olsr_printf(3, "buffer byte 7-12: %02x:%02x:%02x:%02x:%02x:%02x\n", buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
//    struct ether_header *eth_header = (struct ether_header *) buffer;
//    eth_type = &(eth_header->ether_type);
//    memcpy(&plc_mac, eth_header->ether_shost, sizeof(eth_header->ether_shost));
//    uint8_t *frame_ptr = (uint8_t *) buffer;
//    uint8_t *payload_ptr;
//    int frame_len = length;
//    int payload_len;
//    payload_ptr = frame_ptr + sizeof(*eth_header);
//    payload_len = frame_len - sizeof(*eth_header);
//    //if((*eth_type == ntohs(ETHERTYPE_HOMEPLUG)) || (*eth_type == ntohs(ETHERTYPE_HOMEPLUG_AV))) {
//    if ((*eth_type == ntohs(ETHERTYPE_HOMEPLUG_AV))) {
//      hpav_cast_frame(payload_ptr, payload_len, eth_header);
//      //print_blob(frame_ptr, frame_len);
//    }
//  } while (!(*eth_type == ntohs(ETHERTYPE_HOMEPLUG)) && !(*eth_type == ntohs(ETHERTYPE_HOMEPLUG_AV)));
//}

static void save_plc_mac(void) {
  olsr_printf(3, "sockraw: prima di save: %02x:%02x:%02x:%02x:%02x:%02x\n", plc_mac[0], plc_mac[1], plc_mac[2], plc_mac[3], plc_mac[4], plc_mac[5]);
  set_own_plc_mac(plc_mac);
}

static void save_plc_data(void) {
  int i, updated;
  for(i=0; i < p_size_t; i++) {
    updated = update_plc_peer_data(&p_data[i]);
    if(updated) {
      olsr_printf(3, "\nsockraw: plc mac aggiornato: %02x:%02x:%02x:%02x:%02x:%02x\n", p_data[i].mac[0], p_data[i].mac[1], p_data[i].mac[2], p_data[i].mac[3], p_data[i].mac[4], p_data[i].mac[5]);
      olsr_printf(3, "sockraw: plc peer trovato e aggiornato\n");
    }
  }
}

int update_plc_data() {
  uint8_t frame_buf[1518];
  int payload_offset;
  int frame_len = sizeof(frame_buf);
  frame_len = init_frame(frame_buf, frame_len, 0xA038);
  int send_result = send_frame_pcap(frame_buf, frame_len);
  recv_frame_pcap();
  save_plc_mac();
  save_plc_data();
//  olsr_printf(3, "\nsockraw: plc mac: %02x:%02x:%02x:%02x:%02x:%02x\n", plc_mac[0], plc_mac[1], plc_mac[2], plc_mac[3], plc_mac[4], plc_mac[5]);
//  olsr_printf(3, "sockraw: Numero di stazioni: %d\n", p_size_t);
//  olsr_printf(3, "\nsockraw: mac STA 1: %02x:%02x:%02x:%02x:%02x:%02x\n", p_data[0].mac[0], p_data[0].mac[1], p_data[0].mac[2], p_data[0].mac[3], p_data[0].mac[4], p_data[0].mac[5]);
  return 1;
}

//
//
//olsr_printf(3, "frame_len: %d\n", frame_len);
//olsr_printf(3, "frame_buf primi 6 byte: %02x:%02x:%02x:%02x:%02x:%02x\n", frame_buf[0], frame_buf[1], frame_buf[2], frame_buf[3], frame_buf[4], frame_buf[5]);
//olsr_printf(3, "frame_buf byte 7-12: %02x:%02x:%02x:%02x:%02x:%02x\n", frame_buf[6], frame_buf[7], frame_buf[8], frame_buf[9], frame_buf[10], frame_buf[11]);
//olsr_printf(3, "frame_buf byte 13-18: %02x:%02x:%02x:%02x:%02x:%02x\n", frame_buf[12], frame_buf[13], frame_buf[14], frame_buf[15], frame_buf[16], frame_buf[17]);





