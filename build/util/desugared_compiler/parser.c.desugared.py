# Copyright 2016 Eotvos Lorand University, Budapest, Hungary
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import p4_hlir.hlir.p4 as p4
from utils.hlir import getTypeAndLength, hdr_prefix, fld_prefix, fld_id, format_p4_node 
from utils.misc import addError, addWarning 

def format_state(state):
    generated_code = ""
    if isinstance(state, p4.p4_parse_state):
        generated_code += "    return parse_state_" + str(state.name) + "(pd, batch_size, tables);// sugar@21\n"
    elif isinstance(state, p4.p4_parser_exception):
        print "Parser exceptions are not supported yet."
    else:
        return format_p4_node(state)
    return generated_code

def int_to_byte_array(val): # CAUTION: big endian!
    """
    :param val: int
    :rtype:     (int, [int])
    """
    nbytes = 0
    res = []
    while val > 0:
        nbytes += 1
        res.append(int(val % 256))
        val /= 256
    res.reverse()
    return nbytes, res

def get_key_byte_width(branch_on):
    """
    :param branch_on: list of union(p4_field, tuple)
    :rtype:           int
    """
    key_width = 0
    for switch_ref in branch_on:
        if type(switch_ref) is p4.p4_field:
            key_width += (switch_ref.width+7)/8
        elif type(switch_ref) is tuple:
            key_width += max(4, (switch_ref[1] + 7) / 8)
    return key_width

pe_dict = { "p4_pe_index_out_of_bounds" : None,
            "p4_pe_out_of_packet" : None,
            "p4_pe_header_too_long" : None,
            "p4_pe_header_too_short" : None,
            "p4_pe_unhandled_select" : None,
            "p4_pe_checksum" : None,
            "p4_pe_default" : None }

pe_default = p4.p4_parser_exception(None, None)
pe_default.name = "p4_pe_default"
pe_default.return_or_drop = p4.P4_PARSER_DROP

for pe_name, pe in pe_dict.items():
    pe_dict[pe_name] = pe_default
for pe_name, pe in hlir.p4_parser_exceptions.items():
    pe_dict[pe_name] = pe

generated_code += " #include \"dpdk_lib.h\"// sugar@72\n"
generated_code += " #include \"actions.h\" // apply_table_* and action_code_*// sugar@73\n"
generated_code += "\n"
generated_code += " void print_mac(uint8_t* v) { printf(\"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\\n\", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@75\n"
generated_code += " void print_ip(uint8_t* v) { printf(\"%d.%d.%d.%d\\n\",v[0],v[1],v[2],v[3]); }// sugar@76\n"
generated_code += " \n"
generated_code += " static void// sugar@78\n"
generated_code += " extract_header(uint8_t* buf, packet_descriptor_t* pd, header_instance_t h) {// sugar@79\n"
generated_code += "     pd->headers[h] =// sugar@80\n"
generated_code += "       (header_descriptor_t) {// sugar@81\n"
generated_code += "         .type = h,// sugar@82\n"
generated_code += "         .pointer = buf,// sugar@83\n"
generated_code += "         .length = header_instance_byte_width[h]// sugar@84\n"
generated_code += "       };// sugar@85\n"
generated_code += " }// sugar@86\n"
generated_code += " \n"

for pe_name, pe in pe_dict.items():
    generated_code += " static inline void " + str(pe_name) + "(packet_descriptor_t *pd) {// sugar@90\n"
    if pe.return_or_drop == p4.P4_PARSER_DROP:
        generated_code += " drop(pd);// sugar@92\n"
    else:
        format_p4_node(pe.return_or_drop)
    generated_code += " }// sugar@95\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@98\n"
generated_code += "\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    branch_on = parse_state.branch_on
    if branch_on:
        generated_code += " static inline void build_key_" + str(state_name) + "(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@104\n"
        for switch_ref in branch_on:
            if type(switch_ref) is p4.p4_field:
                field_instance = switch_ref
                byte_width = (field_instance.width + 7) / 8
                if byte_width <= 4:
                    generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(field_instance)) + ", *(uint32_t*)key)// sugar@110\n"
                    generated_code += " key += sizeof(uint32_t);// sugar@111\n"
                else:
                    generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(field_instance)) + ", key)// sugar@113\n"
                    generated_code += " key += " + str(byte_width) + ";// sugar@114\n"
            elif type(switch_ref) is tuple:
                generated_code += "     uint8_t* ptr;// sugar@116\n"
                offset, width = switch_ref
                # TODO
                addError("generating parse state %s"%state_name, "current() calls are not supported yet")
        generated_code += " }// sugar@120\n"

