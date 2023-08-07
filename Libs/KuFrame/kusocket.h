#ifndef __KU_SOCKET__
#define __KU_SOCKET__

#include <RTL.h>
#include "bsp.h"
#include "kulist.h"

typedef enum
{
	TCP,
	UDP
} IpProtocol;

typedef struct
{
	int handle;
	SOCKADDR_IN address;
} KuSocket;

typedef struct
{
	KuSocket socket;
	int isstarted;
	int clientcount;
} KuSocketServer;

typedef struct
{
	KuSocket socket;
	int isconnected;
	char *recvbuf;
	KuSocketServer *server;
} KuSocketClient;

extern char *socket_error(int code);
extern int socket_init(KuSocket *socket, unsigned int ip, int port, IpProtocol protocol);
extern int socket_start(KuSocketServer *server, int count);
extern int socket_stop(KuSocketServer *socket);
extern int socket_accept(KuSocketServer *server, KuSocketClient *client);

extern int socket_disconnect(KuSocketClient *client);
extern int socket_send(KuSocketClient *client, char *buf, int len);
extern int socket_recv(KuSocketClient *client);

#endif
