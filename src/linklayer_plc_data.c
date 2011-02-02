
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "scheduler.h"
#include "link_set.h"

#include "linklayer_plc_data.h"

uint8_t own_plc_mac[6];
static ready;
struct plc_peer_entry plc_peer_neighbors[HASHSIZE];


void
init_plc_peer_neighbors(void)
{
  int i;
  for (i = 0; i < HASHSIZE; i++) {
    plc_peer_neighbors[i].next = &plc_peer_neighbors[i];
    plc_peer_neighbors[i].prev = &plc_peer_neighbors[i];
  }
  memset(own_plc_mac, 0, 6);
  ready = false;
}

/**
 *Delete a plc_peer table entry.
 *
 *
 *@param plc_peer the peer entry to delete
 *
 *@return nada
 */

int
delete_plc_peer_neighbor(const union olsr_ip_addr *peer_addr)
{
  uint32_t hash;
  struct plc_peer_entry *entry;

  //olsr_printf(3, "inserting neighbor\n");

  hash = olsr_ip_hashing(peer_addr);

  entry = plc_peer_neighbors[hash].next;

  /*
   * Find neighbor entry
   */
  while (entry != &plc_peer_neighbors[hash]) {
    if (ipequal(&entry->plc_peer_main_addr, peer_addr))
      break;

    entry = entry->next;
  }

  if (entry == &plc_peer_neighbors[hash])
    return 0;

   /* Dequeue */
  DEQUEUE_ELEM(entry);

  free(entry);

  return 1;
}

/**
 *Insert a plc peer entry in the table
 *
 *@param main_addr the main address of the new node
 *
 *@return 0 if plc peer already exists 1 if inserted
 */
struct plc_peer_entry *
insert_plc_peer_neighbor(const union olsr_ip_addr *main_addr, unsigned char *mac)
{
  uint32_t hash;
  struct plc_peer_entry *new_peer;

  hash = olsr_ip_hashing(main_addr);

  /* Check if entry exists */

  for (new_peer = plc_peer_neighbors[hash].next; new_peer != &plc_peer_neighbors[hash]; new_peer = new_peer->next) {
    if (ipequal(&new_peer->plc_peer_main_addr, main_addr))
      return 0;
  }

  //olsr_printf(3, "inserting peer\n");

  new_peer = olsr_malloc(sizeof(struct plc_peer_entry), "New peer entry");

  /* Set address, willingness and status */
  new_peer->plc_peer_main_addr = *main_addr;
  memcpy(new_peer->plc_data.mac, mac, 6);

  /* Queue */
  QUEUE_ELEM(plc_peer_neighbors[hash], new_peer);

  return 1;
}


/**
 *Lookup a peer entry in the plc_peer_neighbors based on an address.
 *
 *@param dst the IP address of the peer to look up
 *
 *@return a pointer to the neighbor struct registered on the given
 *address. NULL if not found.
 */
struct plc_peer_entry *
lookup_plc_peer_by_ip(const union olsr_ip_addr *dst)
{
  struct plc_peer_entry *entry;
  uint32_t hash = olsr_ip_hashing(dst);

  //olsr_printf(3, "\nLookup %s\n", olsr_ip_to_string(&buf, dst));
  for (entry = plc_peer_neighbors[hash].next; entry != &plc_peer_neighbors[hash]; entry = entry->next) {
    //olsr_printf(3, "Checking %s\n", olsr_ip_to_string(&buf, &entry->plc_peer_main_addr));
    if (ipequal(&entry->plc_peer_main_addr, dst))
      return entry;

  }
  //olsr_printf(3, "NOPE\n\n");

  return NULL;

}

/**
 *Lookup a peer entry in the plc_peer_neighbors based on an address.
 *
 *@param dst the IP address of the peer to look up
 *
 *@return a pointer to the neighbor struct registered on the given
 *address. NULL if not found.
 */
