#!/bin/bash

# Define interface
IFACE=ens3

# Remove any existing qdisc on the interface
tc qdisc del dev $IFACE root

# Add root qdisc with rate limiting to 500Mb/s
tc qdisc add dev $IFACE root handle 1: htb default 30
tc class add dev $IFACE parent 1: classid 1:1 htb rate 500Mbit

# Add child qdisc for expedited forwarding (DSCP EF) traffic
tc class add dev $IFACE parent 1:1 classid 1:10 htb rate 128Kbit ceil 128Kbit
tc qdisc add dev $IFACE parent 1:10 handle 10: sfq
tc filter add dev $IFACE protocol ip parent 1:0 prio 1 u32 match ip dscp 46 0xfc flowid 1:10

# Add child qdisc for prioritization (DSCP pri 4) traffic
tc class add dev $IFACE parent 1:1 classid 1:20 htb rate 1Mbit ceil 5Mbit
tc qdisc add dev $IFACE parent 1:20 handle 20: sfq
tc filter add dev $IFACE protocol ip parent 1:0 prio 2 u32 match ip dscp 32 0xfc flowid 1:20

# Add child qdisc for all other traffic with traffic shaping to 450Mb/s
tc class add dev $IFACE parent 1:1 classid 1:30 htb rate 450Mbit
tc qdisc add dev $IFACE parent 1:30 handle 30: sfq

echo "QoS configuration applied successfully!"
