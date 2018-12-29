#ifndef DATATYPE_H
#define DATATYPE_H


#define SHORTSTRING     (64)
#define LONGSTRING      (256)
#define LARGESTRING     (2048*64)
#ifndef WIN32
#define MAX_PATH        (256)
//typedef int             BOOLEAN;
#endif


// type definitions
typedef unsigned char           UINT8;
typedef char                    INT8;
typedef unsigned short int      UINT16;
typedef short int               INT16;
typedef unsigned int            UINT32;
typedef int                     INT32;
typedef unsigned long long      UINT64;
typedef long long               INT64;

// error return value
#define E_SUCCESS        (0)
#define E_FAILED         (-1)
#define E_INVALID        (-2)
#define E_NO_MEMORY      (-3)
#define E_EXIST          (-4)
#define E_NO_EXIST       (-5)
#define E_SOCKET_CONNECT (-6)
#define E_NETWORK_CONNECT    (-7)
#define E_NETWORK_DISCONNECT (-8)
#define E_NO_DEVICE          (-9)
#define E_PARAM_ERROR        (-10)
#define E_IN_PROCESS         (-11)
#define E_NO_DATA            (-12)
#define E_SUPPURTED          (-13)
#define E_UNSUPPORT          (-14)
#define E_BUSY               (-15)
#define E_INPUT              (-16)
#define E_UNKNOWN            (-17)
#define E_PARAM_NULL         (-18)

#define E_OPENDED            (-20)
#define E_READ               (-21)
#define E_WRITE              (-22)

#define E_AUTH_WAIT          (-23)
#define E_ACCEPT_TEM_UNAVAIL (-24)

//wifi 
#define E_NO_AP_FOUND        (-25)
#define E_WRONG_PASSWORD     (-26)
#define E_CONNECTING         (-27)

// release version
typedef enum {
    RELEASE_LEVEL_DEV,  // for delevelop
    RELEASE_LEVEL_PVT,  // for test
    RELEASE_LEVEL_DEM,  // for demo
    RELEASE_LEVEL_API   // for product
}eRelLevel;

#endif //DATATYPE_H
