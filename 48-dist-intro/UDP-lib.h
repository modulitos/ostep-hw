// #define _POSIX_C_SOURCE 201112L    // getnameinfo >= 201112L, sem_timedwait
// >= 200112L, clock_gettime >= 199309L

#include <errno.h>
#include <fcntl.h> // For O_* constants
#include <netdb.h> // getaddrinfo(), getnameinfo(), AI_PASSIVE, gethostbyname()
#include <semaphore.h>
#include <stdio.h>  // printf(), fprintf(), perror
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <string.h> // memset(), strlen()
#include <string.h> // strncpy()
#include <sys/socket.h> // socket(), bind(), connect(), sendto(), recvfrom(), AF_UNSPEC, SOCK_DGRAM
#include <sys/stat.h>  // For mode constants
#include <sys/types.h> // some historical (BSD) implementations required it
#include <time.h>      // timespec, clock_gettime
#include <unistd.h>    // close()

#define BUFFER_SIZE 65507
#define TIMEOUT_SECONDS 10

#ifndef HOMEWORK_UDP_LIB_H
#define HOMEWORK_UDP_LIB_H

// Opens a datagram-style UDP socket at the specified port on localhost, and
// returns the socket descriptor.
int UDP_Open(int port) {
    int sd;
    // AF_INET:      IPv4 Internet protocols
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return -1;

    // sockaddr_in: https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in myaddr;

    bzero(&myaddr, sizeof(myaddr));

    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(port);
    // INADDR_ANY (0.0.0.0) means any address for binding;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        close(sd);
        return -1;
    }
    return sd;
}

// Updates addr with the
int UDP_FillSockAddr(struct sockaddr_in *addr, char *hostname, int port) {
    bzero(addr, sizeof(struct sockaddr_in));
    // https://man7.org/linux/man-pages/man7/ip.7.html
    addr->sin_family = AF_INET;   // address family, host byte order
    addr->sin_port = htons(port); // port in network byte order
    struct in_addr *in_addr;
    struct hostent *host_entry;
    if ((host_entry = gethostbyname(hostname)) == NULL)
        return -1;

    in_addr = (struct in_addr *)host_entry->h_addr;
    addr->sin_addr = *in_addr; // the internet address
    return 0;
}

int UDP_Write(int sd, struct sockaddr_in *addr, char *buffer, int n) {
    int addr_len = sizeof(struct sockaddr_in);
    return sendto(sd, buffer, n, 0, (struct sockaddr *)addr, addr_len);
}

int UDP_Read(int sd, struct sockaddr_in *addr, char *buffer, int n) {
    int len = sizeof(struct sockaddr_in);
    return recvfrom(sd, buffer, n, 0, (struct sockaddr *)addr,
                    (socklen_t *)&len);
}





// #define BUFFER_SIZE 65507
// #define TIMEOUT_SECONDS 10
//
// char * client_sem_name = "/client_sem";
// sem_t * client_sem;
// struct timespec ts;
//
// ssize_t UDP_Write2(int sfd, char *buffer, int nread, struct sockaddr *peer_addr,
//                    int peer_addr_len) {
//     if (nread > BUFFER_SIZE) {
//         fprintf(stderr, "Exceed max buffer size\n");
//         exit(EXIT_FAILURE);
//     }
//
//     int s = 0;
//
//     if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
//         perror("clock_gettime");
//         exit(EXIT_FAILURE);
//     }
//     ts.tv_sec += TIMEOUT_SECONDS;
//     // wait ack
//     errno = 0;
//     int isAsk = 0;
//     if ((isAsk = strncmp(buffer, "ack", 4)) != 0)
//         s = sem_timedwait(client_sem, &ts);
//
//     if (s == -1) {
//         if (errno == ETIMEDOUT) {
//             // retry
//             return UDP_Write(sfd, buffer, nread, peer_addr, peer_addr_len);
//         } else {
//             perror("sem_timedwait");
//             exit(EXIT_FAILURE);
//         }
//     } else {
//         if (isAsk != 0)
//             sem_post(client_sem);
//         return sendto(sfd, buffer, nread, 0, peer_addr, peer_addr_len);
//     }
// }
// int UDP_Open2(char *hostName, char *port, int server) {
//     struct addrinfo hints;
//     struct addrinfo *result, *rp;
//     int sfd, s;
//
//     memset(&hints, 0, sizeof(struct addrinfo));
//     hints.ai_family = AF_UNSPEC; /* Allow IPv4 or Ipv6 */
//     /* Datagram socket (connectionless, unreliable
//        messages of a fixed maximum length) */
//     hints.ai_socktype = SOCK_DGRAM;
//     hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
//     hints.ai_protocol = 0;       /* Any protocal */
//     hints.ai_canonname = NULL;
//     hints.ai_addr = NULL;
//     hints.ai_next = NULL;
//
//     s = getaddrinfo(hostName, port, &hints, &result);
//     if (s != 0) {
//         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
//         exit(EXIT_FAILURE);
//     }
//
//     /* getaddrinfo() returns a list of address structures.
//        Try each address until we successfully bind(2) for
//        server or connect(2) for client. If socket(2) (or
//        bind(2) or connect(2)) fails, we (close the socket
//        and) try the next address. */
//
//     for (rp = result; rp != NULL; rp = rp->ai_next) {
//         sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
//         if (sfd == -1)
//             continue;
//
//         if (server && bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
//             break; /* Success */
//         else if (!server && connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
//             break; /* Success */
//
//         close(sfd);
//     }
//
//     if (rp == NULL) { /* No address succeeded */
//         char *error_message = server ? "Could not bind" : "Could not connect";
//         fprintf(stderr, "%s\n", error_message);
//         exit(EXIT_FAILURE);
//     }
//
//     freeaddrinfo(result); /* No longer needed */
//
//     if (!server && (client_sem = sem_open(client_sem_name, O_CREAT, S_IRWXU,
//                                           1)) == SEM_FAILED) {
//         perror("sem_open");
//         exit(EXIT_FAILURE);
//     }
//
//     return sfd;
// }

#endif // HOMEWORK_UDP_LIB_H
