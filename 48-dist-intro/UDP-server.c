//
// Created by greene machine on 2/15/21.
//

#include "UDP-lib.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int sd = UDP_Open(10000);
    assert(sd > -1);
    printf("starting!\n");
    while (1) {
        struct sockaddr_in addr;
        char message[BUFFER_SIZE];
        printf("waiting on message!\n");
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        printf("message received!\n");
        if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "goodbye world");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
        }
    }
    return 0;
}