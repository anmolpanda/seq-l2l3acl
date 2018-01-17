 #include "dpdk_lib.h"// sugar@22
 #include "actions.h"// sugar@23
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28

 extern void table_vlan_ingress_proc_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_mac_learning_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_routable_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_switching_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_routing_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_acl_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_vlan_egress_proc_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_mcast_src_pruning_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_product_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31

 uint8_t reverse_buffer[14];// sugar@34
 void// sugar@38
 vlan_ingress_proc_add(// sugar@39
 uint8_t field_instance_standard_metadata_ingress_port[2],// sugar@42
 uint8_t field_instance_vlan__vid[2],// sugar@42
 struct vlan_ingress_proc_action action)// sugar@48
 {// sugar@49
     uint8_t key[3];// sugar@50
 memcpy(key+0, field_instance_standard_metadata_ingress_port, 2);// sugar@55
 memcpy(key+2, field_instance_vlan__vid, 2);// sugar@55
 exact_add_promote(TABLE_vlan_ingress_proc, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 vlan_ingress_proc_setdefault(struct vlan_ingress_proc_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_vlan_ingress_proc, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 mac_learning_add(// sugar@39
 uint8_t field_instance_ethernet__srcAddr[6],// sugar@42
 struct mac_learning_action action)// sugar@48
 {// sugar@49
     uint8_t key[6];// sugar@50
 memcpy(key+0, field_instance_ethernet__srcAddr, 6);// sugar@55
 exact_add_promote(TABLE_mac_learning, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 mac_learning_setdefault(struct mac_learning_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_mac_learning, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 routable_add(// sugar@39
 uint8_t field_instance_ethernet__srcAddr[6],// sugar@42
 uint8_t field_instance_ethernet__dstAddr[6],// sugar@42
 uint8_t field_instance_vlan__vid[2],// sugar@42
 struct routable_action action)// sugar@48
 {// sugar@49
     uint8_t key[14];// sugar@50
 memcpy(key+0, field_instance_ethernet__srcAddr, 6);// sugar@55
 memcpy(key+6, field_instance_ethernet__dstAddr, 6);// sugar@55
 memcpy(key+12, field_instance_vlan__vid, 2);// sugar@55
 exact_add_promote(TABLE_routable, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 routable_setdefault(struct routable_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_routable, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 switching_add(// sugar@39
 uint8_t field_instance_ethernet__dstAddr[6],// sugar@42
 uint8_t field_instance_vlan__vid[2],// sugar@42
 struct switching_action action)// sugar@48
 {// sugar@49
     uint8_t key[8];// sugar@50
 memcpy(key+0, field_instance_ethernet__dstAddr, 6);// sugar@55
 memcpy(key+6, field_instance_vlan__vid, 2);// sugar@55
 exact_add_promote(TABLE_switching, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 switching_setdefault(struct switching_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_switching, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 routing_add(// sugar@39
 uint8_t field_instance_ipv4__dstAddr[4],// sugar@42
 uint8_t field_instance_ipv4__dstAddr_prefix_length,// sugar@47
 struct routing_action action)// sugar@48
 {// sugar@49
     uint8_t key[4];// sugar@50
 memcpy(key+0, field_instance_ipv4__dstAddr, 4);// sugar@55
 uint8_t prefix_length = 0;// sugar@58
 prefix_length += field_instance_ipv4__dstAddr_prefix_length;// sugar@64
 int c, d;// sugar@65
 for(c = 3, d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@66
 for(c = 0; c < 4; c++) *(key+c) = *(reverse_buffer+c);// sugar@67
 lpm_add_promote(TABLE_routing, (uint8_t*)key, prefix_length, (uint8_t*)&action);// sugar@68
 }// sugar@73

 void// sugar@75
 routing_setdefault(struct routing_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_routing, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 acl_add(// sugar@39
 uint8_t field_instance_ipv4__srcAddr[4],// sugar@42
 //uint8_t field_instance_ipv4__srcAddr_mask[4],// sugar@45
 uint8_t field_instance_ipv4__dstAddr[4],// sugar@42
 //uint8_t field_instance_ipv4__dstAddr_mask[4],// sugar@45
 uint8_t field_instance_ipv4__protocol[1],// sugar@42
 //uint8_t field_instance_ipv4__protocol_mask[1],// sugar@45
 uint8_t field_instance_l4_metadata__srcPort[2],// sugar@42
 //uint8_t field_instance_l4_metadata__srcPort_mask[2],// sugar@45
 uint8_t field_instance_l4_metadata__dstPort[2],// sugar@42
 //uint8_t field_instance_l4_metadata__dstPort_mask[2],// sugar@45
 struct acl_action action)// sugar@48
 {// sugar@49
     uint8_t key[13];// sugar@50
 memcpy(key+0, field_instance_ipv4__srcAddr, 4);// sugar@55
 memcpy(key+4, field_instance_ipv4__dstAddr, 4);// sugar@55
 memcpy(key+8, field_instance_ipv4__protocol, 1);// sugar@55
 memcpy(key+9, field_instance_l4_metadata__srcPort, 2);// sugar@55
 memcpy(key+11, field_instance_l4_metadata__dstPort, 2);// sugar@55
 }// sugar@73

 void// sugar@75
 acl_setdefault(struct acl_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_acl, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 vlan_egress_proc_add(// sugar@39
 uint8_t field_instance_standard_metadata_egress_spec[2],// sugar@42
 struct vlan_egress_proc_action action)// sugar@48
 {// sugar@49
     uint8_t key[2];// sugar@50
 memcpy(key+0, field_instance_standard_metadata_egress_spec, 2);// sugar@55
 exact_add_promote(TABLE_vlan_egress_proc, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 vlan_egress_proc_setdefault(struct vlan_egress_proc_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_vlan_egress_proc, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 mcast_src_pruning_add(// sugar@39
 uint8_t field_instance_standard_metadata_instance_type[4],// sugar@42
 struct mcast_src_pruning_action action)// sugar@48
 {// sugar@49
     uint8_t key[4];// sugar@50
 memcpy(key+0, field_instance_standard_metadata_instance_type, 4);// sugar@55
 exact_add_promote(TABLE_mcast_src_pruning, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 mcast_src_pruning_setdefault(struct mcast_src_pruning_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_mcast_src_pruning, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 product_add(// sugar@39
 uint8_t field_instance_universal_key_eth_src[6],// sugar@42
 uint8_t field_instance_universal_key_vlan_ingress[1],// sugar@42
 struct product_action action)// sugar@48
 {// sugar@49
     uint8_t key[7];// sugar@50
 memcpy(key+0, field_instance_universal_key_eth_src, 6);// sugar@55
 memcpy(key+6, field_instance_universal_key_vlan_ingress, 1);// sugar@55
 exact_add_promote(TABLE_product, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 product_setdefault(struct product_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_product, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@83
 vlan_ingress_proc_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_standard_metadata_ingress_port = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@88
 if(strcmp("add_vlan", ctrl_m->action_name)==0) {// sugar@93
     struct vlan_ingress_proc_action action;// sugar@94
     action.action_id = action_add_vlan;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to vlan_ingress_proc with action add_vlan\n");// sugar@100
     vlan_ingress_proc_add(// sugar@101
 field_instance_standard_metadata_ingress_port,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct vlan_ingress_proc_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to vlan_ingress_proc with action _nop\n");// sugar@100
     vlan_ingress_proc_add(// sugar@101
 field_instance_standard_metadata_ingress_port,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 mac_learning_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ethernet__srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@93
     struct mac_learning_action action;// sugar@94
     action.action_id = action_mac_learn;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to mac_learning with action mac_learn\n");// sugar@100
     mac_learning_add(// sugar@101
 field_instance_ethernet__srcAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct mac_learning_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to mac_learning with action _nop\n");// sugar@100
     mac_learning_add(// sugar@101
 field_instance_ethernet__srcAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 routable_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ethernet__srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 uint8_t* field_instance_ethernet__dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@88
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[2])->bitmap);// sugar@88
 if(strcmp("route", ctrl_m->action_name)==0) {// sugar@93
     struct routable_action action;// sugar@94
     action.action_id = action_route;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to routable with action route\n");// sugar@100
     routable_add(// sugar@101
 field_instance_ethernet__srcAddr,// sugar@104
 field_instance_ethernet__dstAddr,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct routable_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to routable with action _nop\n");// sugar@100
     routable_add(// sugar@101
 field_instance_ethernet__srcAddr,// sugar@104
 field_instance_ethernet__dstAddr,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 switching_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ethernet__dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 uint8_t* field_instance_vlan__vid = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@88
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@93
     struct switching_action action;// sugar@94
     action.action_id = action_forward;// sugar@95
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@97
 memcpy(action.forward_params.port, port, 2);// sugar@98
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to switching with action forward\n");// sugar@100
     switching_add(// sugar@101
 field_instance_ethernet__dstAddr,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("broadcast", ctrl_m->action_name)==0) {// sugar@93
     struct switching_action action;// sugar@94
     action.action_id = action_broadcast;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to switching with action broadcast\n");// sugar@100
     switching_add(// sugar@101
 field_instance_ethernet__dstAddr,// sugar@104
 field_instance_vlan__vid,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 routing_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ipv4__dstAddr = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->bitmap);// sugar@90
 uint16_t field_instance_ipv4__dstAddr_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->prefix_length;// sugar@91
 if(strcmp("set_nhop", ctrl_m->action_name)==0) {// sugar@93
     struct routing_action action;// sugar@94
     action.action_id = action_set_nhop;// sugar@95
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@97
 memcpy(action.set_nhop_params.smac, smac, 6);// sugar@98
 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@97
 memcpy(action.set_nhop_params.dmac, dmac, 6);// sugar@98
 uint8_t* vid = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[2])->bitmap;// sugar@97
 memcpy(action.set_nhop_params.vid, vid, 2);// sugar@98
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to routing with action set_nhop\n");// sugar@100
     routing_add(// sugar@101
 field_instance_ipv4__dstAddr,// sugar@104
 field_instance_ipv4__dstAddr_prefix_length,// sugar@106
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@93
     struct routing_action action;// sugar@94
     action.action_id = action__drop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to routing with action _drop\n");// sugar@100
     routing_add(// sugar@101
 field_instance_ipv4__dstAddr,// sugar@104
 field_instance_ipv4__dstAddr_prefix_length,// sugar@106
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 acl_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct acl_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to acl with action _nop\n");// sugar@100
     acl_add(// sugar@101
 field_instance_ipv4__srcAddr,// sugar@104
 field_instance_ipv4__dstAddr,// sugar@104
 field_instance_ipv4__protocol,// sugar@104
 field_instance_l4_metadata__srcPort,// sugar@104
 field_instance_l4_metadata__dstPort,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@93
     struct acl_action action;// sugar@94
     action.action_id = action__drop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to acl with action _drop\n");// sugar@100
     acl_add(// sugar@101
 field_instance_ipv4__srcAddr,// sugar@104
 field_instance_ipv4__dstAddr,// sugar@104
 field_instance_ipv4__protocol,// sugar@104
 field_instance_l4_metadata__srcPort,// sugar@104
 field_instance_l4_metadata__dstPort,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 vlan_egress_proc_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_standard_metadata_egress_spec = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 if(strcmp("strip_vlan", ctrl_m->action_name)==0) {// sugar@93
     struct vlan_egress_proc_action action;// sugar@94
     action.action_id = action_strip_vlan;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to vlan_egress_proc with action strip_vlan\n");// sugar@100
     vlan_egress_proc_add(// sugar@101
 field_instance_standard_metadata_egress_spec,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct vlan_egress_proc_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to vlan_egress_proc with action _nop\n");// sugar@100
     vlan_egress_proc_add(// sugar@101
 field_instance_standard_metadata_egress_spec,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 mcast_src_pruning_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_standard_metadata_instance_type = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct mcast_src_pruning_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to mcast_src_pruning with action _nop\n");// sugar@100
     mcast_src_pruning_add(// sugar@101
 field_instance_standard_metadata_instance_type,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@93
     struct mcast_src_pruning_action action;// sugar@94
     action.action_id = action__drop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to mcast_src_pruning with action _drop\n");// sugar@100
     mcast_src_pruning_add(// sugar@101
 field_instance_standard_metadata_instance_type,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 product_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_universal_key_eth_src = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 uint8_t* field_instance_universal_key_vlan_ingress = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[1])->bitmap);// sugar@88
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@93
     struct product_action action;// sugar@94
     action.action_id = action_forward;// sugar@95
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@97
 memcpy(action.forward_params.port, port, 2);// sugar@98
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to product with action forward\n");// sugar@100
     product_add(// sugar@101
 field_instance_universal_key_eth_src,// sugar@104
 field_instance_universal_key_vlan_ingress,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct product_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to product with action _nop\n");// sugar@100
     product_add(// sugar@101
 field_instance_universal_key_eth_src,// sugar@104
 field_instance_universal_key_vlan_ingress,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@113
 vlan_ingress_proc_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("add_vlan", ctrl_m->action_name)==0) {// sugar@117
     struct vlan_ingress_proc_action action;// sugar@118
     action.action_id = action_add_vlan;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for vlan_ingress_proc with action add_vlan\n");// sugar@124
     vlan_ingress_proc_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct vlan_ingress_proc_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for vlan_ingress_proc with action _nop\n");// sugar@124
     vlan_ingress_proc_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 mac_learning_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@117
     struct mac_learning_action action;// sugar@118
     action.action_id = action_mac_learn;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for mac_learning with action mac_learn\n");// sugar@124
     mac_learning_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct mac_learning_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for mac_learning with action _nop\n");// sugar@124
     mac_learning_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 routable_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("route", ctrl_m->action_name)==0) {// sugar@117
     struct routable_action action;// sugar@118
     action.action_id = action_route;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for routable with action route\n");// sugar@124
     routable_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct routable_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for routable with action _nop\n");// sugar@124
     routable_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 switching_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@117
     struct switching_action action;// sugar@118
     action.action_id = action_forward;// sugar@119
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@121
 memcpy(action.forward_params.port, port, 2);// sugar@122
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for switching with action forward\n");// sugar@124
     switching_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("broadcast", ctrl_m->action_name)==0) {// sugar@117
     struct switching_action action;// sugar@118
     action.action_id = action_broadcast;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for switching with action broadcast\n");// sugar@124
     switching_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 routing_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("set_nhop", ctrl_m->action_name)==0) {// sugar@117
     struct routing_action action;// sugar@118
     action.action_id = action_set_nhop;// sugar@119
 uint8_t* smac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@121
 memcpy(action.set_nhop_params.smac, smac, 6);// sugar@122
 uint8_t* dmac = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;// sugar@121
 memcpy(action.set_nhop_params.dmac, dmac, 6);// sugar@122
 uint8_t* vid = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[2])->bitmap;// sugar@121
 memcpy(action.set_nhop_params.vid, vid, 2);// sugar@122
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for routing with action set_nhop\n");// sugar@124
     routing_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@117
     struct routing_action action;// sugar@118
     action.action_id = action__drop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for routing with action _drop\n");// sugar@124
     routing_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 acl_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct acl_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for acl with action _nop\n");// sugar@124
     acl_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@117
     struct acl_action action;// sugar@118
     action.action_id = action__drop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for acl with action _drop\n");// sugar@124
     acl_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 vlan_egress_proc_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("strip_vlan", ctrl_m->action_name)==0) {// sugar@117
     struct vlan_egress_proc_action action;// sugar@118
     action.action_id = action_strip_vlan;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for vlan_egress_proc with action strip_vlan\n");// sugar@124
     vlan_egress_proc_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct vlan_egress_proc_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for vlan_egress_proc with action _nop\n");// sugar@124
     vlan_egress_proc_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 mcast_src_pruning_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct mcast_src_pruning_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for mcast_src_pruning with action _nop\n");// sugar@124
     mcast_src_pruning_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_drop", ctrl_m->action_name)==0) {// sugar@117
     struct mcast_src_pruning_action action;// sugar@118
     action.action_id = action__drop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for mcast_src_pruning with action _drop\n");// sugar@124
     mcast_src_pruning_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 product_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@117
     struct product_action action;// sugar@118
     action.action_id = action_forward;// sugar@119
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@121
 memcpy(action.forward_params.port, port, 2);// sugar@122
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for product with action forward\n");// sugar@124
     product_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct product_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for product with action _nop\n");// sugar@124
     product_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@131
     debug("MSG from controller %d %s\n", ctrl_m->type, ctrl_m->table_name);// sugar@132
     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@133
 if (strcmp("vlan_ingress_proc", ctrl_m->table_name) == 0)// sugar@135
     vlan_ingress_proc_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("mac_learning", ctrl_m->table_name) == 0)// sugar@135
     mac_learning_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("routable", ctrl_m->table_name) == 0)// sugar@135
     routable_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("switching", ctrl_m->table_name) == 0)// sugar@135
     switching_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("routing", ctrl_m->table_name) == 0)// sugar@135
     routing_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("acl", ctrl_m->table_name) == 0)// sugar@135
     acl_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("vlan_egress_proc", ctrl_m->table_name) == 0)// sugar@135
     vlan_egress_proc_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("mcast_src_pruning", ctrl_m->table_name) == 0)// sugar@135
     mcast_src_pruning_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("product", ctrl_m->table_name) == 0)// sugar@135
     product_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 debug("Table add entry: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@138
     }// sugar@139
     else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@140
 if (strcmp("vlan_ingress_proc", ctrl_m->table_name) == 0)// sugar@142
     vlan_ingress_proc_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("mac_learning", ctrl_m->table_name) == 0)// sugar@142
     mac_learning_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("routable", ctrl_m->table_name) == 0)// sugar@142
     routable_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("switching", ctrl_m->table_name) == 0)// sugar@142
     switching_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("routing", ctrl_m->table_name) == 0)// sugar@142
     routing_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("acl", ctrl_m->table_name) == 0)// sugar@142
     acl_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("vlan_egress_proc", ctrl_m->table_name) == 0)// sugar@142
     vlan_egress_proc_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("mcast_src_pruning", ctrl_m->table_name) == 0)// sugar@142
     mcast_src_pruning_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("product", ctrl_m->table_name) == 0)// sugar@142
     product_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 debug("Table setdefault: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@145
     }// sugar@146
 }// sugar@147
 backend bg;// sugar@151
 void init_control_plane()// sugar@152
 {// sugar@153
     debug("Creating control plane connection...\n");// sugar@154
     bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);// sugar@155
     launch_backend(bg);// sugar@156
 /*// sugar@157
 */// sugar@168
 }// sugar@169
