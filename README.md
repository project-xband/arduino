optionally headless mesh node network
==========================

 <0x01> alpha
 <0x02> beta
 <0x03> charlie
 <0x04> delta

 Loop:
==========================
listen on serial:
	take in message
try sending packet:
	done
try listening for packet:
	print to serial

  Steps:
==========================
[ ] Make listening fast
[ ] Push onto message queue