call_number = 0
for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124\n"
    generated_code += " {// sugar@125\n"
    for call in parse_state.call_sequence:
        if call[0] == p4.parse_call.extract:
            header_instance_name = hdr_prefix(call[1].name)
	    generated_code += "     int total_keys = 0;// sugar@129\n"
	    generated_code += "     for(int i = 0; i < batch_size; i++) {// sugar@130\n"
            generated_code += "         uint8_t* buf = (uint8_t*) pd[i].data;// sugar@131\n"
	    if call_number > 0 :
		for state_name1, parse_state1 in hlir.p4_parse_states.items():
	            for call1 in parse_state1.call_sequence:
                        if call1[0] == p4.parse_call.extract:
                            header_instance_name1 = hdr_prefix(call1[1].name)
		            if header_instance_name != header_instance_name1:
	    			generated_code += "         buf += header_instance_byte_width[" + str(header_instance_name1) + "];// sugar@138\n"
            generated_code += "         extract_header(buf, &pd[i], " + str(header_instance_name) + ");// sugar@139\n"
            generated_code += "         buf += header_instance_byte_width[" + str(header_instance_name) + "];// sugar@140\n"
        elif call[0] == p4.parse_call.set:
            dest_field, src = call[1], call[2]
            if type(src) is int or type(src) is long:
                hex(src)
                # TODO
            elif type(src) is p4.p4_field:
                src
                # TODO
            elif type(src) is tuple:
                offset, width = src
                # TODO
            addError("generating parse state %s"%state_name, "set_metadata during parsing is not supported yet")
        call_number = call_number + 1
    branch_on = parse_state.branch_on
    if not branch_on:
        branch_case, next_state = parse_state.branch_to.items()[0]
	if call_number > 0:
	    generated_code += "    }// sugar@158\n"
        generated_code += "    " + str(format_state(next_state)) + "// sugar@159\n"
    else:
        key_byte_width = get_key_byte_width(branch_on)
        generated_code += "         uint8_t key[" + str(key_byte_width) + "];// sugar@162\n"
        generated_code += "         build_key_" + str(state_name) + "(pd, buf, key);// sugar@163\n"
        has_default_case = False
        for case_num, case in enumerate(parse_state.branch_to.items()):
            branch_case, next_state = case
            mask_name  = "mask_value_%d" % case_num
            value_name  = "case_value_%d" % case_num
            if branch_case == p4.P4_DEFAULT:
                has_default_case = True
		generated_code += "     }// sugar@171\n"
                generated_code += "     " + str(format_state(next_state)) + "// sugar@172\n"
                continue
            if type(branch_case) is int:
                value = branch_case
                value_len, l = int_to_byte_array(value)
                generated_code += "         uint8_t " + str(value_name) + "[" + str(value_len) + "] = {// sugar@177\n"
                for c in l:
                    generated_code += "         " + str(c) + ",// sugar@179\n"
                generated_code += "         };// sugar@180\n"
                generated_code += "         if ( memcmp(key, " + str(value_name) + ", " + str(value_len) + ") == 0) {// sugar@181\n"
		generated_code += "             total_keys++;// sugar@182\n"
		generated_code += "             if(unlikely(total_keys == batch_size))// sugar@183\n"
                generated_code += "                 " + str(format_state(next_state)) + "         }// sugar@184\n"
            elif type(branch_case) is tuple:
                value = branch_case[0]
                mask = branch_case[1]
                # TODO
                addError("generating parse state %s"%state_name, "value masking is not supported yet")
            elif type(branch_case) is p4.p4_parse_value_set:
                value_set = branch_case
                # TODO
                addError("generating parse state %s"%state_name, "value sets are not supported yet")
                continue
        if not has_default_case:
            generated_code += "     return NULL;// sugar@196\n"
    generated_code += " }// sugar@197\n"
    generated_code += " \n"

generated_code += " void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables) {// sugar@200\n"
generated_code += "     parse_state_start(pd, batch_size, tables);// sugar@201\n"
generated_code += " }// sugar@202\n"
