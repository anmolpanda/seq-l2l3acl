/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2015 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// A large portion of the code in this file comes from
// main.c in the l3fwd example of DPDK 2.2.0.

#include "dpdk_lib.h"
#include "actions.h"
#include <rte_ethdev.h>
#include <time.h>
#include <x86intrin.h>
#include <cpuid.h>


//Anmol:header files already included via dpdk_lib.h->backend.h
//#include"key.h"
struct rte_mempool *header_pool, *clone_pool;
struct lcore_conf lcore_conf[RTE_MAX_LCORE];

extern void p4_handle_packet(packet* p, unsigned portid);

//=   shared   ================================================================

uint32_t enabled_port_mask = 0;

//=   used only here   ========================================================


extern unsigned int rx_queue_per_lcore;

/* A tsc-based timer responsible for triggering statistics printout */
#define TIMER_MILLISECOND 2000000ULL /* around 1ms at 2 Ghz */
#define MAX_TIMER_PERIOD 86400 /* 1 day max */
int64_t timer_period = 10 * TIMER_MILLISECOND * 1000; /* default period is 10 seconds */

#define MAX_PORTS 16

#define MCAST_CLONE_PORTS       2
#define MCAST_CLONE_SEGS        2

#define RTE_TEST_RX_DESC_DEFAULT 256
#define RTE_TEST_TX_DESC_DEFAULT 256
uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;


#define PKT_MBUF_DATA_SIZE      RTE_MBUF_DEFAULT_BUF_SIZE
#define NB_PKT_MBUF     8192

#define HDR_MBUF_DATA_SIZE      (2 * RTE_PKTMBUF_HEADROOM)
#define NB_HDR_MBUF     (NB_PKT_MBUF * MAX_PORTS)

#define NB_CLONE_MBUF   (NB_PKT_MBUF * MCAST_CLONE_PORTS * MCAST_CLONE_SEGS * 2)

#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */



// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE 1024
struct rte_mbuf* deparse_mbuf;

static const struct rte_eth_conf port_conf = {
    .rxmode = {
        .split_hdr_size = 0,
        .header_split   = 0, /**< Header Split disabled */
        .hw_ip_checksum = 0, /**< IP checksum offload disabled */
        .hw_vlan_filter = 0, /**< VLAN filtering disabled */
        .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
        .hw_strip_crc   = 0, /**< CRC stripped by hardware */
    },
    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
    },
};

struct rte_mempool * pktmbuf_pool[NB_SOCKETS];

//=============================================================================
//cross product init
void compute_cross_product_table(lookup_table_t** tables);

/* Send burst of packets on an output interface */
static inline int
send_burst(struct lcore_conf *qconf, uint16_t n, uint8_t port)
{
    struct rte_mbuf **m_table;
    int ret;
    uint16_t queueid;

    queueid = qconf->tx_queue_id[port];
    m_table = (struct rte_mbuf **)qconf->tx_mbufs[port].m_table;
    ret = rte_eth_tx_burst(port, queueid, m_table, n);
    if (unlikely(ret < n)) {
        do {
            rte_pktmbuf_free(m_table[ret]);
        } while (++ret < n);
    }

    return ret;
}

/* Send burst of outgoing packet, if timeout expires. */
static inline void
send_timeout_burst(struct lcore_conf *qconf)
{
        uint64_t cur_tsc;
        uint8_t portid;
        const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

        cur_tsc = rte_rdtsc();
        if (likely (cur_tsc < qconf->tx_tsc + drain_tsc))
            return;

        for (portid = 0; portid < MAX_PORTS; portid++) {
            if (qconf->tx_mbufs[portid].len != 0) {
                send_burst(qconf, qconf->tx_mbufs[portid].len, portid);
                qconf->tx_mbufs[portid].len = 0; 
            }
        }
        qconf->tx_tsc = cur_tsc;
}



static int
get_socketid(unsigned lcore_id)
{
    if (numa_on)
        return rte_lcore_to_socket_id(lcore_id);
    else
        return 0;
}


static inline void
dbg_print_headers(packet_descriptor_t* pd)
{
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
        //debug("    :: header %d (type=%d, len=%d) = ", i, pd->headers[i].type, pd->headers[i].length);
        for (int j = 0; j < pd->headers[i].length; ++j) {
            //debug("%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);
        }
        //debug("\n");
    }
}

