 #include "dpdk_lib.h"// sugar@72
 #include "actions.h" // apply_table_* and action_code_*// sugar@73

 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@75
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@76
 
 static void// sugar@78
 extract_header(uint8_t* buf, packet_descriptor_t* pd, header_instance_t h) {// sugar@79
     pd->headers[h] =// sugar@80
       (header_descriptor_t) {// sugar@81
         .type = h,// sugar@82
         .pointer = buf,// sugar@83
         .length = header_instance_byte_width[h]// sugar@84
       };// sugar@85
 }// sugar@86
 
 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static void parse_state_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98
 static void parse_state_parse_vlan(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98
 static void parse_state_parse_tcp(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98
 static void parse_state_parse_udp(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98

 static inline void build_key_parse_ethernet(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@104
 EXTRACT_INT32_BITS(pd, field_instance_ethernet__etherType, *(uint32_t*)key)// sugar@110
 key += sizeof(uint32_t);// sugar@111
 }// sugar@120
 static inline void build_key_parse_vlan(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@104
 EXTRACT_INT32_BITS(pd, field_instance_vlan__etherType, *(uint32_t*)key)// sugar@110
 key += sizeof(uint32_t);// sugar@111
 }// sugar@120
 static inline void build_key_parse_ipv4(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@104
 EXTRACT_INT32_BITS(pd, field_instance_ipv4__ihl, *(uint32_t*)key)// sugar@110
 key += sizeof(uint32_t);// sugar@111
 EXTRACT_INT32_BITS(pd, field_instance_ipv4__protocol, *(uint32_t*)key)// sugar@110
 key += sizeof(uint32_t);// sugar@111
 }// sugar@120
 static void parse_state_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
        return parse_state_parse_ethernet(pd, batch_size, tables);// sugar@21
// sugar@159
 }// sugar@197
 
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     int total_keys = 0;// sugar@129
     for(int i = 0; i < batch_size; i++) {// sugar@130
         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131
         extract_header(buf, &pd[i], header_instance_ethernet_);// sugar@139
         buf += header_instance_byte_width[header_instance_ethernet_];// sugar@140
         uint8_t key[2];// sugar@162
         build_key_parse_ethernet(pd, buf, key);// sugar@163
         uint8_t case_value_0[2] = {// sugar@177
         8,// sugar@179
         0,// sugar@179
         };// sugar@180
         if ( memcmp(key, case_value_0, 2) == 0) {// sugar@181
             total_keys++;// sugar@182
             if(unlikely(total_keys == batch_size))// sugar@183
                     return parse_state_parse_ipv4(pd, batch_size, tables);// sugar@21
         }// sugar@184
         uint8_t case_value_1[2] = {// sugar@177
         129,// sugar@179
         0,// sugar@179
         };// sugar@180
         //if ( memcmp(key, case_value_1, 2) == 0) {// sugar@181
         //    total_keys++;// sugar@182
         //    if(unlikely(total_keys == batch_size))// sugar@183
         //            return parse_state_parse_vlan(pd, batch_size, tables);// sugar@21
         //}// sugar@184
     }// sugar@171
     return parse_state_parse_vlan(pd, batch_size, tables);// sugar@21

	//return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@172
 }// sugar@197
 
 static void parse_state_parse_vlan(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     //printf("vlan parse\n");
     int total_keys = 0;// sugar@129
     for(int i = 0; i < batch_size; i++) {// sugar@130
         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131
         buf += header_instance_byte_width[header_instance_ethernet_];// sugar@138
         buf += header_instance_byte_width[header_instance_ipv4_];// sugar@138
         buf += header_instance_byte_width[header_instance_tcp_];// sugar@138
         buf += header_instance_byte_width[header_instance_udp_];// sugar@138
         extract_header(buf, &pd[i], header_instance_vlan_);// sugar@139
         buf += header_instance_byte_width[header_instance_vlan_];// sugar@140
         uint8_t key[2];// sugar@162
         build_key_parse_vlan(pd, buf, key);// sugar@163
         uint8_t case_value_0[2] = {// sugar@177
         8,// sugar@179
         0,// sugar@179
         };// sugar@180
         //if ( memcmp(key, case_value_0, 2) == 0) {// sugar@181
         //    total_keys++;// sugar@182
         //    if(unlikely(total_keys == batch_size))// sugar@183
         //            return parse_state_parse_ipv4(pd, batch_size, tables);// sugar@21
         //}// sugar@184
     }// sugar@171
	 return parse_state_parse_ipv4(pd, batch_size, tables);
    // return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@172
 }// sugar@197
 
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     int total_keys = 0;// sugar@129
     for(int i = 0; i < batch_size; i++) {// sugar@130
         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131
         buf += header_instance_byte_width[header_instance_ethernet_];// sugar@138
         buf += header_instance_byte_width[header_instance_vlan_];// sugar@138
         buf += header_instance_byte_width[header_instance_tcp_];// sugar@138
         buf += header_instance_byte_width[header_instance_udp_];// sugar@138
         extract_header(buf, &pd[i], header_instance_ipv4_);// sugar@139
         buf += header_instance_byte_width[header_instance_ipv4_];// sugar@140
         uint8_t key[2];// sugar@162
         build_key_parse_ipv4(pd, buf, key);// sugar@163
         uint8_t case_value_0[2] = {// sugar@177
         5,// sugar@179
         6,// sugar@179
         };// sugar@180
         /*if ( memcmp(key, case_value_0, 2) == 0) {// sugar@181
             total_keys++;// sugar@182
             if(unlikely(total_keys == batch_size))// sugar@183
                     return parse_state_parse_tcp(pd, batch_size, tables);// sugar@21
         }// sugar@184*/ //Commented by Anmol
         uint8_t case_value_1[2] = {// sugar@177
         5,// sugar@179
         17,// sugar@179
         };// sugar@180
         /*if ( memcmp(key, case_value_1, 2) == 0) {// sugar@181
             total_keys++;// sugar@182
             if(unlikely(total_keys == batch_size))// sugar@183
                     return parse_state_parse_udp(pd, batch_size, tables);// sugar@21
         }// sugar@184*///Commented by Anmol to correct code flow
     }// sugar@171
     return parse_state_parse_tcp(pd, batch_size, tables);// sugar@21

	//return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@172
 }// sugar@197
 
 static void parse_state_parse_tcp(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     int total_keys = 0;// sugar@129
     for(int i = 0; i < batch_size; i++) {// sugar@130
         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131
         buf += header_instance_byte_width[header_instance_ethernet_];// sugar@138
         buf += header_instance_byte_width[header_instance_vlan_];// sugar@138
         buf += header_instance_byte_width[header_instance_ipv4_];// sugar@138
         buf += header_instance_byte_width[header_instance_udp_];// sugar@138
         extract_header(buf, &pd[i], header_instance_tcp_);// sugar@139
         buf += header_instance_byte_width[header_instance_tcp_];// sugar@140
    }// sugar@158
    //Anmol: Call apply_table_xyz
    //return apply_table_product(pd, batch_size, tables);
    return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@159
 }// sugar@197
 
 static void parse_state_parse_udp(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     int total_keys = 0;// sugar@129
     for(int i = 0; i < batch_size; i++) {// sugar@130
         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131
         buf += header_instance_byte_width[header_instance_ethernet_];// sugar@138
         buf += header_instance_byte_width[header_instance_vlan_];// sugar@138
         buf += header_instance_byte_width[header_instance_ipv4_];// sugar@138
         buf += header_instance_byte_width[header_instance_tcp_];// sugar@138
         extract_header(buf, &pd[i], header_instance_udp_);// sugar@139
         buf += header_instance_byte_width[header_instance_udp_];// sugar@140
    }// sugar@158
    return apply_table_vlan_ingress_proc(pd, batch_size, tables);// sugar@159
 }// sugar@197
 
 void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables) {// sugar@200
     parse_state_start(pd, batch_size, tables);// sugar@201
 }// sugar@202
