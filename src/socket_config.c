#include "socket_config.h"



void check(int result, const char *errMsg, int errCode)
{
    if (result == -1)
    {
        perror(errMsg);
        exit(errCode);
    }
}

int set_blocking_mode(int hsocket, int enable_block)
{
    int flag = fcntl(hsocket, F_GETFL, 0);
    if (flag < 0)
    {
        return -1;
    }

    if (enable_block)
    {
        flag &= ~NON_BLOCK_FLAG;
    }
    else
    {
        flag |= NON_BLOCK_FLAG;
    }

    return fcntl(hsocket, F_SETFL, flag);
}

int set_timeout(int hsocket, struct timeval tv) // 변경된 시그니처
{
    int result;

    result = setsockopt(hsocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (result < 0)
    {
        return -1;
    }
    result = setsockopt(hsocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    if (result < 0)
    {
        return -1;
    }

    return 0;
}

int set_buffer_size(int hsocket, int size)
{
    return setsockopt(hsocket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

int set_can_filter(int hsocket, struct can_filter rfilter[4])
{
    socklen_t filter_size = sizeof(struct can_filter) * 4; // 배열의 크기를 직접 계산
    return setsockopt(hsocket, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter, filter_size); // &rfilter가 아닌 rfilter를 사용
}


int set_loopback(int hsocket, int loopback)
{
    return setsockopt(hsocket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
}

int set_recv_own_msgs(int hsocket, int recv_own_msgs)
{
    return setsockopt(hsocket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs));
}

int create_socket(const char *ifname)
{
    int result;
    int hsocket;
    struct sockaddr_can addr;
    struct ifreq ifr;

    hsocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (hsocket < 0)
    {
        return ERR_SOCKET_CREATE_FAILURE;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    memset(&addr, 0, sizeof(struct sockaddr_can));

    strcpy(ifr.ifr_name, ifname);
    result = ioctl(hsocket, SIOCGIFINDEX, &ifr);
    if (result < 0)
    {
        return ERR_SOCKET_CREATE_FAILURE;
    }

    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_family = AF_CAN;

    if (bind(hsocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return ERR_SOCKET_CREATE_FAILURE;
    }

    return hsocket;
}

int configure_socket(int hsocket, int enable_block, struct timeval tv, struct can_filter rfilter[4], int loopback, int recv_own_msgs)
{
    int result;

    result = set_blocking_mode(hsocket, enable_block);
    if (result < 0)
    {
        return ERR_SOCKET_CONFIGURE_FAILURE;
    }

    if (enable_block)
    {
        result = set_timeout(hsocket, tv);
        if (result < 0)
        {
            return ERR_SOCKET_CONFIGURE_FAILURE;
        }
    }

    result = set_buffer_size(hsocket, BUFFER_SIZE);
    if (result < 0)
    {
        return ERR_SOCKET_CONFIGURE_FAILURE;
    }

    result = set_can_filter(hsocket, rfilter);
    if (result < 0)
    {
        return ERR_SOCKET_CONFIGURE_FAILURE;
    }

    result = set_loopback(hsocket, loopback);
    if (result < 0)
    {
        return ERR_SOCKET_CONFIGURE_FAILURE;
    }

    result = set_recv_own_msgs(hsocket, recv_own_msgs);
    if (result < 0)
    {
        return ERR_SOCKET_CONFIGURE_FAILURE;
    }

    return 0;
}

void configure_sockets(int sockets[], int count, int enable_block, struct timeval tv, struct can_filter rfilter[4], int loopback, int recv_own_msgs)
{
    for (int i = 0; i < count; i++)
    {
        int result = configure_socket(sockets[i], enable_block, tv, rfilter, loopback, recv_own_msgs);
        check(result, "Failed to configure socket", ERR_SOCKET_CONFIGURE_FAILURE);
    }
}