static inline unsigned
deparse_headers(packet_descriptor_t* pd, int socketid)
{
    uint8_t* deparse_buffer = (uint8_t*)rte_pktmbuf_append(deparse_mbuf, DEPARSE_BUFFER_SIZE);
    int len = 0;
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
        uint8_t* hdr_ptr = (uint8_t*)(pd->headers[i].pointer);
        unsigned hdr_len = pd->headers[i].length;
        for (int j = 0; j < hdr_len; ++j) {
            *deparse_buffer = *hdr_ptr;
            ++deparse_buffer;
            ++hdr_ptr;
        }
        len += hdr_len;
    }
    return len;
}

/* Get number of bits set. */
static inline uint32_t
bitcnt(uint32_t v)
{
        uint32_t n;

        for (n = 0; v != 0; v &= v - 1, n++)
                ;

        return (n);
}

static void
dpdk_send_packet(struct rte_mbuf *m, uint8_t port, uint32_t lcore_id)
{
    struct lcore_conf *qconf = &lcore_conf[lcore_id];
    uint16_t len = qconf->tx_mbufs[port].len;
    qconf->tx_mbufs[port].m_table[len] = m;
    len++;

    if (unlikely(len == MAX_PKT_BURST)) {
        //debug("    :: BURST SENDING DPDK PACKETS - port:%d\n", port);
        send_burst(qconf, MAX_PKT_BURST, port);
        len = 0;
    }

    qconf->tx_mbufs[port].len = len;
}

/* creating replicas of a packet for  */
static inline struct rte_mbuf *
mcast_out_pkt(struct rte_mbuf *pkt, int use_clone)
{
    struct rte_mbuf *hdr;

    //debug("mcast_out_pkt new mbuf is needed...\n");
        /* Create new mbuf for the header. */
        if ((hdr = rte_pktmbuf_alloc(header_pool)) == NULL)
                return (NULL);

    //debug("hdr is allocated\n");

    /* If requested, then make a new clone packet. */
    if (use_clone != 0 &&
        (pkt = rte_pktmbuf_clone(pkt, clone_pool)) == NULL) {
            rte_pktmbuf_free(hdr);
            return (NULL);
    }

    //debug("setup ne header\n");

    /* prepend new header */
    hdr->next = pkt;


    /* update header's fields */
    hdr->pkt_len = (uint16_t)(hdr->data_len + pkt->pkt_len);
    hdr->nb_segs = (uint8_t)(pkt->nb_segs + 1);

    /* copy metadata from source packet*/
    hdr->port = pkt->port;
    hdr->vlan_tci = pkt->vlan_tci;
    hdr->vlan_tci_outer = pkt->vlan_tci_outer;
    hdr->tx_offload = pkt->tx_offload;
    hdr->hash = pkt->hash;

    hdr->ol_flags = pkt->ol_flags;

    __rte_mbuf_sanity_check(hdr, 1);
    return (hdr);
}

static void
dpdk_mcast_packet(struct rte_mbuf *m, uint32_t port_mask, uint32_t lcore_id)
{
    struct rte_mbuf *mc;
    uint32_t port_num, use_clone;
    uint8_t port;

    port_num = bitcnt(port_mask);

    /* Should we use rte_pktmbuf_clone() or not. */
    use_clone = (port_num <= MCAST_CLONE_PORTS &&
        m->nb_segs <= MCAST_CLONE_SEGS);

    /* Mark all packet's segments as referenced port_num times */
    if (use_clone == 0)
            rte_pktmbuf_refcnt_update(m, (uint16_t)port_num);

    //debug("USE_CLONE = %d\n", use_clone);

    for (port = 0; use_clone != port_mask; port_mask >>= 1, port++) {
        /* Prepare output packet and send it out. */
        if ((port_mask & 1) != 0) {
            //debug("MCAST - PORT -%d\n", port);
            if ((mc = mcast_out_pkt(m, use_clone)) != NULL) {
                //debug("MCAST mc is ready\n");
                dpdk_send_packet(mc, port, lcore_id);
            } else if (use_clone == 0) {
                rte_pktmbuf_free(m);
            }
        }
    }

    /*
     * If we making clone packets, then, for the last destination port,
     * we can overwrite input packet's metadata.
     */
    if (use_clone != 0)
        dpdk_send_packet(m, port, lcore_id);
    else
        rte_pktmbuf_free(m);
}

