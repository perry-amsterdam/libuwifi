/* horst - Highly Optimized Radio Scanning Tool
 *
 * Copyright (C) 2005-2016 Bruno Randolf (br1@einfach.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _NODE_H_
#define _NODE_H_

#include "platform.h"
#include "wlan_parser.h"
#include "ccan/list/list.h"
#include "average.h"
#include "phy_info.h"
#include "conf.h"

struct node_info {
	/* housekeeping */
	struct list_node	list;								// X
	struct list_node	essid_nodes;
	struct list_head	on_channels;	/* channels this node was seen on */
	unsigned int		num_on_channels;
	uint32_t		last_seen;	/* timestamp */					// X uint32

	/* general packet info */
	unsigned int		pkt_types;	/* bitmask of packet types we've seen */
	unsigned int		pkt_count;	/* nr of packets seen */

	/* wlan phy (from radiotap) */
	int			phy_sig_max;
	struct ewma		phy_sig_avg;
	unsigned long		phy_sig_sum;							// X
	int			phy_sig_count;							// X

	/* wlan mac */
	unsigned char		wlan_src[MAC_LEN];	/* Sender MAC address (ID) */		// X
	unsigned char		wlan_bssid[MAC_LEN];
	unsigned int		wlan_channel;	/* channel from beacon, probe frames */		// X
	unsigned int		wlan_mode;	/* AP, STA or IBSS */				// X
	uint64_t		wlan_tsf;
	unsigned int		wlan_bintval;
	unsigned int		wlan_retries_all;
	unsigned int		wlan_retries_last;
	unsigned int		wlan_seqno;
	struct essid_info*	essid;								// TODO!!!
	struct node_info*	wlan_ap_node;							// X
	enum chan_width		wlan_chan_width;
	unsigned char		wlan_tx_streams;
	unsigned char		wlan_rx_streams;


	unsigned int		wlan_wep:1,	/* WEP active? */
				wlan_wpa:1,
				wlan_rsn:1,
				wlan_ht40plus:1;


	/* batman */
	unsigned char		bat_gw:1;

	/* IP */
	unsigned int		ip_src;		/* IP address (if known) */
	unsigned int		olsr_count;	/* number of OLSR packets */
	unsigned int		olsr_neigh;	/* number if OLSR neighbours */
	unsigned int		olsr_tc;	/* unused */

	struct packet_info	last_pkt;
};

struct node_info* node_update(struct packet_info* p, struct list_head* nodes);
void node_timeout(struct list_head* nodes, unsigned int timeout_sec);

#endif
