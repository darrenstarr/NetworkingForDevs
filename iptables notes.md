# Notes

## IPTables



List

iptables -l

List with line numbers

iptables -l --line-numbers

Append

iptables -A

Insert

iptables -I

Policy

iptables -P

Policy can be set of input, output and foward
Sets the default policy for the chain as accept or drop
Default policy is always at the end of the chain. 

chain => 
    INPUT |     // Filter packets on ingress
    OUTPUT |    // Filter packets on egress
    FORWARD |   // filtering packets when the device is acting as a router
    PREROUTING |  // Used for destination NAT or port forwarding
    POSTROUTING | // Used for source NAT (dynamic port address translation/masquerading)

action => 
    DROP |      // Drop packets without informing source 
    ACCEPT |    // Accept or pass packets
    REJECT      // Drop packets and inform source (icmp)

protocol =>
    tcp |
    udp |
    icmp

table =>
    filter |    // Default table for iptables (INPUT|OUTPUT|FORWARD)
    nat |       // Network address translation for SNAT and DNAT I.E. masquerading (PREROUTING|POSTROUTING|OUTPUT)
    mangle |    // Packet magling or mutation (PREROUTING|INPUT|FORWARD|OUTPUT|POSTROUTING)
    raw         // Marking (PREROUTING|OUTPUT), useful for NOTRACK

tcpflags =>
    SYN |       // Synchronize
    ACK |       // Acknowledgement
    FIN |       // Finalize
    RST |       // Rest
    URG |       // Urgent
    PSH |       // Push
    ALL |       // All flags
    NONE        // No flags

iptables -[IA] <chain> [-j <action>] [-s <source>] [-d <destination>] [-p <protocol>] [-dport <destinationport>] [-sport <sourceport>] [-t <table>] [-m iprange --src-range <start ip>-<end ip>] [--dport <port>] [-m multiport -dports <port>,<port>...] [-i <input interface>] [-o <output interface>] [--syn] [--tcp-flags ] [--state <state>,<state>...] [-m mac --mac-source <mac address>]

iptables -D <chain> <linenumber>


Use ! to negate comparisons prior to the matching option

default policy is applied at the end of the list.

iptables -vnL [-t <table>]
   -n (don't reverse DNS)
   -v (verbose, shows interface namaes, packet and byte counters)
   -L list

iptable -t <table> -Z    // Zeroize counters on filter

iptables -F // Flush all chains

iptables-save
iptables-restore

# Full flush script 
iptables -P INPUT ACCEPT
iptables -P OUTPUT ACCEPT
iptables -P FORWARD ACCEPT  

iptables -t filter -F
iptables -t nat -F
iptables -t mangle -F
iptables -t raw -F

iptables -X    // Delete user defined chains

iptables-persistent Ubuntu
/sbin/service iptables save // writes to /etc/sysconfig/iptables

iptables -m addrtype --help

Controlling domain access should use a tool like squid instead of iptables

Port scan destination IP
nmap <ip address>

Connection tracking
state =>
  NEW |             // First packet from a connection
  ESTABLISHED |     // Following packets 
  RELATED |         // Connections associated to another connection (such as FTP, SIP)
  INVALID |         // Packets not associated with a known connection
  UNTRACKED         // Packets marked as NOTRACK in the RAW table