static void
dpdk_bcast_packet(struct rte_mbuf *m, uint8_t ingress_port, uint32_t lcore_id)
{
    struct rte_mbuf *mc;
    uint32_t port_num;
    uint8_t port; //,portid;
    port_num = 2; // TODO: update

    //debug("Broadcast - ingress port:%d/%d\n", ingress_port, port_num);

    /* Mark all packet's segments as referenced port_num times */
//        rte_pktmbuf_refcnt_update(m, (uint16_t)port_num);

    for (port = 0; port<port_num; port++) {
        /* Prepare output packet and send it out. */
        if (port != ingress_port) {
                if ((mc = mcast_out_pkt(m, 1)) != NULL)
                        dpdk_send_packet(mc, port, lcore_id);
        }
    }

    /*
     * If we making clone packets, then, for the last destination port,
     * we can overwrite input packet's metadata.
     */
     rte_pktmbuf_free(m);
}

#define EXTRACT_EGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/6) & /*mask*/0x7fc) >> /*bitoffset*/2
#define EXTRACT_INGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/0) & /*mask*/0x1ff) >> /*bitoffset*/0

/* Enqueue batch_size number of packets, and send burst if queue is filled */
static inline int
send_packets(packet_descriptor_t* pd, int batch_size, int inport)
{
    uint32_t lcore_id = rte_lcore_id();
    for(int i = 0; i<batch_size; i++) {
        int port = EXTRACT_EGRESSPORT(&pd[i]);
        //int inport = EXTRACT_INGRESSPORT(&pd[i]);
        //if(port == 100)
        //      debug("Port is 100, broadcast, why ?\n");

        port =  (inport + 0) % 4;       //Custom port for forward the packet
        struct rte_mbuf *m = (struct rte_mbuf *)((&pd[i])->wrapper);
        struct lcore_conf *qconf = &lcore_conf[lcore_id];
        uint16_t len = qconf->tx_mbufs[port].len;
        qconf->tx_mbufs[port].m_table[len] = m;
        len++;

        if (unlikely(len == MAX_PKT_BURST)) {
                int ret = send_burst(qconf, MAX_PKT_BURST, port);
                if(ret <= 0)
                        debug("no packet was sent\n");
                len = 0;
        }
        qconf->tx_mbufs[port].len = len;
    }
    return 0;
}

static void
set_metadata_inport(packet_descriptor_t* packet_desc, uint32_t inport)
{
    //modify_field_to_const(packet_desc, field_desc(field_instance_standard_metadata_ingress_port), (uint8_t*)&inport, 2);
    int res32; // needed for the macro
    MODIFY_INT32_INT32_BITS(packet_desc, field_instance_standard_metadata_ingress_port, inport); // TODO fix? LAKI
}

void
packets_received(packet_descriptor_t* pd, int batch_size, packet **p, unsigned portid, struct lcore_conf *conf)
{
    for(int i = 0; i < batch_size; i++) {
        pd[i].data = rte_pktmbuf_mtod(p[i], uint8_t *);
        pd[i].wrapper = p[i];
        //set_metadata_inport(&pd[i], portid);
    }
    handle_packets(pd, batch_size, conf->state.tables);
    send_packets(pd, batch_size, portid);
}

