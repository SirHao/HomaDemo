#include "homa.h"
#include<stdio.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

int fd;

static void sigio_handler(int signo)
{
    if (signo != SIGIO){
        printf("not my signo;");
        return ;
    }

}
int main()
{
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_HOMA);
    if (fd < 0) {
        printf("err when socket()");
        return 0;
    }

    int port = 8889;
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    addr_in.sin_addr.s_addr=INADDR_ANY;
    if (bind(fd, (struct sockaddr *) &addr_in, sizeof(addr_in)) != 0){
        printf("err when bind()");
        return 0;
    }

//    if (listen(fd, 1000) == -1) {
//        printf("err when listen()");
//        return 0;
//    }

    char buf[HOMA_MAX_MESSAGE_LENGTH];
    struct sockaddr_in source;
    int length;
    int ret = fcntl(fd, F_SETOWN, getpid());
    if (ret<0) {
        printf("err occur in this ");
    }
    while (1) {
        uint64_t id = 0;
        int result;

        while (1) {
            length = homa_recv(fd, buf, HOMA_MAX_MESSAGE_LENGTH,
                               HOMA_RECV_REQUEST, (struct sockaddr *) &source,
                               sizeof(source), &id);
            printf("homa receive msg! buf:%s\n",buf);
            if (length >= 0)
                break;
            if ((errno == EBADF) || (errno == ESHUTDOWN))
                return 0;
            else if ((errno != EINTR) && (errno != EAGAIN))
                printf("recv error");
        }

        result = homa_reply(fd, buf, length,
                            (struct sockaddr *) &source, sizeof(source),id);
        if (result < 0) {
            printf("homa reply error");
            exit(1);
        }
    }
    return 0;
}