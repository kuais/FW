/**
******************************************************************************
* @file    			w5500.c                                                      *
* @author  			WisIOE Software Team                                         *
* @version 			V1.2																												 *
* @date    			2017-11-01																								   *
* @brief 				读写W5500寄存器底层函数                                      *
******************************************************************************
* @company  		深圳炜世科技有限公司                                         *
* @website  		www.wisioe.com																							 *
* @forum        www.w5500.cn																								 *
* @qqGroup      383035001																										 *
******************************************************************************
*/
#include "w5500.h"
#include "utility.h"
#include "w5500_interface.h"
#include <stdio.h>
#include <string.h>

#ifdef __DEF_IINCHIP_PPP__
#include "md5.h"
#endif

/**
 *@brief		This function is to get the Max size to receive.
 *@param		s: socket number
 *@return	This function return the RX buffer size has been setted
 */
void iinchip_init(void)
{
    setMR(MR_RST);
#ifdef __DEF_IINCHIP_DBG__
    printf("MR value is %02x \r\n", IINCHIP_READ_COMMON(MR));
#endif
}

/**
 *@brief		This function is to set up gateway IP address.
 *@param		addr: a pointer to a 4 -byte array responsible to set the Gateway IP address
 *@return	None
 */
void setGAR(uint8 *addr) { w5500_WriteBytes(GAR0, addr, 4); }

/**
 *@brief		This function is to get gateway IP address.
 *@param		addr: a pointer to a 4 -byte array responsible to get the Gateway IP address
 *@return	None
 */
void getGAR(uint8 *addr) { w5500_ReadBytes(GAR0, addr, 4); }

/**
 *@brief 	This function is to set up SubnetMask address
 *@param		addr: a pointer to a 4 -byte array responsible to set the subway IP address.
 *@return	None
 */
void setSUBR(uint8 *addr) { w5500_WriteBytes(SUBR0, addr, 4); }
/**
 *@brief		This function is to set up MAC address.
 *@param		addr: a pointer to a 6 -byte array responsible to set the MAC address.
 *@return	None
 */
void setSHAR(uint8 *addr) { w5500_WriteBytes(SHAR0, addr, 6); }

/**
 *@brief		This function is to set up Source IP address.
 *@param		addr:a pointer to a 4 -byte array responsible to set the Source IP addres.
 *@return	None
 */
void setSIPR(uint8 *addr) { w5500_WriteBytes(SIPR0, addr, 4); }

/**
 *@brief		This function is to get Subnet mask.
 *@param		addr:a pointer to a 4 -byte array responsible to set the Subnet mask.
 *@return	None
 */
void getSUBR(uint8 *addr) { w5500_ReadBytes(SUBR0, addr, 4); }

/**
 *@brief		This function is to get up Source MAC .
 *@param		addr: a pointer to a 6 -byte array responsible to get the MAC
 *@return	None
 */
void getSHAR(uint8 *addr) { w5500_ReadBytes(SHAR0, addr, 6); }

/**
 *@brief		This function is to get up Source IP .
 *@param		addr: a pointer to a 4 -byte array responsible to get the Source IP
 *@return	None
 */
void getSIPR(uint8 *addr) { w5500_ReadBytes(SIPR0, addr, 4); }
/**
 *@brief		This function is to set the MR register.
 *@param		val: the value to set to MR
 *@return	None
 */
void setMR(uint8 val) { w5500_WriteByte(MR, val); }

/**
 *@brief		This function is to get Interrupt register in common register.
 *@param		None
 *@return	The value read from the IR register
 */
uint8 getIR(void) { return w5500_ReadByte(IR); }

/**
@brief		This function is to set up Retransmission time.
                    If there is no response from the peer or delay in response then retransmission
                    will be there as per RTR (Retry Time-value Register)setting
*@param		timeout: The value write to  the RTR0 register
*@return	None
*/
void setRTR(uint16 timeout)
{
    w5500_WriteByte(RTR0, (uint8)((timeout & 0xff00) >> 8));
    w5500_WriteByte(RTR1, (uint8)(timeout & 0x00ff));
}

