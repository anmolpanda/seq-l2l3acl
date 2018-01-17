 #include "dataplane.h"// sugar@17
 #include "actions.h"// sugar@18
 #include "data_plane_data.h"// sugar@19

 lookup_table_t table_config[NB_TABLES] = {// sugar@21
 {// sugar@24
  .name= "vlan_ingress_proc",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 3,// sugar@27
  .val_size = sizeof(struct vlan_ingress_proc_action),// sugar@28
  .min_size = 0, //64,// sugar@29
  .max_size = 65535 //64// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "mac_learning",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 6,// sugar@27
  .val_size = sizeof(struct mac_learning_action),// sugar@28
  .min_size = 0, //4000,// sugar@29
  .max_size = 65535 //4000// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "routable",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  //.key_size = 14,// sugar@27
  .key_size = 31,
  .val_size = sizeof(struct routable_action),// sugar@28
  .min_size = 0, //64,// sugar@29
  .max_size = 65535 //64// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "switching",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 8,// sugar@27
  .val_size = sizeof(struct switching_action),// sugar@28
  .min_size = 0, //4000,// sugar@29
  .max_size = 65535 //4000// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "routing",// sugar@25
  .type = LOOKUP_LPM,// sugar@26
  .key_size = 4,// sugar@27
  .val_size = sizeof(struct routing_action),// sugar@28
  .min_size = 0, //2000,// sugar@29
  .max_size = 65535 //2000// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "acl",// sugar@25
  .type = LOOKUP_TERNARY,// sugar@26
  //.type = LOOKUP_EXACT,// sugar@26 
  .key_size = 13,// sugar@27
  .val_size = sizeof(struct acl_action),// sugar@28
  .min_size = 0, //1000,// sugar@29
  .max_size = 65535 //1000// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "vlan_egress_proc",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 2,// sugar@27
  .val_size = sizeof(struct vlan_egress_proc_action),// sugar@28
  .min_size = 0, //64,// sugar@29
  .max_size = 65535 //64// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "mcast_src_pruning",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 4,// sugar@27
  .val_size = sizeof(struct mcast_src_pruning_action),// sugar@28
  .min_size = 0, //1,// sugar@29
  .max_size = 255 //1// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "product",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 7,// sugar@27
  .val_size = sizeof(struct product_action),// sugar@28
  .min_size = 0, //4000,// sugar@29
  .max_size = 255 //4000// sugar@30
 },// sugar@31
 };// sugar@32
 counter_t counter_config[NB_COUNTERS] = {// sugar@34
 };// sugar@49
