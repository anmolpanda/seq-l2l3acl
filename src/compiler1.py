import collections

from p4_hlir.main import HLIR
from p4_hlir.hlir import p4_parser
from p4_hlir.hlir import p4_tables

import re
import os
import sys
from os.path import isfile, join

def main():
    p4_path = "/root/workspace/ankit/vanilla_p4c/examples/l2_switch_test.p4" 
    if os.path.isfile(p4_path) is False:
        print("FILE NOT FOUND: %s" % p4_path)
        sys.exit(1)

    h = HLIR(p4_path)
    h.build()
    print "\n\n"
    for table_names, table in h.p4_tables.items():
	print table_names, "\t", table.actions
    print "\n\n"


    for actions, action in h.p4_actions.items():
	print actions, "\t", action.flat_call_sequence
	print action.signature
    print "\n\n"

    for actions_s, action_s in h.p4_action_selectors.items():
	print actions, "\t", action
    print "\n\n"

    for headers, header in h.p4_headers.items():
	print headers, "\t", header.layout, "\t", header.attributes
    print "\n\n"

    for pss, ps in h.p4_parse_states.items():
	print pss, "\t", ps.call_sequence
main()

