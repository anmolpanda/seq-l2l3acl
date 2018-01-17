 #include <stdlib.h>// sugar@17
 #include <string.h>// sugar@18
 #include "dpdk_lib.h"// sugar@19
 #include "actions.h"// sugar@20
 #include <arpa/inet.h>// sugar@21
	
 //#include "key.h" 
 extern void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@23

 extern void increase_counter (int counterid, int index);// sugar@25

 void apply_table_vlan_ingress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_mac_learning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_routable(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_switching(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_routing(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_acl(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_vlan_egress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_mcast_src_pruning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28
 void apply_table_product(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28



 uint8_t reverse_buffer[14];// sugar@31


 void table_vlan_ingress_proc_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
    int length = 0;
    for(int i = 0; i < batch_size; i++) {// sugar@72	
	//debug("VLAN ingress: %d, VLAN ID: %d before extraction\n", key[i].vlan.ingress_port, key[i].vlan.vid);
	uint8_t vid[2] = {0};	
	uint16_t vid_16 = 0;
	memcpy(&key[i].vlan.ingress_port, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_standard_metadata_ingress_port)), 2);
	memcpy(&key[i].vlan.vid, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_vlan__vid)), 2);
	//EXTRACT_BYTEBUF(&pd[i], field_instance_vlan__vid, vid);
	
	memcpy(&vid_16, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_vlan__vid)), 2);
	vid_16 = vid_16 & 0x0003;	
	key[i].vlan.vid = vid_16;
	debug("VLAN ingress: %d, VLAN ID: %d\n", key[i].vlan.ingress_port, key[i].vlan.vid);
	}
 }// sugar@106

 void table_mac_learning_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
    int length = 0;// sugar@79
    for(int i = 0; i < batch_size; i++) {// sugar@80
        uint8_t* address = FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__srcAddr));	// sugar@81
        memcpy(key[i].eth.src, address, 6);// sugar@82
 }// sugar@105
 }// sugar@106

 void table_routable_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
    int length = 0;// sugar@79
    for(int i = 0; i < batch_size; i++) {// sugar@80
        memcpy(key[i].eth.src,  FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__srcAddr)), 6);// sugar@82
        memcpy(key[i].eth.dst,  FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__dstAddr)), 6);// sugar@82
	memcpy(&key[i].vlan.vid, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_vlan__vid)), 2);
	key[i].vlan.vid = key[i].vlan.vid & 0x0003;
	//key[i].vlan.vid = 120;
	//debug("Mac src: %d.%d.%d.%d.%d.%d, Mac dst: %d.%d.%d.%d.%d.%d, vid: %d\n", key[i].eth.src[0],key[i].eth.src[1],key[i].eth.src[2], key[i].eth.dst[3], key[i].eth.dst[4] ,key[i].eth.src[5],key[i].eth.dst[0],key[i].eth.dst[1],key[i].eth.dst[2],key[i].eth.dst[3], key[i].eth.dst[4], key[i].eth.dst[5], key[i].vlan.vid);
	
}
}
 void table_switching_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
    int length = 0;// sugar@79
    for(int i = 0; i < batch_size; i++) {// sugar@80
 	memcpy(key[i].eth.dst, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__dstAddr)), 6);	
	//memcpy(&key[i].vlan.vid, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_vlan__vid)), 2);
	
	// sugar@105
 }// sugar@106
}
 void table_routing_key(packet_descriptor_t* pd, int batch_size, uint8_t* key[batch_size], uint16_t* len) {// sugar@68
 for(int i = 0; i< batch_size; i++) {// sugar@72
     EXTRACT_INT32_BITS(&pd[i], field_instance_ipv4__dstAddr, *(uint32_t*)key[i])// sugar@73
    uint32_t ip = ntohl(*(uint32_t *)key[i]);// sugar@98
    memcpy(key[i], &ip, 4);// sugar@99
 }// sugar@105
 }// sugar@106


 void table_acl_key(packet_descriptor_t* pd, int batch_size, uint8_t key[batch_size][13], uint16_t* len) {// sugar@68
 for(int i = 0; i< batch_size; i++) {// sugar@72
 	 int index = 0;
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__srcAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__dstAddr, &key[i][index])// sugar@49
	 index += sizeof(uint32_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_ipv4__protocol, &key[i][index])// sugar@49
	 index += sizeof(uint8_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__srcPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
	 EXTRACT_BYTEBUF(&pd[i], field_instance_l4_metadata__dstPort, &key[i][index])// sugar@49
	 index += sizeof(uint16_t);// sugar@50
	 //key[i][0] = 100; key[i][1] = 0; key[i][2] = 0; key[i][3] = 0;
	 //key[i][4] = 100; key[i][5] = 0; key[i][6] = 0; key[i][7] = 0;
	 //key[i][8] = 221; key[i][9] = 0; key[i][10] = 0; key[i][11] = 0; key[i][12] = 0;
	 //debug("IP src: %d.%d.%d.%d, IP dst: %d.%d.%d.%d, IP Protocol: %d\n", key[i][0], key[i][1], key[i][2], key[i][3], key[i][4], key[i][5], key[i][6], key[i][7], key[i][8] );
 	 //debug("TP src: %d,%d, TP dst: %d,%d\n", key[i][9], key[i][10], key[i][11], key[i][12]);
 }// sugar@105
}// sugar@106

 void table_vlan_egress_proc_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
 for(int i = 0; i< batch_size; i++) {// sugar@72
     //EXTRACT_INT32_BITS(&pd[i], field_instance_standard_metadata_egress_spec, *(uint32_t*)key[i])// sugar@73
     memcpy(&key[i].vlan.egress_port, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_standard_metadata_egress_port)), 2);
	
 }// sugar@105
 }// sugar@106

 void table_mcast_src_pruning_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][4], uint16_t* len) {// sugar@68
 for(int i = 0; i< batch_size; i++) {// sugar@72
     EXTRACT_INT32_BITS(&pd[i], field_instance_standard_metadata_instance_type, *(uint32_t*)key[i])// sugar@73
 }// sugar@105
 }// sugar@106

 void table_product_key(packet_descriptor_t* pd, int batch_size, struct universal_key key[batch_size], uint16_t* len) {// sugar@68
	for(int i = 0; i < batch_size; i++) {
		//vlan ingress and vid
		memcpy(&key[i].vlan.ingress_port, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_standard_metadata_ingress_port)), 2);
		//memcpy(&key[i].vlan.egress_port, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_standard_metadata_egress_port)), 2);
		memcpy(&key[i].vlan.vid, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_vlan__vid)), 1);
		//mac src and dst
		memcpy(key[i].eth.src, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__srcAddr)), 6);	
		memcpy(key[i].eth.dst, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ethernet__dstAddr)), 6);
