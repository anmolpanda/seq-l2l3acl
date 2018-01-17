 #ifndef __DATA_PLANE_DATA_H__// sugar@14
 #define __DATA_PLANE_DATA_H__// sugar@15

 #include "parser.h"// sugar@17

 #define NB_TABLES 9// sugar@19

 enum table_names {// sugar@21
 TABLE_vlan_ingress_proc,// sugar@23
 TABLE_mac_learning,// sugar@23
 TABLE_routable,// sugar@23
 TABLE_switching,// sugar@23
 TABLE_routing,// sugar@23
 TABLE_acl,// sugar@23
 TABLE_vlan_egress_proc,// sugar@23
 TABLE_mcast_src_pruning,// sugar@23
 TABLE_product,// sugar@23
 TABLE_// sugar@24
 };// sugar@25

 #define NB_COUNTERS 0// sugar@27

 enum counter_names {// sugar@29
 COUNTER_// sugar@32
 };// sugar@33
 
 #endif // __DATA_PLANE_DATA_H__// sugar@35
