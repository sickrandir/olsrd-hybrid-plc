#include "olsr_types.h"
#include "hashing.h"

struct plc_data {
  uint8_t mac[6];
  uint8_t tx_rate;
  uint8_t rx_rate;
};

struct plc_peer_entry {
  union olsr_ip_addr plc_peer_main_addr;
  struct plc_data plc_data;
  struct plc_peer_entry *next;
  struct plc_peer_entry *prev;
};

void init_plc_peer_neighbors(void);

struct plc_peer_entry *insert_plc_peer_neighbor(const union olsr_ip_addr *, unsigned char *);

struct plc_peer_entry *lookup_plc_peer_by_ip(const union olsr_ip_addr *);

struct plc_peer_entry *lookup_plc_peer_by_mac(unsigned char *);

int delete_plc_peer_neighbor(const union olsr_ip_addr *);

void print_plc_peer_neighbors(void);

const char * get_own_plc_mac(void);

int set_own_plc_mac(unsigned char *);
