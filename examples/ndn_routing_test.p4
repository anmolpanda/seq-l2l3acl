header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type ndn_t {
    fields {
	ip_src  :  32;
        ip_dst  :  32;
        url     :  256;
        url_len :  16;
        cksum   :  16;
        totalLen:  16;
    }
}

parser start {
    return parse_ethernet;
}

#define ETHERTYPE_IPV4 0x0800

header ethernet_t ethernet;

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ndn;
        default: ingress;
    }
}

header ndn_t ndn;

parser parse_ndn {
    extract(ndn);
    return ingress;
}

action on_miss() {
}

action fib_hit_nexthop(port) {
    modify_field(standard_metadata.egress_port, port);
}


table ndn_fib_lpm {
    reads {
        ndn.url : exact;
    }
    actions {
        on_miss;
        fib_hit_nexthop;
    }
    size : 512;
}

control ingress {
        apply(ndn_fib_lpm);
}

control egress {
}
