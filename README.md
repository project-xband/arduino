optionally headless mesh node network
==========================

 Loop:
==========================
listen on serial:
	take in message
try sending packet:
	done
try listening for packet:
	print to serial

Files:
===========================
RadioHead library with update frequency and max_packet_len
Arduino code for mesh node, broadcasts to every other xband radio
