//
// Created by greene machine on 2/15/21.
//

#include "UDP-lib.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // int sd = UDP_Open(20000);
    int sd = UDP_Open(20000);
    struct sockaddr_in addrSnd, addrRcv;
    // int rc = UDP_FillSockAddr(&addrSnd, "localhost", 10000);
    int rc = UDP_FillSockAddr(&addrSnd, "cs.wisc.edu", 10000);
    printf("rc fill sockaddr: %d\n", rc);
    if (rc != 0)
        return EXIT_FAILURE;
    char message[BUFFER_SIZE];
    sprintf(message, "hello world");
    printf("sending message!\n");
    rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    printf("rc udp write: %d\n", rc);
    if (rc > 0) {
        rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
        printf("rc udp read: %d\n", rc);
        if (rc != 0)
            return EXIT_FAILURE;
    }
    return 0;
}