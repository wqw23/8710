
#ifndef _IOTPLATFORM_OTA_HEADER_
#define _IOTPLATFORM_OTA_HEADER_

int IOTOtaP_ReceiveDataPrepare(void);
int IOTOtaP_ReceiveData(void *buffer, unsigned int length);
int IOTOtaP_Prepare(void);
int IOTOtaP_Finished(int result);

#endif  //!_IOTPLATFORM_OTA_HEADER_