void
dpdk_main_loop(void)
{
    uint64_t total=0, iterations=0;
    int batch_size = 0;
    packet *pkts_burst[4*MAX_PKT_BURST];//Ankit
    packet *p;
    uint64_t prev_tsc, diff_tsc, cur_tsc;
    unsigned i, j, portid, nb_rx;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;
    uint8_t queueid;

    prev_tsc = 0;

    unsigned lcore_id = rte_lcore_id();
    struct lcore_conf *qconf = &lcore_conf[lcore_id];

    if (qconf->n_rx_queue == 0) {
        RTE_LOG(INFO, P4_FWD, "lcore %u has nothing to do\n", lcore_id);
        return;
    }

    RTE_LOG(INFO, P4_FWD, "entering main loop on lcore %u\n", lcore_id);

    for (i = 0; i < qconf->n_rx_queue; i++) {

        portid = qconf->rx_queue_list[i].port_id;
        queueid = qconf->rx_queue_list[i].queue_id;
        RTE_LOG(INFO, P4_FWD, " -- lcoreid=%u portid=%u rxqueueid=%hhu\n", lcore_id, portid, queueid);
    }

    /*struct lcore_conf *conf = &lcore_conf[lcore_id];*/

    packet_descriptor_t pd[MAX_PKT_BURST];
    for(int i =0; i < MAX_PKT_BURST; i++) {
	    init_dataplane(&pd[i], qconf->state.tables);
    }
//Anmol: Call init here
    init_vlan_ingress(qconf->state.tables);
    init_mac(qconf->state.tables);
    init_routable(qconf->state.tables);
    init_switching(qconf->state.tables);
    init_acl(qconf->state.tables);
    //compute_cross_product_table_123(qconf->state.tables);
    while (1) {

        cur_tsc = rte_rdtsc();

        /*
         * TX burst queue drain
         */
        diff_tsc = cur_tsc - prev_tsc;
        if (unlikely(diff_tsc > drain_tsc)) {

            for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++) {
                if (qconf->tx_mbufs[portid].len == 0)
                    continue;
                send_burst(qconf,
                         qconf->tx_mbufs[portid].len,
                         (uint8_t) portid);
                qconf->tx_mbufs[portid].len = 0;
            }
            prev_tsc = cur_tsc;
        }

        /*
         * Read packet from RX queues
         */
        for (i = 0; i < qconf->n_rx_queue; i++) {
            portid = qconf->rx_queue_list[i].port_id;
            queueid = qconf->rx_queue_list[i].queue_id;

   	    nb_rx = rte_eth_rx_burst((uint8_t) portid, queueid,
                         pkts_burst, MAX_PKT_BURST);
	    batch_size = nb_rx;
	    if(unlikely(batch_size == 0)) continue;
	    for(int j = 0; j < batch_size; j++) {
		//rte_pktmbuf_free(pkts_burst[j]);
		rte_prefetch0(rte_pktmbuf_mtod( pkts_burst[j], void *));
		//packets_received(pd, 1, &pkts_burst[j], portid, qconf);	//Ankit-Application_Batching_disabled
	    }
	    packets_received(pd, batch_size, pkts_burst, portid, qconf);
	}
    }
}


static int
launch_one_lcore(__attribute__((unused)) void *dummy)
{
    dpdk_main_loop();
    return 0;
}

int launch_dpdk()
{

    /* Needed for L2 multicasting - e.g. acting as a hub
        cloning headers and sometimes packet data*/
    header_pool = rte_pktmbuf_pool_create("header_pool", NB_HDR_MBUF, 32,
            0, HDR_MBUF_DATA_SIZE, rte_socket_id());

    if (header_pool == NULL)
            rte_exit(EXIT_FAILURE, "Cannot init header mbuf pool\n");

    clone_pool = rte_pktmbuf_pool_create("clone_pool", NB_CLONE_MBUF, 32,
            0, 0, rte_socket_id());

    if (clone_pool == NULL)
            rte_exit(EXIT_FAILURE, "Cannot init clone mbuf pool\n");

    rte_eal_mp_remote_launch(launch_one_lcore, NULL, CALL_MASTER);

    unsigned lcore_id;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        if (rte_eal_wait_lcore(lcore_id) < 0)
            return -1;
    }
    return 0;
}

//init vlan table
void init_vlan_ingress(lookup_table_t** tables){
	//define table key
	struct universal_key new_key = universal_key_default();
	//set table size
	int num_vlan_ingress = 4, num_vlan_ids = 4;
	//define action value
	struct vlan_ingress_proc_action vaction_value;
	vaction_value.action_id = action__nop;

	int i = 0, j = 0; //indices for for-loops
	for( i = 0; i < num_vlan_ingress; i++ ) {
		new_key.vlan.ingress_port = i;
		for( j = 0; j < num_vlan_ids; j++ ){
			new_key.vlan.vid = j;
			//debug("VLAN ingress: %d, VLAN ID: %d\n", new_key.vlan.ingress_port, new_key.vlan.vid);
			exact_add_universal(tables[TABLE_vlan_ingress_proc], new_key, (uint8_t *) &vaction_value); 
		}
	}
	printf("VLAN ingress table created; entries filled %d\n", tables[TABLE_vlan_ingress_proc]->counter);

}

//init mac table
void init_mac(lookup_table_t** tables){
	//define table key
	struct universal_key new_key = universal_key_default();
	//set table size
	int num_eth_src = 256;
	//define action value
	struct mac_learning_action maction_value;
	maction_value.action_id = action__nop;

	int i = 0; //indices for for-loops
	for(i = 0; i <= num_eth_src; i++){
                //Add src mac
                int number = i, index = 5;
                uint8_t src[6];
                while (index >= 0) {
                        src[index] = number % 255;
                        number = number / 255;
                        index-=1;
                }
		src[0] = 160;
		src[1] = 54;
		src[2] = 159;
		exact_add_universal(tables[TABLE_mac_learning], new_key, (uint8_t *) &maction_value);
        //        key->eth.src = src;
        }
	printf("MAC table created; entries filled %d\n", tables[TABLE_mac_learning]->counter);

}

