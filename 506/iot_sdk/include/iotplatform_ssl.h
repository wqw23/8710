#ifndef _IOTPLATFORM_SSL_HEADER_
#define _IOTPLATFORM_SSL_HEADER_ 

#include "datatype.h"
#include "iotplatform.h"

typedef void* SSL_Handle;
void IOTSSL_Destroy(SSL_Handle handle);
SSL_Handle IOTSSL_New(const char *server_name, int server_port, const char *cert);
int IOTSSL_GetError(void);
int IOTSSL_Read(SSL_Handle handle, UINT32 read_length, void *read_buf);
int IOTSSL_Readn(SSL_Handle handle, UINT32 read_length, void *read_buf);
int IOTSSL_Write(SSL_Handle handle, char *data, int data_len);
SSL_Handle IOTSSL_ServerNew(const char *server_ip, int server_port,
            const char *cert, int cert_len, const char *key,  int key_len);
int IOTSSL_ServerAccept(SSL_Handle handle);
int IOTSSL_ServerShutdown(SSL_Handle handle);

#endif  //!_IOTPLATFORM_SSL_HEADER_

