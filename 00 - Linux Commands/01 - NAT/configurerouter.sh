#!/bin/bash

# Define interfaces
INET_IF="eth0"
LAN_IF="eth1"
LAN_IP="10.1.1.1/24"

# Configure the LAN interface
nmcli con mod "$LAN_IF" ipv4.addresses $LAN_IP ipv4.method manual
nmcli con up "$LAN_IF"

# Enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward
sysctl -w net.ipv4.ip_forward=1

# Set up NAT with iptables
iptables -t nat -A POSTROUTING -s 10.0.0.0/8 -o $INET_IF -j MASQUERADE
iptables -t nat -A POSTROUTING -s 172.16.0.0/12 -o $INET_IF -j MASQUERADE
iptables -t nat -A POSTROUTING -s 192.168.0.0/16 -o $INET_IF -j MASQUERADE

# Save iptables rules
iptables-save > /etc/sysconfig/iptables

# Ensure iptables rules are applied on reboot
cat <<EOT > /etc/sysconfig/iptables-config
IPTABLES_SAVE_ON_STOP="yes"
IPTABLES_SAVE_ON_RESTART="yes"
IPTABLES_SAVE_COUNTER="no"
EOT

# Restart network services to apply changes
systemctl restart NetworkManager