//init vlan table
void init_routable(lookup_table_t** tables){
	//define table key
	struct universal_key new_key = universal_key_default();
	//set table size
	int num_eth = 1024, num_vlan_ids = 4;
	//define action value
	struct routable_action raction_value;
	raction_value.action_id = action__nop;

	int i = 0, j = 0, k = 0; //indices for for-loops
	for( i = 0; i < num_vlan_ids; i++ ) {
		new_key.vlan.vid = i;
		for(j = 0; j < num_eth; j++){
			//memcpy(new_key.eth.src, real_macs[j], 6);
			//Add src mac
			int number = j, index = 5;
			uint8_t src[6];
			while (index >= 0) {
				src[index] = number % 255;
				number = number / 255;
				index-=1;
			}
			src[0] = 160;
			src[1] = 54;
			src[2] = 159;
			memcpy(new_key.eth.src, src, 6);
			memcpy(new_key.eth.dst, src, 6);
			//printf("Src:\t");
			//print_mac(new_key.eth.src);
//			for(k = 0; k < num_eth; k++){
//				//if(k == j){
//				//	continue;
//				//}
//				//memcpy(new_key.eth.dst, real_macs[k], 6);
//				//Add dst mac
//				int number = k, index = 5;
//				uint8_t src[6];
//				while (index >= 0) {
//					src[index] = number % 255;
//					number = number / 255;
//					index-=1;
//				}
//				src[0] = 160;
//				src[1] = 54;
//				src[2] = 159;
//				memcpy(new_key.eth.dst, src, 6);
//				//printf("Dst:\t");
				//print_mac(new_key.eth.dst);
				debug("Mac src: %d.%d.%d.%d.%d.%d, Mac dst: %d.%d.%d.%d.%d.%d, vid: %d\n", new_key.eth.src[0],new_key.eth.src[1],new_key.eth.src[2], new_key.eth.dst[3], new_key.eth.dst[4] ,new_key.eth.src[5],new_key.eth.dst[0],new_key.eth.dst[1],new_key.eth.dst[2],new_key.eth.dst[3], new_key.eth.dst[4], new_key.eth.dst[5], new_key.vlan.vid);

	                        exact_add_universal(tables[TABLE_routable], new_key, (uint8_t *) &raction_value);

//			}
		}	
	}
	printf("ROUTABLE table created; entries filled %d\n", tables[TABLE_routable]->counter);

}

//init vlan table
void init_switching(lookup_table_t** tables){
	//define table key
	struct universal_key new_key = universal_key_default();
	//set table size
	int num_eth = 4, num_vlan_ids = 4;
	//define action value
	struct switching_action switching_value;
	switching_value.action_id = action_forward;

	int i = 0, j = 0, k = 0; //indices for for-loops
	for( i = 0; i < num_vlan_ids; i++ ) {
		new_key.vlan.vid = i;
		for(k = 0; k < num_eth; k++){
				if(k == j){
					continue;
				}
				//memcpy(new_key.eth.dst, real_macs[k], 6);
				//Add dst mac
				int number = k, index = 5;
				uint8_t src[6];
				while (index >= 0) {
					src[index] = number % 255;
					number = number / 255;
					index-=1;
				}
				src[0] = 160;
				src[1] = 54;
				src[2] = 159;
				memcpy(new_key.eth.dst, src, 6);
				//printf("Dst:\t");
				//print_mac(new_key.eth.dst);
	                        exact_add_universal(tables[TABLE_switching], new_key, (uint8_t *) &switching_value);
		}	
	}
	printf("SWICHING table created; entries filled %d\n", tables[TABLE_switching]->counter);

}


