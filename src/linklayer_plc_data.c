
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

struct plc_peer_entry plc_peer_neighbors[HASHSIZE];

void
init_plc_peer_neighbors(void)
{
  int i;

  for (i = 0; i < HASHSIZE; i++) {
    plc_peer_neighbors[i].next = &plc_peer_neighbors[i];
    plc_peer_neighbors[i].prev = &plc_peer_neighbors[i];
  }
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
      return new_peer;
  }

  //olsr_printf(3, "inserting peer\n");

  new_peer = olsr_malloc(sizeof(struct plc_peer_entry), "New peer entry");

  /* Set address, willingness and status */
  new_peer->plc_peer_main_addr = *main_addr;
  memcpy(new_peer->plc_data.mac, mac, 6);

  /* Queue */
  QUEUE_ELEM(plc_peer_neighbors[hash], new_peer);

  return new_peer;
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
  int i;

  //brute force lookup per mac address
  for (i = 0; i < HASHSIZE; i++) {
	  entry = &plc_peer_neighbors[i];
	  if (memcmp(mac, entry->plc_data.mac, 6) == 0) {
	    olsr_printf(3, "linklayer_plc_data: lookup by mac: trovato!");
	    return entry;
	  }
	  for (entry = plc_peer_neighbors[i].next; entry != &plc_peer_neighbors[i]; entry = entry->next) {
	      //olsr_printf(3, "Checking %s\n", olsr_ip_to_string(&buf, &entry->plc_peer_main_addr));
		  if (memcmp(mac, entry->plc_data.mac, 6) == 0)
		  	  return entry;
	  }
  }
  return NULL;
}

const char * get_own_plc_mac(void) {
	return own_plc_mac;
}

int set_own_plc_mac(unsigned char * mac) {
	memcpy(own_plc_mac, mac, 6);
	olsr_printf(3, "olsrd: linklayer_plc_data: My PLC MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", own_plc_mac[0], own_plc_mac[1], own_plc_mac[2], own_plc_mac[3], own_plc_mac[4], own_plc_mac[5]);
}

int update_plc_peer_data(struct plc_data * p_data) {
  struct plc_peer_entry *entry;
  entry = lookup_plc_peer_by_mac(p_data->mac);
  if(entry != NULL)
    return 0;
  if(entry != NULL) {
    entry->plc_data.rx_rate = p_data->rx_rate;
    entry->plc_data.tx_rate = p_data->tx_rate;
    return 1;
  }
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
//#ifdef NODEBUG
//  /* The whole function doesn't do anything else. */
//#ifndef NODEBUG
//  const int iplen = olsr_cnf->ip_version == AF_INET ? 15 : 39;
//#endif
//  int idx;
//  OLSR_PRINTF(1,
//              "\n--- %02d:%02d:%02d.%02d ------------------------------------------------ NEIGHBORS\n\n"
//              "%*s  LQ     NLQ    SYM   MPR   MPRS  will\n", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, (int)now.tv_usec / 10000,
//              iplen, "IP address");
//
//  for (idx = 0; idx < HASHSIZE; idx++) {
//    struct plc_peer_entry *neigh;
//    for (neigh = plc_peer_neighbors[idx].next; neigh != &plc_peer_neighbors[idx]; neigh = neigh->next) {
//      struct link_entry *lnk = get_best_link_to_neighbor(&neigh->plc_peer_main_addr);
//      if (lnk) {
//        struct ipaddr_str buf;
//        OLSR_PRINTF(1, "%-*s  %5.3f  %5.3f  %s  %s  %s  %d\n", iplen, olsr_ip_to_string(&buf, &neigh->plc_peer_main_addr),
//                    lnk->loss_link_quality, lnk->neigh_link_quality, neigh->status == SYM ? "YES " : "NO  ",
//                    neigh->is_mpr ? "YES " : "NO  ", olsr_lookup_mprs_set(&neigh->plc_peer_main_addr) == NULL ? "NO  " : "YES ",
//                    neigh->willingness);
//      }
//    }
//  }
//#endif
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
