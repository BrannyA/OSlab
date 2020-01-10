## Lab5 Device Driver

#### task1 Setup TX/RX DMA descriptors  
Send: Send 256 packets and should be captured by Wireshark.
Receive: Continuously check whether there is incoming data. When it received all the packets that pktRxTx sends, print and exit.   

#### task2 Implement MAC driver with blocking   
Send: The same to task1.
Receive: The receive thread is unblocked until it receives 64 packets. The number of packets it received is checked in clock interrupts.   

#### task3 Handling MAC interrupt   
Send: The same to task 1.
Receive: Implemented MAC interrupt. The receive thread is unblocked until it receives 64 packets. The receive thread checks the number of packets during the MAC interrupt. 