void init_acl(lookup_table_t** tables) 
{
        //define key of combined table
        struct universal_key new_key = universal_key_default();
        uint8_t key[13] = {0}; 
	int num_ip, num_tp, index;
        //assuming VLAN {0,8} and MAC {0,255}
        //num_vlan = 4;
	//num_vid = 256;
	//num_mac = 4;
	num_ip = 4;
	num_tp = 1;
	//define output action value
        struct acl_action avalue;
        avalue.action_id = action__nop;

	//array for protocol numbers
	int ip_proto[1] = {4};
	//static real mac entries
	//160:54:159:14:54:72
	//160:54:159:14:54:74
	//160:54:159:62:235:162
	//160:54:159:62:235:164
	//uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for all for-loops
        int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0, q = 0 ;
									
	uint8_t ip[4] = {0,0,0,0};
	uint8_t depth = -1;
	uint8_t mask[13] = {255,255,255,255,255,255,255,255,255,255,255,255,255};
	
	FILE *fd_ipv4_src_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
	if(fd_ipv4_src_prefix == NULL){
		printf("Error: Can't open prefix file\n");
	}
	//int prefix_count_in_file = 527961;
	m = 0;
	for(m = 0; m < num_ip; m++){
		//printf("%d\n", i);

		int oct1, oct2, oct3, oct4, len;
		char temp_buff[64];
		if(fscanf(fd_ipv4_src_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
			&oct1, &oct2, &oct3, &oct4, &len) == EOF){
			printf("END OF prefix FILE reached\n");
			
			break;
		}

		key[3] = oct4; key[2] = oct3; key[1] = oct2; key[0] = oct1;
		
		key[7] = oct4; key[6] = oct3; key[5] = oct2; key[4] = oct1;
		key[8] = 221;
		debug("Oct1234: %d.%d.%d.%d\n", oct1, oct2, oct3, oct4);
//    		key[3] = 0; key[2] = 0; key[1] = 0; key[0] = 100;
//		
//		key[7] = 0; key[6] = 0; key[5] = 0; key[4] = 100;
//		key[8] = 221;
    		ternary_add(tables[TABLE_acl], key, mask, (uint8_t *) &avalue);
	}
	fclose(fd_ipv4_src_prefix);
        printf("ACL table created; entries filled %d\n", tables[TABLE_acl]->counter);
}

//init acl with uint8_t key
void
l2l3_acl_acl_init_table(lookup_table_t** tables) {

    uint8_t src[13] = {100, 42, 229, 0, 100, 42, 229, 0 ,6,0,0,0,0};
    struct acl_action avalue;
    avalue.action_id = action__nop;
    uint8_t mask[13] = {255,255,255,255,255,255,255,255,255,255,255,255,255};
    ternary_add(tables[TABLE_acl], src, mask, (uint8_t *) &avalue);

    debug("Table 6 init Done with entries\n");
}

//init vlan egress table
void init_vlan_egress(lookup_table_t** tables){
	//define table key
	struct universal_key new_key = universal_key_default();
	//set table size
	int num_vlan_egress = 4, num_vlan_ids = 4;
	//define action value
	struct vlan_egress_proc_action vaction_value;
	vaction_value.action_id = action__nop;

	int i = 0, j = 0; //indices for for-loops
	for( i = 0; i < num_vlan_egress; i++ ) {
		new_key.vlan.egress_port = i;
		for( j = 0; j < num_vlan_ids; j++ ){
			new_key.vlan.vid = j;
			exact_add_universal(tables[TABLE_vlan_egress_proc], new_key, (uint8_t *) &vaction_value); 
		}
	}
	printf("VLAN egress table created; entries filled %d\n", tables[TABLE_vlan_egress_proc]->counter);

}
//Anmol: Code for cross product vlan_ingress X mac
void compute_cross_product_table_12(lookup_table_t** tables) 
{
        //define key of combined table
        struct universal_key new_key = universal_key_default();
        int table1_entries, table2_entries;
        //assuming VLAN {0,8} and MAC {0,255}
        table1_entries = 4;
        table2_entries = 256;
        //define output action value
        struct mac_learning_action mvalue;
        mvalue.action_id = action_forward;

	//static real mac entries
	//160:54:159:14:54:72
	//160:54:159:14:54:74
	//160:54:159:62:235:162
	//160:54:159:62:235:164
	uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for two for loops
        int i = 0, j = 0;
        for(i = 0; i < table1_entries; i++){
                //Add VLAN entry in key
                new_key.vlan.ingress_port = i;
                for(j=0; j < 4; j++){
			memcpy(new_key.eth.src, real_macs[j], 6);
			print_mac(new_key.eth.src);
			
			debug("VLAN ingress: %d\n", new_key.vlan.ingress_port);
		  /* 
			printf("VLAN ingress inside new_key: %d\t MAC: %d:%d:%d\n",new_key.vlan.ingress_port, new_key.eth.src[3], new_key.eth.src[4], new_key.eth.src[5]);
			printf("Printing Key::VLAN\tvid: %d\tingress_port: %d\tegress_port: %d\n", new_key.vlan.vid, new_key.vlan.ingress_port, new_key.vlan.egress_port);	
			if(new_key.eth.src != NULL)
				printf("|Printing Key::ETH|\tMAC src: %d:%d:%d:%d:%d:%d\n",new_key.eth.src[0], new_key.eth.src[1], new_key.eth.src[2], new_key.eth.src[3], new_key.eth.src[4], new_key.eth.src[5]);
			if(new_key.ip.src != NULL)
				printf("|Printing Key::IP|\tIP protocol: %d\tIP src: %d:%d:%d:%d\tIP dst: %d:%d:%d:%d\n", new_key.ip.proto, new_key.ip.src[0], new_key.ip.src[1], new_key.ip.src[2], new_key.ip.src[3], new_key.ip.dst[0], new_key.ip.dst[1], new_key.ip.dst[2], new_key.ip.dst[3]);
			printf("|Printing Key::TP|\tTP src_port: %d\tTP dst_port: %d\n", new_key.tp.src_port, new_key.tp.dst_port);          
			debug("VLAN ingress inside new_key: %d\t MAC: %d:%d:%d\n",new_key.vlan.ingress_port, new_key.eth.src[3], new_key.eth.src[4], new_key.eth.src[5]);
		*/
			exact_add_universal(tables[TABLE_product], new_key, (uint8_t *) &mvalue);
                }
        }
        printf("Crossproduct table created; entried filled %d\n", tables[TABLE_product]->counter);
}

