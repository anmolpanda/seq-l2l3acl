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
from utils.hlir import *  

generated_code += " #include <stdlib.h>// sugar@17\n"
generated_code += " #include <string.h>// sugar@18\n"
generated_code += " #include \"dpdk_lib.h\"// sugar@19\n"
generated_code += " #include \"actions.h\"// sugar@20\n"
generated_code += " #include <arpa/inet.h>// sugar@21\n"
generated_code += " \n"
generated_code += " extern void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@23\n"
generated_code += "\n"
generated_code += " extern void increase_counter (int counterid, int index);// sugar@25\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@28\n"
generated_code += "\n"

generated_code += " uint8_t reverse_buffer[" + str(max([t[1] for t in map(getTypeAndLength, hlir.p4_tables.values())])) + "];// sugar@31\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    if table.name == "ndn_fib_lpm":
        generated_code += "void terminal_lookup(lookup_table_t* t, int  batch_size, uint8_t key[][32], uint8_t** values, uint16_t* length) {// sugar@35\n"
	generated_code += "    int terminal[batch_size];// sugar@36\n"
	generated_code += "    for(int i = 0; i < batch_size; i++) {// sugar@37\n"
	generated_code += "       struct ndn_fib_lpm_action* res = (struct ndn_fib_lpm_action*)values[i];// sugar@38\n"
	generated_code += "        if(res != NULL && res->fib_hit_nexthop_params.is_Terminal == 1) {// sugar@39\n"
	generated_code += "            continue;// sugar@40\n"
	generated_code += "        }// sugar@41\n"
	generated_code += "\n"
	generated_code += "        int c_i = length[i]-2;  // want to skip last component, as already checked in exact_lookups()// sugar@43\n"
	generated_code += "        terminal[i] = 0;// sugar@44\n"
	generated_code += "        for(; c_i >= 0; c_i--)// sugar@45\n"
	generated_code += "        {// sugar@46\n"
	generated_code += "            if(unlikely(key[i][c_i] == '/')) {// sugar@47\n"
	generated_code += "                char url[32] = {0};// sugar@48\n"
	generated_code += "                memcpy(url, key[i], c_i+1);// sugar@49\n"
	generated_code += "                values[i] = one_exact_lookup(t, (uint8_t *)url);// sugar@50\n"
	generated_code += "                res = (struct ndn_fib_lpm_action*)values[i];// sugar@51\n"
	generated_code += "                if(res != NULL && res->fib_hit_nexthop_params.is_Terminal == 1) {// sugar@52\n"
	generated_code += "                    terminal[i] = 1;// sugar@53\n"
	generated_code += "                    break;// sugar@54\n"
	generated_code += "                }// sugar@55\n"
	generated_code += "            }// sugar@56\n"
	generated_code += "        }// sugar@57\n"
	generated_code += "    }// sugar@58\n"
	generated_code += "}// sugar@59\n"