/**
@brief		This function is to set the number of Retransmission.
                    If there is no response from the peer or delay in response then recorded time
                    as per RTR & RCR register seeting then time out will occur.
*@param		retry: Times to  retry
*@return	None
*/
void setRCR(uint8 retry) { w5500_WriteByte(WIZ_RCR, retry); }

/**
*@brief		This function is to the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)
                    If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
                    set in IR register.
*@param		mask: the bits to clear
*@return	None
*/
void clearIR(uint8 mask) { w5500_WriteByte(IR, ~mask | getIR()); }

/**
 *@brief  	This function is to set the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
 *@param		s: socket number
 *@param		Sn_MSSR: the maximum segment size
 *@return	None
 */
void setSn_MSS(SOCKET s, uint16 Sn_MSSR)
{
    w5500_WriteByte(Sn_MSSR0(s), (uint8)((Sn_MSSR & 0xff00) >> 8));
    w5500_WriteByte(Sn_MSSR1(s), (uint8)(Sn_MSSR & 0x00ff));
}

/**
 *@brief  	This function is to set the IP Time to live(TTL) Register
 *@param		s: socket number
 *@param		Sn_MSSR: the IP Time to live
 *@return	None
 */
void setSn_TTL(SOCKET s, uint8 ttl) { w5500_WriteByte(Sn_TTL(s), ttl); }

/**
 *@brief		This function is to read the Interrupt & Soket Status registe
 *@param		s: socket number
 *@return	socket interrupt status
 */
uint8 getSn_IR(SOCKET s) { return w5500_ReadByte(Sn_IR(s)); }

/**
 *@brief 	This function is to write the Interrupt & Soket Status register to clear the interrupt
 *@param		s: socket number
 *@return  socket interrupt status
 */
void setSn_IR(uint8 s, uint8 val) { w5500_WriteByte(Sn_IR(s), val); }

/**
 *@brief 	This function is to get socket status
 *@param		s: socket number
 *@return  socket status
 */
uint8 getSn_SR(SOCKET s) { return w5500_ReadByte(Sn_SR(s)); }

/**
*@brief		This fuction is to get socket TX free buf size
                    This gives free buffer size of transmit buffer. This is the data size that user can transmit.
                    User shuold check this value first and control the size of transmitting data
*@param		s: socket number
*@return  socket TX free buf size
*/
uint16 getSn_TX_FSR(SOCKET s)
{
    uint16 val = 0, val1 = 0;
    do
    {
        val1 = w5500_ReadByte(Sn_TX_FSR0(s));
        val1 = (val1 << 8) + w5500_ReadByte(Sn_TX_FSR1(s));
        if (val1 != 0)
        {
            val = w5500_ReadByte(Sn_TX_FSR0(s));
            val = (val << 8) + w5500_ReadByte(Sn_TX_FSR1(s));
        }
    } while (val != val1);
    return val;
}

/**
 *@brief		This fuction is to give size of received data in receive buffer.
 *@param		s: socket number
 *@return  socket TX free buf size
 */
uint16 getSn_RX_RSR(SOCKET s)
{
    uint16 val = 0, val1 = 0;
    do
    {
        val1 = w5500_ReadByte(Sn_RX_RSR0(s));
        val1 = (val1 << 8) + w5500_ReadByte(Sn_RX_RSR1(s));
        if (val1 != 0)
        {
            val = w5500_ReadByte(Sn_RX_RSR0(s));
            val = (val << 8) + w5500_ReadByte(Sn_RX_RSR1(s));
        }
    } while (val != val1);
    return val;
}

