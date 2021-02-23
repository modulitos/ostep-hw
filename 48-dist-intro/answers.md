# Distributed Systems

## Homework (Code)

In this section, we’ll write some simple communication code to get you familiar with the task of doing so. Have fun!

## Questions

### 1. Using the code provided in the chapter, build a simple UDP-based server and client. The server should receive messages from the client, and reply with an acknowledgment. In this first attempt, do not add any re-transmission or robustness (assume that communication works perfectly). Run this on a single machine for testing; later, run it on two different machines.

    https://man7.org/linux/man-pages/man2/socket.2.html
    https://man7.org/linux/man-pages/man2/bind.2.html

    Can also use `man`, but it's a bit out of date:

    ```
    $ man 3 getaddrinfo
    $ man 7 socket
    ```


### 2. Turn your code into a **communication library**. Specifically, make your own API, with send and receive calls, as well as other API calls as needed. Rewrite your client and server to use your library instead of raw socket calls.

### 3. Add reliable communication to your burgeoning communication library,in the form of **timeout/retry**. Specifically, your library should make a copy of any message that it is going to send. When sending it, it should start a timer, so it can track how long it has been since the message was sent. On the receiver, the library should **acknowledge** received messages. The client send should **block** when sending, i.e., it should wait until the message has been acknowledged before returning. It should also be willing to retry sending indefinitely. The maximum message size should be that of the largest single message you can send with UDP. Finally, be sure to perform timeout/retry efficiently by putting the caller to sleep until either an ack arrives or the transmission times out; do not spin and waste the CPU!

    Max UDP payload size: see the [answer](https://github.com/xxyzz/cnata/blob/master/3/wireshark_lab.md#wireshark-lab-udp) of question four.


### 4. Make your library more efficient and feature-filled. First, add very-large message transfer. Specifically, although the network limit max-imum message size, your library should take a message of arbitrar-ily large size and transfer it from client to server. The client shouldtransmit these large messages in pieces to the server; the server-sidelibrary code should assemble received fragments into the contigu-ous whole, and pass the single large buffer to the waiting servercode.

### 5. Do the above again, but with high performance. Instead of sendingeach fragment one at a time, you should rapidly send many pieces,thus allowing the network to be much more highly utilized. To doso, carefully mark each piece of the transfer so that the re-assemblyon the receiver side does not scramble the message.

### 6. A final implementation challenge: asynchronous message sendwithin-order delivery. That is, the client should be able to repeatedly callsend to send one message after the other; the receiver should call re-ceive and get each message in order, reliably; many messages from the sender should be able to be in flight concurrently. Also add as ender-side call that enables a client to wait for all outstanding mes-sages to be acknowledged.7. Now, one more pain point: measurement. Measure the bandwidthof each of your approaches; how much data can you transfer be-tween two different machines, at what rate? Also measure latency:for single packet send and acknowledgment, how quickly does itfinish? Finally, do your numbers look reasonable? What did youexpect? How can you better set your expectations so as to know ifthere is a problem, or that your code is working well?