//		debug("VLAN ingress: %d, VLAN egress: %d\n", key[i].vlan.ingress_port, key[i].vlan.egress_port);
		memcpy(key[i].ip.src, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ipv4__srcAddr)),  4);
		memcpy(key[i].ip.dst, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ipv4__dstAddr)),  4);
		memcpy(&key[i].ip.proto, FIELD_BYTE_ADDR(&pd[i], field_desc(field_instance_ipv4__protocol)),  4);	
		//tcp port
		memcpy(&key[i].tp.src_port, FIELD_BYTE_ADDR(&pd[i], field_desc( field_instance_l4_metadata__srcPort)),  4);
		memcpy(&key[i].tp.dst_port, FIELD_BYTE_ADDR(&pd[i], field_desc( field_instance_l4_metadata__dstPort)),  4);
		//debug("TP src: %d, TP dst: %d\n", key[i].tp.src_port, key[i].tp.dst_port);
		//debug("EXTRACTION::IPsrc: %d:%d:%d:%d, IPdst: %d:%d:%d:%d\n", key[i].ip.src[3], key[i].ip.src[2], key[i].ip.src[1], key[i].ip.src[0], key[i].ip.dst[3], key[i].ip.dst[2], key[i].ip.dst[1], key[i].ip.dst[0]);
	
		}
}

 void apply_table_vlan_ingress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     struct universal_key new_key[batch_size];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE vlan_ingress_proc\n");// sugar@117
     
     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }

     table_vlan_ingress_proc_key(pd, batch_size, new_key, length);// sugar@118
     exact_lookup(tables[TABLE_vlan_ingress_proc], batch_size, 3, new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct vlan_ingress_proc_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct vlan_ingress_proc_action* res = (struct vlan_ingress_proc_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             //debug(" VLAN   :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_add_vlan:// sugar@133
             //debug("    :: EXECUTING ACTION add_vlan...\n");// sugar@134
             action_code_add_vlan(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__nop:// sugar@133
             debug("VLAN    :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    return apply_table_mac_learning(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_mac_learning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][6];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE mac_learning\n");// sugar@117
      
     struct universal_key new_key[batch_size];
     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }

     table_mac_learning_key(pd, batch_size, new_key, length);// sugar@118
     exact_lookup(tables[TABLE_mac_learning], batch_size, 6, new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct mac_learning_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct mac_learning_action* res = (struct mac_learning_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             //debug("  MAC  :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_mac_learn:// sugar@133
             //debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@134
             action_code_mac_learn(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__nop:// sugar@133
             debug(" MAC   :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    //Anmol: Retrict number of lookups to two
    return apply_table_routable(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_routable(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][14];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE routable\n");// sugar@117
     
     struct universal_key new_key[batch_size];
     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }

     table_routable_key(pd, batch_size, new_key, length);// sugar@118
     exact_lookup(tables[TABLE_routable], batch_size, sizeof(struct universal_key), new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct routable_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct routable_action* res = (struct routable_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug("ROUTABLE  :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_route:// sugar@133
             debug("ROUTABLE    :: EXECUTING ACTION route...\n");// sugar@134
             action_code_route(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__nop:// sugar@133
             debug("ROUTABLE    :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
     debug("Batch size: %d\n", batch_size);
    //return apply_table_routing(pd, batch_size, tables);// sugar@151
    return apply_table_switching(pd, batch_size, tables);// sugar@151

 }// sugar@158

 void apply_table_switching(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][8];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE switching\n");// sugar@117
      
     struct universal_key new_key[batch_size];
     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }
     table_switching_key(pd, batch_size, new_key, length);// sugar@118
     exact_lookup(tables[TABLE_switching], batch_size, sizeof(struct universal_key), new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct switching_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct switching_action* res = (struct switching_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug(" SWITCHING   :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_forward:// sugar@133
             debug("SWITCHING   :: EXECUTING ACTION forward...\n");// sugar@134
             action_code_forward(&pd[i], tables, res->forward_params);// sugar@136
         break;// sugar@139
         case action_broadcast:// sugar@133
             //debug("    :: EXECUTING ACTION broadcast...\n");// sugar@134
             action_code_broadcast(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    //debug("SWITCHING Batch_size: %d\n", batch_size); 
    return apply_table_acl(pd, batch_size, tables);// sugar@151
    //return apply_table_vlan_egress_proc(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_routing(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][4];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE routing\n");// sugar@117
     
     struct universal_key new_key[batch_size];
     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }
 
     table_routing_key(pd, batch_size, new_key, length);// sugar@118
     lpm_lookup(tables[TABLE_routing], batch_size, sizeof(struct universal_key), new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct routing_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct routing_action* res = (struct routing_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_set_nhop:// sugar@133
             //debug("    :: EXECUTING ACTION set_nhop...\n");// sugar@134
             action_code_set_nhop(&pd[i], tables, res->set_nhop_params);// sugar@136
         break;// sugar@139
         case action__drop:// sugar@133
             //debug("    :: EXECUTING ACTION _drop...\n");// sugar@134
             action_code__drop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    return apply_table_switching(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_acl(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     uint8_t key[batch_size][13];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE acl\n");// sugar@117
     
     table_acl_key(pd, batch_size, key, length);// sugar@118
        
     //int index = *(int*)(values[i] +sizeof(struct acl_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         
     	 values[i] = ternary_lookup(tables[TABLE_acl], (uint8_t*)key[i]);// sugar@73
	 struct acl_action* res = (struct acl_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug(" ACL   :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action__nop:// sugar@133
             debug("ACL   :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__drop:// sugar@133
             //debug("    :: EXECUTING ACTION _drop...\n");// sugar@134
             action_code__drop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    //debug("ACL BATCH_size: %d\n", batch_size);
    return apply_table_vlan_egress_proc(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_vlan_egress_proc(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][2];// sugar@114
     debug("EGRESS Batch_size: %d\n", batch_size);
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE vlan_egress_proc\n");// sugar@117
     struct universal_key new_key[batch_size];
     for(int i=0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }
     
     table_vlan_egress_proc_key(pd, batch_size, new_key, length);// sugar@118
     exact_lookup(tables[TABLE_vlan_egress_proc], batch_size, sizeof(struct universal_key), new_key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct vlan_egress_proc_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct vlan_egress_proc_action* res = (struct vlan_egress_proc_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             //debug("  VLAN EGRESS  :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_strip_vlan:// sugar@133
             //debug("    :: EXECUTING ACTION strip_vlan...\n");// sugar@134
             action_code_strip_vlan(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__nop:// sugar@133
             debug("    :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    //return apply_table_product(pd, batch_size, tables);// sugar@151
 }// sugar@158

 void apply_table_mcast_src_pruning(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     uint8_t key[batch_size][4];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     //debug("  :::: EXECUTING TABLE mcast_src_pruning\n");// sugar@117
     table_mcast_src_pruning_key(pd, batch_size, key, length);// sugar@118
     exact_lookup(tables[TABLE_mcast_src_pruning], batch_size, 4, key, values);// sugar@119
     //int index = *(int*)(values[i] +sizeof(struct mcast_src_pruning_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct mcast_src_pruning_action* res = (struct mcast_src_pruning_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug("MCAST_SRC_PRUNING :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action__nop:// sugar@133
             //debug("    :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         case action__drop:// sugar@133
             //debug("    :: EXECUTING ACTION _drop...\n");// sugar@134
             action_code__drop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    // sugar@151
 }// sugar@158


 void apply_table_product(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112
 {// sugar@113
     //uint8_t key[batch_size][8]; //TODO: Change the size of array accourdingly
     struct universal_key new_key[batch_size];
     //uint8_t key[batch_size][7];// sugar@114
     uint8_t* values[batch_size];// sugar@115
     uint16_t length[batch_size];// sugar@116
     debug("  :::: EXECUTING TABLE product\n");// sugar@117

     for(int i =0; i< batch_size; i++){
	new_key[i] = universal_key_default();
     }
     table_product_key(pd, batch_size, new_key, length);// sugar@118 
/*     for(int i = 0; i < batch_size; i++) {
	new_key[i] = universal_key_default();
	new_key[i].eth.src = &key[i][2];
	new_key[i].vlan.ingress_port  = key[i][1];
     }
*/
     exact_lookup(tables[TABLE_product], batch_size, sizeof(struct universal_key), new_key, values);// sugar@119

     //int index = *(int*)(values[i] +sizeof(struct product_action)); (void)index;// sugar@122
     for(int i = 0; i < batch_size; i++) {// sugar@123
         struct product_action* res = (struct product_action*)values[i];// sugar@124
         if(res == NULL) {// sugar@127
             debug("PRODUCT TABLE :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@128
             continue;// sugar@129
         }// sugar@130
         switch (res->action_id) {// sugar@131
         case action_forward:// sugar@133
             debug("    :: EXECUTING ACTION forward...\n");// sugar@134
             action_code_forward(&pd[i], tables, res->forward_params);// sugar@136
         break;// sugar@139
         case action__nop:// sugar@133
             //debug("    :: EXECUTING ACTION _nop...\n");// sugar@134
             action_code__nop(&pd[i], tables);// sugar@138
         break;// sugar@139
         }// sugar@140
     }// sugar@141
    //if ((GET_INT32_AUTO(pd, field_instance_standard_metadata_ingress_port))==(GET_INT32_AUTO(pd, field_instance_standard_metadata_egress_spec))) { return apply_table_mcast_src_pruning(pd, batch_size, tables); } else {  }// sugar@151
 }// sugar@158

 void init_headers(packet_descriptor_t* packet_desc) {// sugar@161
 packet_desc->headers[header_instance_standard_metadata] = (header_descriptor_t) { .type = header_instance_standard_metadata, .length = header_instance_byte_width[header_instance_standard_metadata],// sugar@165
                               .pointer = calloc(header_instance_byte_width[header_instance_standard_metadata], sizeof(uint8_t)) };// sugar@166
 packet_desc->headers[header_instance_ethernet_] = (header_descriptor_t) { .type = header_instance_ethernet_, .length = header_instance_byte_width[header_instance_ethernet_], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_universal_key] = (header_descriptor_t) { .type = header_instance_universal_key, .length = header_instance_byte_width[header_instance_universal_key], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_vlan_] = (header_descriptor_t) { .type = header_instance_vlan_, .length = header_instance_byte_width[header_instance_vlan_], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_ipv4_] = (header_descriptor_t) { .type = header_instance_ipv4_, .length = header_instance_byte_width[header_instance_ipv4_], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_tcp_] = (header_descriptor_t) { .type = header_instance_tcp_, .length = header_instance_byte_width[header_instance_tcp_], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_udp_] = (header_descriptor_t) { .type = header_instance_udp_, .length = header_instance_byte_width[header_instance_udp_], .pointer = NULL };// sugar@168
 packet_desc->headers[header_instance_intrinsic_metadata] = (header_descriptor_t) { .type = header_instance_intrinsic_metadata, .length = header_instance_byte_width[header_instance_intrinsic_metadata],// sugar@165
                               .pointer = calloc(header_instance_byte_width[header_instance_intrinsic_metadata], sizeof(uint8_t)) };// sugar@166
 packet_desc->headers[header_instance_l4_metadata_] = (header_descriptor_t) { .type = header_instance_l4_metadata_, .length = header_instance_byte_width[header_instance_l4_metadata_],// sugar@165
                               .pointer = calloc(header_instance_byte_width[header_instance_l4_metadata_], sizeof(uint8_t)) };// sugar@166
 }// sugar@169


 void init_keyless_tables() {// sugar@177
 }// sugar@185

 void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@187
     init_headers(pd);// sugar@188
     init_keyless_tables();// sugar@189
 }// sugar@190
 
 void handle_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@193
 {// sugar@194
     //int value32;// sugar@195
     //EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@196
     //debug("### HANDLING PACKET ARRIVING AT PORT %" PRIu32 "...\n", value32);// sugar@197
     parse_packets(pd, batch_size, tables);// sugar@198
 }// sugar@199
