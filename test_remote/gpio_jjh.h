#ifndef _gpio_jjh_h
#define _gpio_jjh_h

extern int l2netgpio_connect(int *client_socket,char *IP,int port);
extern void Close_l2netgpio_Connect(int *client_socket);
extern int Get_NetGPIO_GPIStat(int *client_socket);
extern int L2NetGpio_SendRecv(int *client_socket,char *cmd,int cmdlen,char *res,int reslen);
extern int netGpio_init(void);
extern unsigned char read_netgpi(void);
extern void write_netgpo(unsigned char value);
#endif 






