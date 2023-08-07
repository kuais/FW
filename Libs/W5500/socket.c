/**
******************************************************************************
* @file    			socket.c *
* @author  			WisIOE Software Team *
* @version 			V1.2
**
* @date    			2017-11-01
**
* @brief 				socket编程相关函数 *
******************************************************************************
* @company  		深圳炜世科技有限公司 *
* @website  		www.wisioe.com
**
* @forum        www.w5500.cn
**
* @qqGroup      383035001
**
******************************************************************************
*/
#include "socket.h"
#include "stdio.h"
#include "w5500.h"
#include "w5500_interface.h"
#include <RTL.h>
#include <string.h>

// #define SOCKET_DEBUG

/**
*@brief   This Socket function initialize the channel in perticular mode,
                    and set the port and wait for W5200 done it.
*@param		s: socket number.
*@param		protocol: The socket to chose.
*@param		port:The port to bind.
*@param		flag: Set some bit of MR,such as **< No Delayed Ack(TCP) flag.
*@return  1 for sucess else 0.
*/
uint8 socket_new(SOCKET s, uint8 protocol, uint16 port, uint8 flag)
{
    uint8 ret;
    if (((protocol & 0x0F) == Sn_MR_TCP) || ((protocol & 0x0F) == Sn_MR_UDP) || ((protocol & 0x0F) == Sn_MR_IPRAW) || ((protocol & 0x0F) == Sn_MR_MACRAW) ||
        ((protocol & 0x0F) == Sn_MR_PPPOE))
    {
        socket_close(s);
        w5500_WriteByte(Sn_MR(s), protocol | flag);
        w5500_WriteByte(Sn_PORT0(s), (uint8)((port & 0xff00) >> 8));
        w5500_WriteByte(Sn_PORT1(s), (uint8)(port & 0x00ff));
        w5500_WriteByte(Sn_CR(s), Sn_CR_OPEN); // run sockinit Sn_CR

        /* wait to process the command... */
        while (w5500_ReadByte(Sn_CR(s)))
            os_dly_wait(1);
        /* ------- */
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

/**
 *@brief   This function close the socket and parameter is "s" which represent
 *the socket number
 *@param		s: socket number.
 *@return  None
 */
void socket_close(SOCKET s)
{

    w5500_WriteByte(Sn_CR(s), Sn_CR_CLOSE);

    /* wait to process the command... */
    while (w5500_ReadByte(Sn_CR(s)))
        os_dly_wait(1); /* ------- */

    w5500_WriteByte(Sn_IR(s), 0xFF); /* all clear */
}

/**
*@brief   This function established  the connection for the channel in passive
(server) mode. This function waits for the request from the peer.
*@param		s: socket number.
*@return  1 for success else 0.
*/
uint8 socket_listen(SOCKET s)
{
    uint8 ret;
    if (socket_status(s) == SOCK_INIT)
    {
        w5500_WriteByte(Sn_CR(s), Sn_CR_LISTEN);
        /* wait to process the command... */
        while (w5500_ReadByte(Sn_CR(s)))
            ;
        /* ------- */
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

/**
*@brief		This function established  the connection for the channel in Active
(client) mode. This function waits for the untill the connection is established.
*@param		s: socket number.
*@param		addr: The server IP address to connect
*@param		port: The server IP port to connect
*@return  1 for success else 0.
*/
uint8 socket_connect(SOCKET s, uint8 *addr, uint16 port)
{
    uint8 ret;
    if (((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
        ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) || (port == 0x00))
    {
        ret = 0;
    }
    else
    {
        ret = 1;
        // set destination IP
        w5500_WriteByte(Sn_DIPR0(s), addr[0]);
        w5500_WriteByte(Sn_DIPR1(s), addr[1]);
        w5500_WriteByte(Sn_DIPR2(s), addr[2]);
        w5500_WriteByte(Sn_DIPR3(s), addr[3]);
        w5500_WriteByte(Sn_DPORT0(s), (uint8)((port & 0xff00) >> 8));
        w5500_WriteByte(Sn_DPORT1(s), (uint8)(port & 0x00ff));
        w5500_WriteByte(Sn_CR(s), Sn_CR_CONNECT);
        /* wait for completion */
        while (w5500_ReadByte(Sn_CR(s)))
            os_dly_wait(1);

        while (1)
        {
            uint8_t status = socket_status(s);
            if (status == SOCK_ESTABLISHED)
                break;
            uint8_t flag = getSn_IR(s);
            if (flag & Sn_IR_TIMEOUT)
            {
                w5500_WriteByte(Sn_IR(s), (Sn_IR_TIMEOUT)); // clear TIMEOUT Interrupt
                ret = 0;
                break;
            }
			else if (flag & Sn_IR_DISCON)
            {
                w5500_WriteByte(Sn_IR(s), (Sn_IR_DISCON)); // clear TIMEOUT Interrupt
                ret = 0;
                break;
            }
            os_dly_wait(1);
        }
    }

    return ret;
}

/**
 *@brief   This function used for disconnect the socket s
 *@param		s: socket number.
 *@return  1 for success else 0.
 */
void socket_disconnect(SOCKET s)
{
    w5500_WriteByte(Sn_CR(s), Sn_CR_DISCON);

    /* wait to process the command... */
    while (w5500_ReadByte(Sn_CR(s)))
        ;
    /* ------- */
}

/**
 *@brief   This function used to send the data in TCP mode
 *@param		s: socket number.
 *@param		buf: data buffer to send.
 *@param		len: data length.
 *@return  1 for success else 0.
 */
uint16 socket_send(SOCKET s, const uint8 *buf, uint16 len)
{
    uint8 status = 0;
    uint16 freesize = 0;
    uint16 ret = len;

    // if freebuf is available, start.
    do
    {
        freesize = getSn_TX_FSR(s);
        status = socket_status(s);
        if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
            return 0;
        // os_dly_wait(1);
    } while (freesize < ret);

    // copy data
    send_data_processing(s, (uint8 *)buf, ret);
    w5500_WriteByte(Sn_CR(s), Sn_CR_SEND);

    /* wait to process the command... */
    while (w5500_ReadByte(Sn_CR(s)))
        ;

    while ((w5500_ReadByte(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
    {
        status = socket_status(s);
        if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
        {
            printf("SEND_OK Problem!!\r\n");
            socket_close(s);
            return 0;
        }
        // os_dly_wait(1);
    }
    w5500_WriteByte(Sn_IR(s), Sn_IR_SEND_OK);

#ifdef __DEF_IINCHIP_INT__
    putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
    w5500_WriteByte(Sn_IR(s), Sn_IR_SEND_OK);
#endif
//	printf("\r\nSend:%d\r\n", len);
#ifdef SOCKET_DEBUG
    printf("\r\nWrite:\r\n");
    for (int i = 0; i < len; i++)
        printf("%c", buf[i]);
    printf("\r\n");
#endif
    //    os_dly_wait(350);
    return ret;
}

/**
*@brief		This function is an application I/F function which is used to
receive the data in TCP mode. It continues to wait for data as much as the
application wants to receive.
*@param		s: socket number.
*@param		buf: data buffer to receive.
*@param		len: data length.
*@return  received data size for success else 0.
*/
uint16 socket_recv(SOCKET s, uint8 *buf, uint16 len)
{
    if (len == 0)
        return 0;
    int lenBak, err1, err2, len0 = getSn_RX_RSR(s);
    U8 status;
    err1 = err2 = 0;
    while (len0 < len)
    {
        status = socket_status(s);
        if ((status != SOCK_ESTABLISHED))
        {
            err1++;
            if (err1 == 2000)
                return 0;
        }
        err1 = 0;
        len0 = getSn_RX_RSR(s);
        if (lenBak != len0)
        {
            lenBak = len0;
            err2 = 0;
        }
        else
        {

            // if (err2++ == 8000)
            if (err2++ == 20000)
                return 0;				
        }
        os_dly_wait(1);
    }
    recv_data_processing(s, buf, len);
    w5500_WriteByte(Sn_CR(s), Sn_CR_RECV);
    /* wait to process the command... */
    while (w5500_ReadByte(Sn_CR(s)))
        ;

//	printf("\r\nRead Retry:%d\r\n", errCount);
//	printf("\r\nRead:%d\r\n", len);
#ifdef SOCKET_DEBUG
    printf("\r\nRead:\r\n");
    for (int i = 0; i < len; i++)
        printf("%c", buf[i]);
    printf("\r\n");
#endif
    //    os_dly_wait(100);
    return len;
}

/**
*@brief   This function is an application I/F function which is used to send the
data for other then TCP mode. Unlike TCP transmission, The peer's destination
address and the port is needed.
*@param		s: socket number.
*@param		buf: data buffer to send.
*@param		len: data length.
*@param		addr: IP address to send.
*@param		port: IP port to send.
*@return  This function return send data size for success else 0.
*/
uint16 socket_sendto(SOCKET s, const uint8 *buf, uint16 len, uint8 *addr, uint16 port)
{
    uint16 ret = len;

    if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) || ((port == 0x00))) //||(ret == 0) )
    {
        /* added return value */
        ret = 0;
    }
    else
    {
        w5500_WriteByte(Sn_DIPR0(s), addr[0]);
        w5500_WriteByte(Sn_DIPR1(s), addr[1]);
        w5500_WriteByte(Sn_DIPR2(s), addr[2]);
        w5500_WriteByte(Sn_DIPR3(s), addr[3]);
        w5500_WriteByte(Sn_DPORT0(s), (uint8)((port & 0xff00) >> 8));
        w5500_WriteByte(Sn_DPORT1(s), (uint8)(port & 0x00ff));
        // copy data
        send_data_processing(s, (uint8 *)buf, ret);
        w5500_WriteByte(Sn_CR(s), Sn_CR_SEND);
        /* wait to process the command... */
        while (w5500_ReadByte(Sn_CR(s)))
            os_dly_wait(1);
        /* ------- */
        while ((w5500_ReadByte(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
        {
            if (w5500_ReadByte(Sn_IR(s)) & Sn_IR_TIMEOUT)
            {
                /* clear interrupt */
                w5500_WriteByte(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); /* clear SEND_OK & TIMEOUT */
                return 0;
            }
            os_dly_wait(1);
        }
        w5500_WriteByte(Sn_IR(s), Sn_IR_SEND_OK);
    }
    return ret;
}

/**
*@brief   This function is an application I/F function which is used to receive
the data in other then TCP mode. This function is used to receive UDP, IP_RAW
and MAC_RAW mode, and handle the header as well.
*@param		s: socket number.
*@param		buf: data buffer to receive.
*@param		len: data length.
*@param		addr: IP address to receive.
*@param		port: IP port to receive.
*@return	This function return received data size for success else 0.
*/
uint16 socket_recvfrom(SOCKET s, uint8 *buf, uint16 len, uint8 *addr, uint16 *port)
{
    uint8 head[8];
    uint16 data_len = 0;
    uint16 ptr = 0;
    uint32 addrbsb = 0;
    if (len > 0)
    {
        ptr = w5500_ReadByte(Sn_RX_RD0(s));
        ptr = ((ptr & 0x00ff) << 8) + w5500_ReadByte(Sn_RX_RD1(s));
        addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18;

        switch (w5500_ReadByte(Sn_MR(s)) & 0x07)
        {
        case Sn_MR_UDP:
            w5500_ReadBytes(addrbsb, head, 0x08);
            ptr += 8;
            // read peer's IP address, port number.
            addr[0] = head[0];
            addr[1] = head[1];
            addr[2] = head[2];
            addr[3] = head[3];
            *port = head[4];
            *port = (*port << 8) + head[5];
            data_len = head[6];
            data_len = (data_len << 8) + head[7];

            addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18;
            w5500_ReadBytes(addrbsb, buf, data_len);
            ptr += data_len;

            w5500_WriteByte(Sn_RX_RD0(s), (uint8)((ptr & 0xff00) >> 8));
            w5500_WriteByte(Sn_RX_RD1(s), (uint8)(ptr & 0x00ff));
            break;

        case Sn_MR_IPRAW:
            //	   	printf("\r\n Sn_MR_IPRAW \r\n");
            w5500_ReadBytes(addrbsb, head, 0x06);

            ptr += 6;
            addr[0] = head[0];
            addr[1] = head[1];
            addr[2] = head[2];
            addr[3] = head[3];
            data_len = head[4];
            data_len = (data_len << 8) + head[5];

            addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18;

            //		printf(" data：%d \r\n",data_len);
            w5500_ReadBytes(addrbsb, buf, data_len);

            ptr += data_len;

            w5500_WriteByte(Sn_RX_RD0(s), (uint8)((ptr & 0xff00) >> 8));
            w5500_WriteByte(Sn_RX_RD1(s), (uint8)(ptr & 0x00ff));

            break;

        case Sn_MR_MACRAW:
            //	 printf("\r\n Sn_MR_MCRAW \r\n");
            w5500_ReadBytes(addrbsb, head, 0x02);
            ptr += 2;
            data_len = head[0];
            data_len = (data_len << 8) + head[1] - 2;
            if (data_len > 1514)
            {
                printf("data_len over 1514\r\n");
                while (1)
                    ;
            }

            addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18;
            w5500_ReadBytes(addrbsb, buf, data_len);
            ptr += data_len;

            w5500_WriteByte(Sn_RX_RD0(s), (uint8)((ptr & 0xff00) >> 8));
            w5500_WriteByte(Sn_RX_RD1(s), (uint8)(ptr & 0x00ff));
            break;

        default:
            break;
        }
        w5500_WriteByte(Sn_CR(s), Sn_CR_RECV);

        /* wait to process the command... */
        while (w5500_ReadByte(Sn_CR(s)))
            ;
        /* ------- */
    }
    return data_len;
}

uint8 socket_status(SOCKET s) { return w5500_ReadByte(Sn_SR(s)); }

#ifdef __MACRAW__
/**
 *@brief   OPen the 0-th socket with MACRAW mode
 *@param		None
 *@return	None
 */
void macraw_open(void)
{
    uint8 sock_num = 0;
    uint16 dummyPort = 0;
    uint8 mFlag = 0;
    sock_num = 0;
    socket_close(sock_num); // Close the 0-th socket
    socket_new(sock_num, Sn_MR_MACRAW, dummyPort, mFlag);
}

/**
 *@brief   OPen the 0-th socket with MACRAW mode
 *@param		buf: data buffer to send.
 *@param		len: data length.
 *@return	This function return sended data size for success else 0.
 */
uint16 macraw_send(const uint8 *buf, uint16 len)
{
    uint16 ret = 0;
    uint8 sock_num;
    sock_num = 0;

    if (len > getIINCHIP_TxMAX(sock_num))
        ret = getIINCHIP_TxMAX(sock_num); // check size not to exceed MAX size.
    else
        ret = len;

    send_data_processing(sock_num, (uint8 *)buf, len);

    // W5500 SEND COMMAND
    w5500_WriteByte(Sn_CR(sock_num), Sn_CR_SEND);
    while (w5500_ReadByte(Sn_CR(sock_num)))
        ;
    while ((w5500_ReadByte(Sn_IR(sock_num)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
        ;
    w5500_WriteByte(Sn_IR(sock_num), Sn_IR_SEND_OK);

    return ret;
}

/**
 *@brief   OPen the 0-th socket with MACRAW mode
 *@param		buf: data buffer to send.
 *@param		len: data length.
 *@return	This function return received data size for success else 0.
 */
uint16 macraw_recv(uint8 *buf, uint16 len)
{
    uint8 sock_num;
    uint16 data_len = 0;
    uint16 dummyPort = 0;
    uint16 ptr = 0;
    uint8 mFlag = 0;
    sock_num = 0;

    if (len > 0)
    {

        data_len = 0;

        ptr = w5500_ReadByte(Sn_RX_RD0(sock_num));
        ptr = (uint16)((ptr & 0x00ff) << 8) + w5500_ReadByte(Sn_RX_RD1(sock_num));
        //-- read_data(s, (uint8 *)ptr, data, len); // read data
        data_len = IINCHIP_READ_RXBUF(0, ptr);
        ptr++;
        data_len = ((data_len << 8) + IINCHIP_READ_RXBUF(0, ptr)) - 2;
        ptr++;

        if (data_len > 1514)
        {
            printf("data_len over 1514\r\n");
            printf("\r\nptr: %X, data_len: %X", ptr, data_len);

            /** recommand : close and open **/
            socket_close(sock_num); // Close the 0-th socket
            socket_new(sock_num, Sn_MR_MACRAW, dummyPort,
                       mFlag); // OPen the 0-th socket with MACRAW mode
            return 0;
        }

        IINCHIP_READ_RXBUF_BURST(sock_num, ptr, data_len, (uint8 *)(buf));
        ptr += data_len;

        w5500_WriteByte(Sn_RX_RD0(sock_num), (uint8)((ptr & 0xff00) >> 8));
        w5500_WriteByte(Sn_RX_RD1(sock_num), (uint8)(ptr & 0x00ff));
        w5500_WriteByte(Sn_CR(sock_num), Sn_CR_RECV);
        while (w5500_ReadByte(Sn_CR(sock_num)))
            ;
    }

    return data_len;
}
#endif
