#!/bin/bash

# Set variables
INTERFACE="ens3"
IP_ADDRESS="10.1.2.194/24"
GATEWAY="10.1.2.193"

# Configure the network interface
nmcli con add type ethernet ifname $INTERFACE con-name $INTERFACE ip4 $IP_ADDRESS gw4 $GATEWAY

# Restart NetworkManager service to apply changes
systemctl restart NetworkManager

# Verify the configuration
nmcli con show $INTERFACE
nmcli dev show $INTERFACE

echo "Network interface $INTERFACE configured with IP $IP_ADDRESS and gateway $GATEWAY"
