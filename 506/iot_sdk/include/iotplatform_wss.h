
#ifndef _IOTPLATFORM_WSS_HEADER_
#define _IOTPLATFORM_WSS_HEADER_

int IOTWebsocket_Init(void);
void IOTWebsocket_Exit(void);
int IOTWebsocket_New(char *cloud_ip, char *cloud_port,char *path, char *rootcert, void *recvFunc);
int IOTWebsocket_Send(const char *data, unsigned int length);
int IOTWebsocket_IsConnected(void);
void IOTWebsocket_Close(void);

#endif  //!_IOTPLATFORM_WSS_HEADER_