//Anmol: Code for cross product vlan_ingres X mac X routable
void compute_cross_product_table_123(lookup_table_t** tables) 
{
        //define key of combined table
        struct universal_key new_key = universal_key_default();
        int num_vlan, num_vid, num_mac, num_ip, num_tp;
        //assuming VLAN {0,8} and MAC {0,255}
        num_vlan = 4;
	num_vid = 256;
	num_mac = 4;
	num_ip = 4;
	num_tp = 4;
	//define output action value
        struct mac_learning_action mvalue;
        mvalue.action_id = action_forward;

	//array for protocol numbers
	int ip_proto[1] = {4};
	//static real mac entries
	//160:54:159:14:54:72
	//160:54:159:14:54:74
	//160:54:159:62:235:162
	//160:54:159:62:235:164
	//uint8_t real_macs[4][6] = {{160,54,159,14,54,72}, {160, 54, 159, 14, 54, 74}, {160,54,159,62,235,162}, {160,54,159,62,235,164}};

        //indices for all for-loops
        int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0, q = 0 ;
        for(i = 0; i < num_vlan; i++){
                //Add VLAN ingress in key
                new_key.vlan.ingress_port = i;
		//debug("Init cross product:: VLAN ingress: %d vlan id: %d vlan egress: %d\n", new_key.vlan.ingress_port, new_key.vlan.vid, new_key.vlan.egress_port);
		for( l = 0; l < num_vid; l++ ){
			//Add VIDs
			new_key.vlan.vid = l; 
			for(j = 0; j < num_mac; j++){
				//memcpy(new_key.eth.src, real_macs[j], 6);
				//Add src mac
				int number = j, index = 5;
				uint8_t src[6];
				while (index >= 0) {
					src[index] = number % 255;
					number = number / 255;
					index-=1;
				}
				src[0] = 160;
				src[1] = 54;
				src[2] = 159;
				memcpy(new_key.eth.src, src, 6);

				//printf("Src:\t");
				//print_mac(new_key.eth.src);
				for(k = 0; k < num_mac; k++){
					if(k == j){
						continue;
					}
					//memcpy(new_key.eth.dst, real_macs[k], 6);
					//Add dst mac
					int number = k, index = 5;
					uint8_t src[6];
					while (index >= 0) {
						src[index] = number % 255;
						number = number / 255;
						index-=1;
					}
					src[0] = 160;
					src[1] = 54;
					src[2] = 159;
					memcpy(new_key.eth.dst, src, 6);
					//printf("Dst:\t");
					//print_mac(new_key.eth.dst);
				
					
					uint8_t ip[4] = {0,0,0,0};
        				uint8_t depth = -1;

					FILE *fd_ipv4_src_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
					if(fd_ipv4_src_prefix == NULL){
						printf("Error: Can't open prefix file\n");
					}
					//int prefix_count_in_file = 527961;
					m = 0;
					for(m = 0; m < num_ip; m++){
						//printf("%d\n", i);

						int oct1, oct2, oct3, oct4, len;
						char temp_buff[64];
						if(fscanf(fd_ipv4_src_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
							&oct1, &oct2, &oct3, &oct4, &len) == EOF){
							printf("END OF prefix FILE reached\n");
							
							break;
						}

						
						new_key.ip.src[3] = oct4; new_key.ip.src[2] = oct3; new_key.ip.src[1] = oct2; new_key.ip.src[0] = oct1; 
						
						FILE *fd_ipv4_dst_prefix = fopen("/root/workspace/atul/ipv4/uniq_ipv4_rib_201409_1_percent", "r+");
						if(fd_ipv4_dst_prefix == NULL){
							printf("Error: Can't open prefix file\n");
						}
						//int prefix_count_in_file = 527961;
						n = 0;
						for(n = 0; n < num_ip; n++){
							//printf("%d\n", i);

							if(fscanf(fd_ipv4_dst_prefix, "%s %d.%d.%d.%d/%d", temp_buff,
								&oct1, &oct2, &oct3, &oct4, &len) == EOF){
								printf("END OF prefix FILE reached\n");
								
								break;
							}

							//ip[3] = oct4; ip[2] = oct3; ip[1] = oct2; ip[0] = oct1; 
							//depth = len;
							new_key.ip.dst[3] = oct4; new_key.ip.dst[2] = oct3; new_key.ip.dst[1] = oct2; new_key.ip.dst[0] = oct1; 
							//port[0] = i % NUM_PORT;
						
							debug("IPsrc: %d:%d:%d:%d, IPdst: %d:%d:%d:%d TP src: %d, TP dst %d\n", new_key.ip.src[3], new_key.ip.src[2], new_key.ip.src[1], new_key.ip.src[0], new_key.ip.dst[3], new_key.ip.dst[2], new_key.ip.dst[1], new_key.ip.dst[0], new_key.tp.src_port, new_key.tp.dst_port);
							//Add ip proto
							o = 0;
							for( o = 0; o < 1; o++ ){
								new_key.ip.proto = 1;

								for (p = 0; p < num_tp; p++){
									new_key.tp.src_port = p;	
									for(q = 0; q < num_tp; q++ ){
										new_key.tp.dst_port = q;
										exact_add_universal(tables[TABLE_product], new_key, (uint8_t *) &mvalue);	
										}
								}
							}
						}

						fclose(fd_ipv4_dst_prefix);
					}
					fclose(fd_ipv4_src_prefix);

	
						//exact_add_universal(tables[TABLE_product], new_key, (uint8_t *) &mvalue);	
				}//mac dst
			}//mac src
		}//vid
        }//vlan ingress
        printf("Crossproduct table created; entried filled %d\n", tables[TABLE_product]->counter);
}


