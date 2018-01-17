#include<stdio.h>
#include<linux/types.h>
#define ETH_ALEN 6

//define key struct
struct universal_key {
       	struct {
		uint16_t vid; //vlan id
		uint16_t ingress_port; //vlan in port
		uint16_t egress_port;  //vlan out port
	} vlan;
	struct {
                uint8_t src[6];   /* Ethernet source address. */
                uint8_t dst[6];   /* Ethernet destination address. */
        } eth;
        struct {
                uint8_t proto;   /* IP protocol or lower 8 bits of ARP opcode. */
        	uint8_t src[4];  /*Source IP*/
		uint8_t dst[4];	 /*Destination IP*/	
	} ip;
        struct {
                uint32_t src_port;             /* TCP/UDP/SCTP source port. */
                uint32_t dst_port;             /* TCP/UDP/SCTP destination port. */
        } tp; 
};/* Ensure that we can do comparisons as longs. */

struct universal_key inline universal_key_default(void) {
    struct universal_key default_key = {.vlan={0,2,2}, .eth ={{0}, {0}}, .ip = {1, {0}, {0}}, .tp = {1,1}};
    return default_key;
}
//
//void print_key(struct universal_key key){
//	printf("Printing Key::VLAN\tvid: %d\tingress_port: %d\tegress_port: %d\n", key.vlan.vid, key.vlan.ingress_port, key.vlan.egress_port);	
//	printf("|Printing Key::ETH|\tMAC src: %d:%d:%d:%d:%d:%d\tMAC dst: %d:%d:%d:%d:%d:%d\n",key.eth.src[0], key.eth.src[1], key.eth.src[2], key.eth.src[3], key.eth.src[4], key.eth.src[5], key.eth.dst[0], key.eth.dst[1], key.eth.dst[2], key.eth.dst[3], key.eth.dst[4], key.eth.dst[5]);
//	printf("|Printing Key::IP|\tIP protocol: %d\tIP src: %d:%d:%d:%d\tIP dst: %d:%d:%d:%d\n", key.ip.proto, key.ip.src[0], key.ip.src[1], key.ip.src[2], key.ip.src[3], key.ip.dst[0], key.ip.dst[1], key.ip.dst[2], key.ip.dst[3]);
//	printf("|Printing Key::TP|\tTP src_port: %d\tTP dst_port: %d\n", key.tp.src_port, key.tp.dst_port);
//}
//
