 #ifndef __ACTION_INFO_GENERATED_H__// sugar@14
 #define __ACTION_INFO_GENERATED_H__// sugar@15
 

 #define FIELD(name, length) uint8_t name[(length + 7) / 8];// sugar@18

 enum actions {// sugar@21
 action_add_vlan,// sugar@27
 action__nop,// sugar@27
 action_mac_learn,// sugar@27
 action_route,// sugar@27
 action_forward,// sugar@27
 action_broadcast,// sugar@27
 action_set_nhop,// sugar@27
 action__drop,// sugar@27
 action_strip_vlan,// sugar@27
 };// sugar@28
 struct action_forward_params {// sugar@33
    FIELD(port, 9);// sugar@35
    int is_Terminal;// sugar@36
 };// sugar@37
 struct action_set_nhop_params {// sugar@33
    FIELD(smac, 48);// sugar@35
    FIELD(dmac, 48);// sugar@35
    FIELD(vid, 12);// sugar@35
    int is_Terminal;// sugar@36
 };// sugar@37
// struct action_forward_params {// sugar@33
//    FIELD(port, 9);// sugar@35
//    int is_Terminal;// sugar@36
// };// sugar@37
 struct vlan_ingress_proc_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct mac_learning_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct routable_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct switching_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
 struct action_forward_params forward_params;// sugar@45
     };// sugar@46
 };// sugar@47
 struct routing_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
 struct action_set_nhop_params set_nhop_params;// sugar@45
     };// sugar@46
 };// sugar@47
 struct acl_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct vlan_egress_proc_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct mcast_src_pruning_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
     };// sugar@46
 };// sugar@47
 struct product_action {// sugar@40
     int action_id;// sugar@41
     union {// sugar@42
 struct action_forward_params forward_params;// sugar@45
     };// sugar@46
 };// sugar@47
 void apply_table_vlan_ingress_proc(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_add_vlan(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_mac_learning(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_mac_learn(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_routable(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_route(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_switching(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_forward(packet_descriptor_t *pd, lookup_table_t **tables, struct action_forward_params);// sugar@53
 void action_code_broadcast(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_routing(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_set_nhop(packet_descriptor_t *pd, lookup_table_t **tables, struct action_set_nhop_params);// sugar@53
 void action_code__drop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_acl(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__drop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_vlan_egress_proc(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_strip_vlan(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_mcast_src_pruning(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void action_code__drop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 void apply_table_product(packet_descriptor_t *pd, int batch_size, lookup_table_t** tables);// sugar@50
 void action_code_forward(packet_descriptor_t *pd, lookup_table_t **tables, struct action_forward_params);// sugar@53
 void action_code__nop(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@55
 #endif// sugar@57
