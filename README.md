NetworkFTP
==========

•	Command line clients request files from a server on the network and the server breaks the file into many packets, sending them to the client using TCP. 
•	Packet drops are handled via the Sliding Window Protocol.

Executable for rcopy: /rcopy/build/debug
Executable for server: /server/build/debug

Instructions: 
•	Starting the server will give a port number that the rcopy must enter when executing.
•	rcopy requests a file from the server and waits for download.