generated_code += "\n"
def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " void table_" + str(table.name) + "_key(packet_descriptor_t* pd, int batch_size, uint8_t key[][" + str(key_length) + "], uint16_t* len) {// sugar@68\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    for match_field, match_type, match_mask in sortedfields:
        if match_field.width <= 32:
	    generated_code += " for(int i = 0; i< batch_size; i++) {// sugar@72\n"
            generated_code += "     EXTRACT_INT32_BITS(&pd[i], " + str(fld_id(match_field)) + ", *(uint32_t*)key[i])// sugar@73\n"
            ##[     key += sizeof(uint32_t);
        elif match_field.width > 32 and match_field.width % 8 == 0:
            byte_width = (match_field.width+7)/8
            ##[//EXTRACT_BYTEBUF(pd, ${fld_id(match_field)}, key)
            ##[ //key += ${byte_width};
	    generated_code += "    int length = 0;// sugar@79\n"
	    generated_code += "    for(int i = 0; i < batch_size; i++) {// sugar@80\n"
	    generated_code += "        uint8_t* address = FIELD_BYTE_ADDR(&pd[i], field_desc(" + str(fld_id(match_field)) + "));	// sugar@81\n"
	    generated_code += "        memcpy(key[i], address, " + str(key_length) + ");// sugar@82\n"
	    if fld_id(match_field) == "field_instance_ndn_url":
		generated_code += "        length = *(uint16_t *)(address + 32);// sugar@84\n"
		generated_code += "        length = (length < 32) ? length : 31;// sugar@85\n"
	        generated_code += "        //Truncate the string at a terminal character \"/\"// sugar@86\n"
		generated_code += "        for(int j = length-1; j >= 0; j--) {// sugar@87\n"
            	generated_code += "            if(unlikely(key[i][j] == '/'))// sugar@88\n"
            	generated_code += "                break;// sugar@89\n"
            	generated_code += "            else// sugar@90\n"
            	generated_code += "                key[i][j] = '\\0';// sugar@91\n"
            	generated_code += "        }// sugar@92\n"
		generated_code += "        len[i] = length;// sugar@93\n"
        else:
            print "Unsupported field %s ignored in key calculation." % fld_id(match_field)
    if table_type == "LOOKUP_LPM":
	if key_length == 4:
	    generated_code += "    uint32_t ip = ntohl(*(uint32_t *)key[i]);// sugar@98\n"
	    generated_code += "    memcpy(key[i], &ip, 4);// sugar@99\n"
	else:
            generated_code += " //key -= " + str(key_length) + ";// sugar@101\n"
            generated_code += "     int c, d;// sugar@102\n"
            generated_code += "     for(c = " + str(key_length-1) + ", d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key[i]+c);// sugar@103\n"
            generated_code += "     for(c = 0; c < " + str(key_length) + "; c++) *(key[i]+c) = *(reverse_buffer+c);// sugar@104\n"
    generated_code += " }// sugar@105\n"
    generated_code += " }// sugar@106\n"
    generated_code += "\n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    lookupfun = {'LOOKUP_LPM':'lpm_lookup', 'LOOKUP_EXACT':'exact_lookup', 'LOOKUP_TERNARY':'ternary_lookup'}
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@112\n"
    generated_code += " {// sugar@113\n"
    generated_code += "     uint8_t key[batch_size][" + str(key_length) + "];// sugar@114\n"
    generated_code += "     uint8_t* values[batch_size];// sugar@115\n"
    generated_code += "     uint16_t length[batch_size];// sugar@116\n"
    generated_code += "     //debug(\"  :::: EXECUTING TABLE " + str(table.name) + "\\n\");// sugar@117\n"
    generated_code += "     table_" + str(table.name) + "_key(pd, batch_size, key, length);// sugar@118\n"
    generated_code += "     " + str(lookupfun[table_type]) + "(tables[TABLE_" + str(table.name) + "], batch_size, " + str(key_length) + ", key, values);// sugar@119\n"
    if table.name == "ndn_fib_lpm":
	generated_code += "     terminal_lookup(tables[TABLE_" + str(table.name) + "], batch_size, key, values, length);// sugar@121\n"
    generated_code += "     //int index = *(int*)(values[i] +sizeof(struct " + str(table.name) + "_action)); (void)index;// sugar@122\n"
    generated_code += "     for(int i = 0; i < batch_size; i++) {// sugar@123\n"
    generated_code += "         struct " + str(table.name) + "_action* res = (struct " + str(table.name) + "_action*)values[i];// sugar@124\n"
    for counter in table.attached_counters:
        generated_code += "         increase_counter(COUNTER_" + str(counter.name) + ", index);// sugar@126\n"
    generated_code += "         if(res == NULL) {// sugar@127\n"
    generated_code += "             debug(\"    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\\n\");// sugar@128\n"
    generated_code += "             continue;// sugar@129\n"
    generated_code += "         }// sugar@130\n"
    generated_code += "         switch (res->action_id) {// sugar@131\n"
    for action in table.actions:
        generated_code += "         case action_" + str(action.name) + ":// sugar@133\n"
        generated_code += "             //debug(\"    :: EXECUTING ACTION " + str(action.name) + "...\\n\");// sugar@134\n"
        if action.signature:
            generated_code += "             action_code_" + str(action.name) + "(&pd[i], tables, res->" + str(action.name) + "_params);// sugar@136\n"
        else:
            generated_code += "             action_code_" + str(action.name) + "(&pd[i], tables);// sugar@138\n"
        generated_code += "         break;// sugar@139\n"
    generated_code += "         }// sugar@140\n"
    generated_code += "     }// sugar@141\n"
    if 'hit' in table.next_:
        if table.next_['hit'] is not None:
            generated_code += " if(res != NULL)// sugar@144\n"
            generated_code += "     apply_table_" + str(table.next_['hit'].name) + "(pd, tables);// sugar@145\n"
        if table.next_['miss'] is not None:
            generated_code += " if(res == NULL)// sugar@147\n"
            generated_code += "     apply_table_" + str(table.next_['miss'].name) + "(pd, tables);// sugar@148\n"
    else:
	action, nextnode = table.next_.items()[0]
	generated_code += "    " + str(format_p4_node(nextnode)) + "// sugar@151\n"
        ##[     switch (res->action_id) {
        #for action, nextnode in table.next_.items():
            ##[     case action_${action.name}:
            ##[     ${format_p4_node(nextnode)}
            ##[     break;
        ##[     }
    generated_code += " }// sugar@158\n"
    generated_code += "\n"

generated_code += " void init_headers(packet_descriptor_t* packet_desc) {// sugar@161\n"
for hi in header_instances(hlir):
    n = hdr_prefix(hi.name)
    if hi.metadata:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_instance_byte_width[" + str(n) + "],// sugar@165\n"
        generated_code += "                               .pointer = calloc(header_instance_byte_width[" + str(n) + "], sizeof(uint8_t)) };// sugar@166\n"
    else:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_instance_byte_width[" + str(n) + "], .pointer = NULL };// sugar@168\n"
generated_code += " }// sugar@169\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " extern void " + str(table.name) + "_setdefault(struct " + str(table.name) + "_action);// sugar@175\n"
generated_code += "\n"
generated_code += " void init_keyless_tables() {// sugar@177\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " struct " + str(table.name) + "_action " + str(table.name) + "_a;// sugar@182\n"
        generated_code += " " + str(table.name) + "_a.action_id = action_" + str(action.name) + ";// sugar@183\n"
        generated_code += " " + str(table.name) + "_setdefault(" + str(table.name) + "_a);// sugar@184\n"
generated_code += " }// sugar@185\n"
generated_code += "\n"
generated_code += " void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@187\n"
generated_code += "     init_headers(pd);// sugar@188\n"
generated_code += "     init_keyless_tables();// sugar@189\n"
generated_code += " }// sugar@190\n"

generated_code += " \n"
generated_code += " void handle_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@193\n"
generated_code += " {// sugar@194\n"
generated_code += "     //int value32;// sugar@195\n"
generated_code += "     //EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@196\n"
generated_code += "     //debug(\"### HANDLING PACKET ARRIVING AT PORT %\" PRIu32 \"...\\n\", value32);// sugar@197\n"
generated_code += "     parse_packets(pd, batch_size, tables);// sugar@198\n"
generated_code += " }// sugar@199\n"