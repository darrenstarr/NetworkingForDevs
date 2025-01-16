write a rhel bash script which implement quality of service using the tc command. 

It shoud have a root qdisc rate limiting to 500Mb/s on interface ens3. 

It should implement children qdiscs as follows. 

One will expidite traffic marked as DSCP EF and rate limit to 128Kb/s per conversation. 

Another should implement prioritization for DSCP marking DSCP pri 4 and rate attempt to guarantee up to 1Mb/s bandwidth per conversation but rate limit to 5Mb/s per conversation.

Another should implement traffic shaping to 450Mb/s for all other traffic.