struct plc_peer_entry *
lookup_plc_peer_by_mac(unsigned char *mac)
{
  struct plc_peer_entry *entry;
  int i,j;
  olsr_printf(3, "olsrd: linklayer_plc_data: in lookup: lookup di: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  //brute force lookup per mac address
  for (i = 0; i < HASHSIZE; i++) {
	  entry = &plc_peer_neighbors[i];

	  for (entry = plc_peer_neighbors[i].next; entry != &plc_peer_neighbors[i]; entry = entry->next) {
      if (memcmp(mac, entry->plc_data.mac, 6) == 0)
        return entry;
	  }
  }
  return NULL;
}

struct plc_peers_iterator *init_plc_peers_iterator() {
  struct plc_peers_iterator iter;
  iter.num_iterated = 0;
  return &iter;
}

struct plc_peer_entry *iter_next(struct plc_peers_iterator *iter) {
  struct plc_peer_entry *entry;
  int i, j;
  j = 0;

  for (i = 0; i < HASHSIZE; i++) {
    entry = &plc_peer_neighbors[i];
    for (entry = plc_peer_neighbors[i].next; entry != &plc_peer_neighbors[i]; entry = entry->next) {
      j++;
      if (j > iter->num_iterated) {
        iter->num_iterated++;
        return entry;
      }
    }
  }
  return NULL;
}


unsigned char * get_own_plc_mac(void) {
	return own_plc_mac;
}

int set_own_plc_mac(unsigned char * mac) {
	memcpy(own_plc_mac, mac, 6);
	ready = true;
	olsr_printf(3, "olsrd: linklayer_plc_data: My PLC MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", own_plc_mac[0], own_plc_mac[1], own_plc_mac[2], own_plc_mac[3], own_plc_mac[4], own_plc_mac[5]);
}

int update_plc_peer_data(struct plc_data * p_data) {
  struct plc_peer_entry *entry;
  olsr_printf(3, "olsrd: linklayer_plc_data: lookup di: %02X:%02X:%02X:%02X:%02X:%02X\n", p_data->mac[0], p_data->mac[1], p_data->mac[2], p_data->mac[3], p_data->mac[4], p_data->mac[5]);
  entry = lookup_plc_peer_by_mac(p_data->mac);
  if(entry == NULL)
    return 0;
  if(entry != NULL) {
    olsr_printf(3, "olsrd: linklayer_plc_data: Updating PLC MAC: %02X:%02X:%02X:%02X:%02X:%02X with TX_rate: %d - RX_rate: %d\n", entry->plc_data.mac[0], entry->plc_data.mac[1], entry->plc_data.mac[2], entry->plc_data.mac[3], entry->plc_data.mac[4], entry->plc_data.mac[5], p_data->rx_rate, p_data->rx_rate);
    entry->plc_data.rx_rate = p_data->rx_rate;
    entry->plc_data.tx_rate = p_data->tx_rate;
    entry->plc_data.raw_modulation_rate = p_data->raw_modulation_rate;
    return 1;
  }
}

bool is_ready() {
  return ready;
}


/**
 *Prints the registered neighbors and two hop neighbors
 *to STDOUT.
 *
 *@return nada
 */
void
print_plc_peer_neighbors(void)
{
  int idx;
  olsr_printf(1, "\n---IP address ----- PLC MAC ------TX-RATE---RX-RATE-----RAW MODULATION RATE (Mbits/s)\n");
  for (idx = 0; idx < HASHSIZE; idx++) {
    struct plc_peer_entry *entry;
    for (entry = plc_peer_neighbors[idx].next; entry != &plc_peer_neighbors[idx]; entry = entry->next) {
      struct ipaddr_str buf;
      olsr_printf(1, "\n%s---%02X:%02X:%02X:%02X:%02X:%02X ----%d  /  %d-----%.2f\n", olsr_ip_to_string(&buf, &entry->plc_peer_main_addr), entry->plc_data.mac[0], entry->plc_data.mac[1], entry->plc_data.mac[2], entry->plc_data.mac[3], entry->plc_data.mac[4], entry->plc_data.mac[5], entry->plc_data.tx_rate, entry->plc_data.rx_rate, entry->plc_data.raw_modulation_rate);
    }
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
