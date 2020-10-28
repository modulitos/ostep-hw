#!/usr/bin/env python3

with open("./trace.txt") as trace_file, open("./vpns.txt", "w") as vpn_file:
    for line in trace_file:
        if not line.startswith("="):
            vpn_file.write(str((int("0x" + line[3:11], 16) & 0xFFFFF000) >> 12) + "\n")
