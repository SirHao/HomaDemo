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
#include <time.h>
int main()
{
    int skfd;
    skfd = socket(AF_INET, SOCK_DGRAM, 140);
    if (skfd < 0)
    {
        perror("socket error");
        exit(-1);
    }

    int ret;
    time_t tm;
    struct sockaddr_in desAddr;
    bzero(&desAddr, sizeof(desAddr));
    desAddr.sin_family = AF_INET;
    desAddr.sin_port = htons(8889);
    desAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char buf[HOMA_MAX_MESSAGE_LENGTH];
    char inputs[3];
    while (1)
    {
        inputs[0]='a';
        inputs[1]='b';
        uint64_t rpc_id;
        ret = homa_send(skfd, &inputs, strlen(inputs),(struct sockaddr*)&desAddr, sizeof(desAddr),&rpc_id);
        if (ret < 0)
        {
            perror("");
            exit(-1);
        }

        int len = homa_recv(skfd, buf, HOMA_MAX_MESSAGE_LENGTH,
                           HOMA_RECV_REQUEST, (struct sockaddr *) &desAddr,
                           sizeof(desAddr), &rpc_id);
        printf("homa client receive msg! buf:%s\n",buf);

        sleep(10);
    }

    return 0;
}
