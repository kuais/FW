#include "kusocket.h"
#include "ku.h"

char *socket_error(int code)
{
    switch (code)
    {
    case SCK_SUCCESS:
        return "Success";
    case SCK_ERROR:
        return "General Error";
    case SCK_EINVALID:
        return "Invalid socket descriptor";
    case SCK_EINVALIDPARA:
        return "Invalid parameter";
    case SCK_EWOULDBLOCK:
        return "It would have blocked";
    case SCK_EMEMNOTAVAIL:
        return "Not enough memory in memory pool";
    case SCK_ECLOSED:
        return "Connection is closed or aborted";
    case SCK_ELOCKED:
        return "Socket is locked in RTX environment";
    case SCK_ETIMEOUT:
        return "Socket, Host Resolver timeout";
    case SCK_EINPROGRESS:
        return "Host Name resolving in progress";
    case SCK_ENONAME:
        return "Host Name not existing ";
    default:
        return "Unknow";
    }
}

int socket_init(KuSocket *sock, unsigned int ip, int port, IpProtocol protocol)
{
    switch (protocol)
    {
    case TCP:
        sock->handle = socket(AF_INET, SOCK_STREAM, 0);
        sock->address.sin_family = PF_INET;
        break;
    case UDP:
        sock->handle = socket(AF_INET, SOCK_DGRAM, 0);
        sock->address.sin_family = PF_INET;
    default:
        break;
    }
    if (sock->handle < 0)
        printf("%s\r\n", socket_error(sock->handle));
    else
    {
        sock->address.sin_port = htons(port);
        sock->address.sin_addr.s_addr = ip;
    }
    return sock->handle;
}

int socket_start(KuSocketServer *server, int count)
{
    int ret = 0;
    SOCKADDR_IN addr = server->socket.address;
    if (server->isstarted)
        return ret;

    ret = bind(server->socket.handle, (SOCKADDR *)&addr, sizeof(addr));
    if (ret >= 0)
        ret = listen(server->socket.handle, count);
    if (ret < 0)
        printf("%s\r\n", socket_error(ret));
    else
    {
        server->isstarted = TRUE;
        printf("服务端[%d.%d.%d.%d:%d]已启动.\r\n", server->socket.address.sin_addr.s_b1, server->socket.address.sin_addr.s_b2,
               server->socket.address.sin_addr.s_b3, server->socket.address.sin_addr.s_b4, ntohs(server->socket.address.sin_port));
    }
    return ret;
}
int socket_stop(KuSocketServer *server)
{
    int ret = 0;
    if (!server->isstarted)
        return 0;

    ret = closesocket(server->socket.handle);
    if (ret < 0)
        printf("%s\r\n", socket_error(ret));
    else
    {
        printf("服务端[%d.%d.%d.%d:%d]已停止.\r\n", server->socket.address.sin_addr.s_b1, server->socket.address.sin_addr.s_b2,
               server->socket.address.sin_addr.s_b3, server->socket.address.sin_addr.s_b4, ntohs(server->socket.address.sin_port));
    }
    server->isstarted = FALSE;
    return ret;
}
int socket_accept(KuSocketServer *server, KuSocketClient *client)
{
    int ret, addrLen;
    ret = accept(server->socket.handle, (SOCKADDR *)&client->socket.address, &addrLen);
    if (ret < 0)
        printf("%s\r\n", socket_error(ret));
    else
    {
        client->socket.handle = ret;
        client->server = server;
        client->isconnected = TRUE;
        client->server->clientcount++;
        printf("远程客户端[%d.%d.%d.%d:%d]已连接.\r\n", client->socket.address.sin_addr.s_b1, client->socket.address.sin_addr.s_b2,
               client->socket.address.sin_addr.s_b3, client->socket.address.sin_addr.s_b4, ntohs(client->socket.address.sin_port));
    }
    return ret;
}

int socket_disconnect(KuSocketClient *client)
{
    int ret = 0;
    if (!client->isconnected)
        return ret;

    client->isconnected = FALSE;
    client->server->clientcount--;
    ret = closesocket(client->socket.handle);
    if (ret < 0)
        printf("%s\r\n", socket_error(ret));
    else
    {
        printf("远程客户端[%d.%d.%d.%d:%d]已断开.\r\n", client->socket.address.sin_addr.s_b1, client->socket.address.sin_addr.s_b2,
               client->socket.address.sin_addr.s_b3, client->socket.address.sin_addr.s_b4, ntohs(client->socket.address.sin_port));
    }
    return ret;
}

int socket_send(KuSocketClient *client, char *buf, int len)
{
    int ret = 0;
    ret = send(client->socket.handle, buf, len, 0);
    if (ret < 0)
    {
        printf("%s\r\n", socket_error(ret));
        switch (ret)
        {
        case SCK_EINVALID:
        case SCK_ECLOSED:
            socket_disconnect(client);
            break;
        }
    }
    else
    {
        kuprintf("%d bytes have sent to [%d.%d.%d.%d:%d]!\r\n", len, client->socket.address.sin_addr.s_b1, client->socket.address.sin_addr.s_b2,
                 client->socket.address.sin_addr.s_b3, client->socket.address.sin_addr.s_b4, ntohs(client->socket.address.sin_port));
    }
    return ret;
}

int socket_recv(KuSocketClient *client)
{
    int ret = 0;
    ret = recv(client->socket.handle, client->recvbuf, sizeof(client->recvbuf), 0); // MSG_DONTWAIT
    if (ret < 0)
    {
        printf("%s\r\n", socket_error(ret));
        switch (ret)
        {
        case SCK_EINVALID:
        case SCK_ECLOSED:
            socket_disconnect(client);
            break;
        }
    }
    else
    {
        kuprintf("%d bytes have received from [%d.%d.%d.%d:%d]!\r\n", ret, client->socket.address.sin_addr.s_b1, client->socket.address.sin_addr.s_b2,
                 client->socket.address.sin_addr.s_b3, client->socket.address.sin_addr.s_b4, ntohs(client->socket.address.sin_port));
    }
    return ret;
}