/**
*@brief   This function is being called by send() and sendto() function also.

                    This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
                    register. User should read upper byte first and lower byte later to get proper value.
*@param		s: socket number
*@param		data: data buffer to send
*@param		len: data length
*@return  socket TX free buf size
*/
void send_data_processing(SOCKET s, uint8 *data, uint16 len)
{
    uint16 ptr = 0;
    uint32 addrbsb = 0;
    if (len == 0)
        return;

    ptr = w5500_ReadByte(Sn_TX_WR0(s));
    ptr = ((ptr & 0x00ff) << 8) + w5500_ReadByte(Sn_TX_WR1(s));
    addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x10;
    w5500_WriteBytes(addrbsb, data, len);

    ptr += len;
    w5500_WriteByte(Sn_TX_WR0(s), (uint8)((ptr & 0xff00) >> 8));
    w5500_WriteByte(Sn_TX_WR1(s), (uint8)(ptr & 0x00ff));
}

/**
*@brief  	This function is being called by recv() also.
                    This function read the Rx read pointer register
                    and after copy the data from receive buffer update the Rx write pointer register.
                    User should read upper byte first and lower byte later to get proper value.
*@param		s: socket number
*@param		data: data buffer to receive
*@param		len: data length
*@return  None
*/
void recv_data_processing(SOCKET s, uint8 *data, uint16 len)
{
    uint16 ptr = 0;
    uint32 addrbsb = 0;

    if (len == 0)
    {
        printf("CH: %d Unexpected2 length 0\r\n", s);
        return;
    }

    ptr = w5500_ReadByte(Sn_RX_RD0(s));
    ptr = ((ptr & 0x00ff) << 8) + w5500_ReadByte(Sn_RX_RD1(s));

    addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18;
    w5500_ReadBytes(addrbsb, data, len);
    ptr += len;

    w5500_WriteByte(Sn_RX_RD0(s), (uint8)((ptr & 0xff00) >> 8));
    w5500_WriteByte(Sn_RX_RD1(s), (uint8)(ptr & 0x00ff));
}

/**
*@brief		This function set the transmit & receive buffer size as per the channels is used
*@Note: 	TMSR and RMSR bits are as follows\n
                    Maximum memory size for Tx, Rx in the W5500 is 16K Bytes,\n
                    In the range of 16KBytes, the memory size could be allocated dynamically by each channel.\n
                    Be attentive to sum of memory size shouldn't exceed 8Kbytes\n
                    and to data transmission and receiption from non-allocated channel may cause some problems.\n
                    If the 16KBytes memory is already  assigned to centain channel, \n
                    other 3 channels couldn't be used, for there's no available memory.\n
                    If two 4KBytes memory are assigned to two each channels, \n
                    other 2 channels couldn't be used, for there's no available memory.\n
*@param		index: 	socket index
*@param		tx_size: tx buffer size to set=tx_size[s]*(1024)
*@param		rx_size: rx buffer size to set=rx_size[s]*(1024)
*@return	None
*/
void socket_buf_init(uint16 index, uint8 tx_size, uint8 rx_size)
{
    w5500_WriteByte((Sn_TXMEM_SIZE(index)), tx_size);
    w5500_WriteByte((Sn_RXMEM_SIZE(index)), rx_size);
}

/**
 *@brief		检测物理层连接
 *@param		无
 *@return	无
 */
void PHY_check(void)
{
    uint8 PHY_connect = 0;
    uint8 v = getPHYCFG();
    PHY_connect = 0x01 & v;
    if (PHY_connect == 0)
    {
        printf(" \r\n 请检查网线是否连接?\r\n");

        while (PHY_connect == 0)
        {
            PHY_connect = 0x01 & getPHYCFG();
            printf(" .");
            delay_ms(500);
        }
        printf("\r\n");
    }
}

uint8 getPHYCFG(void) { return w5500_ReadByte(PHYCFGR); }

void setPHYCFG(uint8 v) { return w5500_WriteByte(PHYCFGR, v); }