//setting vlan entries
void set_vlan_entries(struct universal_key* key){
        int i=0;
        for(i = 0; i < 8; i++){
                key->vlan.ingress_port = i;
        }
}

//setting MAC entries
void set_mac_entries(struct universal_key* key, int entries){
        int i = 0;
        for(i = 0; i <= entries; i++){
                //Add src mac
                int number = i, index = 5;
                uint8_t src[6];
                while (index >= 0) {
                        src[index] = number % 255;
                        number = number / 255;
                        index-=1;
                }
        //        key->eth.src = src;
        }
        for(i = 0; i <= entries; i++){
                //Add dst mac
                int number = i, index = 5;
                uint8_t dst[6];
                while (index >= 0) {
                        dst[index] = number % 255;
                        number = number / 255;
                        index-=1;
                }
          //      key->eth.dst = dst;
        }
}
//setting ip address source and destination
void set_ip_entries(struct universal_key* key, int entries){
        int i = 0;
        //adding src
        for(i = 0; i < entries; i++){
                //Add ip entries
                int number = i, index = 3;
                uint8_t src[4];
                while (index >= 0){
                        src[index] = number % 255;
                        number = number % 255;
                        index -= 1;
                }
            //    key->ip.src = src;
        }
        //adding dst
        for(i = 0; i < entries; i++){
                //Add ip entries
                int number = i, index = 3;
                uint8_t dst[4];
                while (index >= 0){
                        dst[index] = number % 255;
                        number = number % 255;
                        index -= 1;
                }
              //  key->ip.dst = dst;
        }

}
//setting tcp port entries
void set_tp_entries(struct universal_key* key, int entries){
        int i = 0;
        for(i = 0; i < entries; i++){
                key->tp.src_port = i;
        }
        for(i = 0; i < entries; i++){
                key->tp.dst_port = i;
        }
}




