



table product {
    reads {
        universal_key.eth_src 	: exact;
        universal_key.vlan_ingress 	: exact;

    }
    actions {forward; _nop;}
    size : 4000;
}

