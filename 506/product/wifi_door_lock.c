
/******************************************************************************
**文 件 名: Wifi_Door_Lock_purifier.c
**作     者: wqw
**生成日期: 2018年11月11日
**功能描述: wifi门锁业务处理
******************************************************************************/
//Standard head file
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "time.h"
#include "timer_api.h"
#include "lwip/def.h"
#include "cJSON.h"
//IOT SDK head
#include "iotdevice.h"
#include "log.h"
#include "iotsdk_wifi.h"
#include "iotsdk.h"
#include "log.h"
#include "datatype.h"
#include "security_func.h"
//Adapter head file
#include "protocol.h"
#include "product_config.h"
#include "wifi_door_lock_uart_driver.h"
#include "wifi_door_lock.h"
#include "message_queue.h"
#include "hal_dev.h"
#include "report_task.h"


/*从MCU得到的帧类型*/
#define MCU_INVALID_ACK_PACKAGE                     (0x7f)//0x7f 表示不支持该命令,请不要再次发送命令.0x7f命令是MCU发送给WiFi模组
#define MCU_ACK_PACKAGE                             (0x00)
#define MCU_MODULE_START_INFO                       (0x01)//启动模组是否启动cmd=01
#define MCU_TIME_SERVICE_INFO                       (0x02)//该数据包用于 MCU 向 WiFi 模组请求授时
#define MCU_REPORT_DOOR_LOCK_INFO                   (0x04)//该数据包用于 MCU 上报门锁状态
#define MCU_RESET_COMMAND_INFO                      (0x07)//该数据包用于 MCU 告诉 WiFi模组当前进行复位需要进入待配置状态
#define MCU_DOOR_LOCK_REPORT_SETTING_INFO           (0x09)
#define MCU_DOOR_LOCK_DEPLOY_WITHDRAW_INFO          (0x0a)//该数据包是 WiFi 模组和 MCU 之间通信， 设置/通知 布防/撤防-Deploy/withdraw
#define MCU_DOOR_LOCK_REPORT_ADD_INFO               (0x0c)//MCU 上报添加密码/指纹/IC 卡
#define MCU_DOOR_LOCK_REPORT_DELETE_INFO            (0x0d)//MCU 上报删除密码/指纹/IC 卡
#define MCU_DOOR_LOCK_REPORT_ELECTRIC_QUANTITY_INFO (0x0f)//该命令用于MCU主动上报电池电量,尤其是在电池电量低于一个阈值时,每次唤醒WiFi模组都需要上报电池电量
#define MCU_DOOR_LOCK_REPORT_SERIAL_CODE_INFO       (0x10)//WIFI请求门锁串码时,mcu回复0x10响应

#define MCU_REVERSE_TIME_INFO                       (0x03)


/*从WIFI模块发送到MCU的数据包类型*/
#define DEVICE_INVALID_ACK_PACKAGE                  (0xff)//0xff表示不支持该命令,请不要再次发送命令.0xff命令是WiFi模组发送给MCU
#define DEVICE_ACK_PACKAGE                          (0x80)
#define DEVICE_READY_PACKAGE                        (0x81)
#define DEVICE_TIME_SERVICE_PACKAGE                 (0x82)
#define DEVICE_TURN_OFF_POWER_PACKAGE               (0x85)//该数据包用于 WiFi 模组告诉 MCU 可以断电了
#define DEVICE_OTA_APPLICATION_PACKAGE              (0x86)//该数据包用于WiFi模组告诉MCU当前准备进行OTA操作请180秒后才做强制断电
#define DEVICE_WIFI_STATUS_REPORT_PACKAGE           (0x88)//该数据包用于 WiFi 模组将自己的状态给到MCU,MCU可以控制喇叭播放出来
#define DEVICE_GET_DOOR_LOCK_SETTING_INFO_PACKAGE   (0x89)//该数据包是 WiFi 模组发送命令要求 MCU 上报门锁当前设置
#define DEVICE_REQUEST_ELECTRIC_QUANTITY_PACKAGE    (0x8f)//WiFi模组请求门锁电池电量
#define DEVICE_REQUEST_SERIAL_CODE_PACKAGE          (0x90)//WiFi模组请求门锁串码

#define DEVICE_REVERSE_TIME_PACKAGE                 (0x83)//该数据包用于 WiFi 模组向 MCU 请求授时,收到指令83,然后mcu发送数据包
#define DEVICE_APPLICATION_DEPLOY_WITHDRAW_PACKAGE  (0x8b)//WiFi 模组申请布防/撤防数据包
#define DEVICE_APPLICATION_ADD_PASSWORD_PACKAGE     (0x8c)//WiFi 模组申请添加密码
#define DEVICE_APPLICATION_DEL_PASSWORD_FINGERPRINT_IC_PACKAGE   (0x8d)//WiFi 模组申请删除密码/指纹/IC 卡
#define DEVICE_APPLICATION_TIME_START_WIFI_PACKAGE  (0x8e)//WiFi 模组申请定时启动 WiFi 模组


/*事件类型*/
#define TYPE_EVENT_MODULE_START                     (0x01)
#define TYPE_EVENT_REQ_TIME                         (0x02)
#define TYPE_EVENT_GET_TIME                         (0x03)
#define TYPE_EVENT_REPORT_LOCK_STATUS               (0x04)
#define TYPE_EVENT_RESET                            (0x07)
#define TYPE_EVENT_REPORT_SETTING_INFO              (0x09)
#define TYPE_EVENT_DEPLOY_WITHDRAW_INFO             (0x0a)
#define TYPE_EVENT_REPORT_ADD_INFO                  (0x0c)
#define TYPE_EVENT_REPORT_DELETE_INFO               (0x0d)
#define TYPE_EVENT_REPORT_ELECTRIC_QUANTITY_INFO    (0x0f)
#define TYPE_EVENT_REPORT_SERIAL_CODE_INFO          (0x10)

/*联网状态*/
#define NETWORK_CONFIGURE_DEVICE_STAR_SUCCESS           (0x00)//开始配置网络 WiFi模组已经进入到待配置网络状态
#define NETWORK_CONFIGURE_SEND_SSID_PWD_SUCCESS         (0x01)//客户端已经将 SSID 和 PWD 发给了WiFi模组
#define NETWORK_CONFIGURE_CONNECT_ROUTER_SUCCESS        (0x02)//配置网络中,WiFi 模组连接路由器成功
#define NETWORK_CONFIGURE_SSID_ERROR                    (0x03)//配置网络中,WiFi 模组连接路由器失败，失败的原因是无法找到指定的 SSID
#define NETWORK_CONFIGURE_PWD_ERROR                     (0x04)//配置网络中,WiFi 模组连接路由器失败，失败的原因是无法给出的路由密码错误
#define NETWORK_NORMAL_CLOUD_SUCCESS                    (0x05)//配置网络中,连接云端成功，整个配置结束
#define NETWORK_NORMAL_CONNECT_ROUTER_ERROR             (0x06)//正常使用中,连接路由器失败
#define NETWORK_NORMAL_CONNECT_CLOUD_ERROR              (0x07)//正常使用中,连接云端失败
#define NETWORK_NORMAL_CLOUD_AUTHORIZATION_ERROR        (0x08)//正常使用中,设备从云端授权失败


/*属性信息*/
typedef struct{
    UINT32 id;
    UINT16 value;
}COM_DEV_INT_ATTR_STRUCT;

/*属性信息*/
typedef struct{
    UINT32 id;
    UINT8* value;
}COM_DEV_STRING_ATTR_STRUCT;

static UINT8 s_Network_Status = NETWORK_CONFIGURE_DEVICE_STAR_SUCCESS;  /*记录网络状态*/

static COM_DEV_INT_ATTR_STRUCT s_Com_Dev_Int_Attr[]= {/*记录attribute，用于通过wifi上传*/
    {0x000c0000, 0xFF},
    {0x000c0005, 0xFF},
    {0x000c0008, 0xFF},
    {0x000c0011, 0xFF},
    {0x000c0012, 0xFF},
    {0x000c0013, 0xFF},
    {0x000c1001, 0x00}
};

static COM_DEV_STRING_ATTR_STRUCT s_Com_Dev_String_Attr[]= {/*记录attribute，用于通过wifi上传*/
    {0x000c0009, "0xFF,0xFF"},
    {0x000c000c, "0xFF,0xFF"},
    {0x000c000d, "0xFF,0xFF"},
    {0x000c0010, "0xFF,0xFF"},
    {0x000c0015, "0xFF,0xFF"},
    {0x000c0016, "0xFF,0xFF"},
    {0x000c0014, "0xFF,0xFF"}
};

/*用于表明发送事件包的类型*/
enum
{
    WIFI_ACK_PACKAGE = 0,     /*WIFI模组ack包--被动*/
    WIFI_EVENT_PACKAGE,       /*WIFI模组事件包--主动*/
    WIFI_ACTION_PACKAGE,       /*WIFI模组动作包--主动*/
};

//=======================================================
/*定义锁体上报的状态声明*/
/*用于表明锁的状态*/
enum
{
    LOCK_CLOSE = 0,     /*关闭*/
    LOCK_OPEN,          /*打开*/
    LOCK_WARNING,       /*报警*/
    LOCK_RETAIN,        /*保留*/
    LOCK_REMIND,        /*提醒*/
};

/*报警时用于表明锁的数据类型*/
enum
{
    SMART_LOCK_IS_SMASHED= 0,   /*智能锁被撬*/
    FORCIBLY_OPEN_LOCK,         /*强行开门*/
    FINGERPRINT_IS_FROZEN,      /*指纹尝试开锁被冻结*/
    PASSWORD_IS_FROZEN,         /*密码尝试开锁被冻结*/
    CARD_IS_FROZEN,             /*卡尝试开锁被冻结*/
    KEY_IS_FROZEN,              /*钥匙尝试开锁被冻结*/
    REMOTE_CONTROL_IS_FROZEN,   /*遥控尝试开锁被冻结*/
    DATA_REMIND,                /*保留*/
    DURESS_UNLOCKING_ALARM,     /*胁迫开锁报警*/
};

/*门锁警告状态上报-Confiuence*/
enum
{
    LOCK_NORMAL= 0,             /*正常*/
    PASSWORD_ERROR_OVERFLOW,    /*密码连续试错超过上限*/
    VIOLENCE_OPEN_LOCK,         /*暴力开锁*/
    RESET_FACTORY_SETTING,      /*恢复出厂设置*/
    LOW_POWER_ALAEM=5,          /*低电报警*/
};

/*门锁设置状态上报-Confiuence*/
enum
{
    PASSWORD_EVENT= 0,    /*密码事件*/
    FINGERPRINT_EVENT,    /*指纹事件*/
    CARD_EVENT,           /*卡事件*/
};

/*提醒时用于表明锁的数据类型*/
enum
{
    FORGET_THE_KEY= 0,          /*忘拔钥匙*/
    LOCK_DOOR_REMINDER,         /*锁门提醒*/
    KNOCKING_DOOR_REMINDER,     /*敲门提醒*/
    SOS_HELP_REMINDER,          /*SOS求救提醒*/
    DOOR_IS_NOT_CLOSED,         /*门没关好*/
    DOOR_HAS_BEEN_LOCKED,       /*门已反锁*/
    DOOR_IS_UNLOCKED,           /*门已解锁*/
    NORMALLY_OPEN,              /*常开已开启*/
    LOW_BATTERY_REMINDER,       /*电量低压提醒*/
};

/*用于表明锁的用户类别*/
enum
{
    DEFAULT_USER = 0,           /*默认用户*/
    FINGERPRINT_USER,           /*指纹用户*/
    PASSWORD_USER,              /*密码用户*/
    CARD_USER,                  /*卡用户*/
    KEY_USER,                   /*钥匙用户*/
    MOBILE_PHONE_USER,          /*手机用户*/
};

/*用于表明布防/撤防*/
enum
{
    ARMING = 0,                 /*布防*/
    DISARMING = 1,              /*撤防*/
};

/*用于表明getfunction的list数组成员的编号*/
enum
{
    LIST_UNLOCK_ID     =0,           /*指纹/密码/卡片编号*/
    LIST_UNLOCK_TYPE   =1,           /*三种 fingerprint, password, card*/
    LIST_UNLOCK_OPERA  =2,           /*0:add ; 1:delete; 2: modify*/
    LIST_UNLOCK_VALUE  =3,           /*密码的value, 其他为空或者不传*/
};
/*用于表明getfunction的三种操作类型*/
#define  FINGERPRINT_OPERA  "fingerprint"
#define  PASSWORD_OPERA     "password"
#define  CARD_OPERA         "card"

/*用于表明getfunction的某种操作类型的具体操作*/
enum
{
    UNLOCK_ADD_OPERA        =0,     /*添加操作*/
    UNLOCK_DELETE_OPERA     =1,     /*删除操作*/
    UNLOCK_MODIFY_OPERA     =2,     /*修改操作*/
};

//=======================================================
#define WIFI_DOOR_LOCK_FUNCTION_KEY     (0x10000401)/*WIFI门锁的function key*/
#define RECV_QUEUE_BUF_SIZE             (128)       /*接收消息队列缓冲区大小*/
#define TIMEOUT_SEND_COUNT              (5)         /*超时发送次数*/
#define INITIATIVE_RESEND_TIMEOUT       (3 * 100)   /*软件定时器超时时间-300MS*/
#define WIFI_READY_RESEND_TIMEOUT       (10 * 100)  /*WIFI_ready超时时间-1000MS*/
#define MESSAGE_QUEUE_COUNT             (10)        /*申请消息队列的个数*/
#define BODY_PASSWORD_DATA_HEAD         (2)         /*body中储存密码信息的前几个字节,从body[2]开始是存储的是密码编号*/
#define GET_PASSWORD_TYPE_NUM           (3)         /*获取MCU中储存密码的类型个数:密码,指纹,IC卡*/
#define DELAY_TIME_GET_MCU_PASS         (30 * 100)  /*延时3000MS去获取mcu的密码*/
#define RECV_SERIAL_CODE_SIZE           (15)        /*接收串码数据的大小*/

UINT8 Seq_Number=0;                                 /*记录wifi主动向mcu发送的序包*/
UINT8 Mcu_Seq_Number=0;                             /*记录mcu主动向wifi发送的序包*/
static INT8 initiative_resend_count;                /*记录wifi主动向mcu发送的次数*/
UINT8 delay_time = 0x03;                            /*关闭电源请求，延时多长时间关闭*/
Message_Queue_Info_t* Wifi_Send_Mes_Queue_Info;     /*wifi模组发送数据消息队列，用于向数据处理任务发送数据*/
Message_Queue_Info_t* Uart_Data_Queue_Info;         /*串口发送数据消息队列，用于向数据处理任务发送数据*/

#define REPORT_CLOUD_COUNT              (10)        /*缓存数据上报的大小*/
FRAME_STRUCT* frame_info_buf[REPORT_CLOUD_COUNT]=NULL;/*缓存上报的数据*/
UINT8 frame_info_count = 0;                         /*记录缓存上报数据的个数*/
UINT8 frame_info_report_times = 0;                  /*记录缓存上报数据的次数*/
UINT8 wifi_connect_cloud_success = 0;               /*wifi模组成功连接到云端标志*/
UINT8 wifi_enter_softap = 0;                        /*wifi模组成功连接到云端标志*/
#ifdef ENADLE_GET_MCU_PASS_INFO
xSemaphoreHandle getpass_mutex;                     /*给0x89指令上报云端密码信息加锁,密码过多上报时间可能超过300ms,防止重复发送*/
#endif
UINT8 mcu_serial_code[RECV_SERIAL_CODE_SIZE] = {0}; /*保存mcu串码*/
UINT8 gadgetid[64]={0};                             /*记录设备的gadgetid*/

void _Wifi_Door_Lock_Data_Queue_Loading(UINT8 frame_type, UINT8 *body, UINT16 body_len);

/*****************************************************************************
**函 数 名: Get_Gadgetid_Function
**输入参数: UINT8* gadgetid
**输出参数: 无
**返 回 值: Get_Gadgetid_Function
**功能描述: 获取adgetid
**作     者: wqw
*****************************************************************************/
void Get_Gadgetid_Function(UINT8* arg)
{
    iots_strcpy(gadgetid,arg);
}

 /*****************************************************************************
 **函 数 名: Wifi_Door_Lock_Handle_GetGadgetFunction_MsgAck
 **输入参数: int isok, char *msg_type, cJSON *itemdata, void *param
 **输出参数: 无
 **返 回 值:
 **功能描述: get_function的回调函数
 **作     者: wqw
 *****************************************************************************/
 void Wifi_Door_Lock_Handle_GetGadgetFunction_MsgAck(int isok, char *msg_type, cJSON *itemdata, void *param)
 {
    cJSON *itemcode    = NULL;
    cJSON *functions   = NULL;
    cJSON *get_data    = NULL;
    cJSON *tmp         = NULL;
    cJSON *list        = NULL;
    cJSON *object      = NULL;
    cJSON *tmp1,*tmp2,*tmp3,*tmp4;
    UINT8 i;
    //UINT8 password_expiration_date[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    UINT8 body_tmp[2]  ={0};
    UINT8 password_body[14] ={0};
    UINT8 password_expiration_date[] ="123456";

    for(i=0;i<6;i++)
        log_debug0("password_expiration_date[%d]=0x%x\n",i,password_expiration_date[i]-'0');

    for(i=0;i<14;i++)
        log_debug0("3333333333333333333333333333333333333333password_body[%d]=0x%x\n",i,password_body[i]);

    itemcode = cJSON_GetObjectItem(itemdata, "code");
    log_debug0("get function code:%d\n",itemcode->valueint);

    if(!itemcode->valueint){//code值返回0-成功进行下面的操作
        functions = cJSON_GetObjectItem(itemdata, "functions");
        get_data =cJSON_GetObjectItem(functions,"gadget_id");
        log_debug0("get_data->valuestring:%s\n",get_data->valuestring);

        if(!strcmp(get_data->valuestring,gadgetid)){
            list=cJSON_GetObjectItem(get_data,"list");
            if( list != NULL)
            {
                log_debug0("list no data\n");
            }else{
                object = cJSON_GetArrayItem(list,0);//获取list[0]
                log_debug0("list数组里面的内容=%s\n",cJSON_Print(object));
                tmp = cJSON_GetObjectItem(object,"unlock_opera");
                log_debug0("unlock_opera:%d\n",tmp->valueint);
                switch(tmp->valueint){
                    case UNLOCK_ADD_OPERA:
                        tmp1 = cJSON_GetObjectItem(object,"unlock_type");
                        log_debug0("unlock_type:%d\n",tmp1->valueint);
                        if(!strcmp(tmp1->valuestring,PASSWORD_OPERA)){//密码类型
                            tmp2 = cJSON_GetObjectItem(object,"unlock_value");
                            log_debug0("unlock_value:%s\n",tmp2->valuestring);
                            tmp3 = cJSON_GetObjectItem(object,"unlock_id");
                            log_debug0("unlock_id:%d\n",tmp3->valueint);
                            password_body[0]=tmp3->valueint;
                            password_body[1]=0x00;//密码加密方法 0x00表示明文,0x01表示MD5的密文
                            iots_strncpy(&password_body[2],tmp2->valuestring,6);
                            iots_strncpy(&password_body[8],password_expiration_date,6);//密码有效截止时间,全0xff表示一直有效
                            _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_APPLICATION_ADD_PASSWORD_PACKAGE, password_body, 14);

                        }else{//指纹、IC卡类型

                        }
                        break;

                    case UNLOCK_DELETE_OPERA:
                        if(!strcmp(tmp->valuestring,PASSWORD_OPERA)){//密码类型       0x00 密码 0x01 指纹 0x02 IC 卡
                            body_tmp[0]=0;
                        }else if(!strcmp(tmp->valuestring,FINGERPRINT_OPERA)){
                            body_tmp[0]=1;
                        }else{
                            body_tmp[0]=2;
                        }
                        object = cJSON_GetArrayItem(list,LIST_UNLOCK_TYPE); //获取操作类型(指纹密码IC卡)
                        tmp = cJSON_GetObjectItem(object,"unlock_type");
                        log_debug0("unlock_type:%s\n",tmp->valuestring);

                        object = cJSON_GetArrayItem(list,LIST_UNLOCK_ID);
                        tmp = cJSON_GetObjectItem(object,"unlock_id");      //获取编号
                        log_debug0("unlock_id:%d\n",tmp->valueint);
                        body_tmp[1]=tmp->valueint;
                        _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_APPLICATION_DEL_PASSWORD_FINGERPRINT_IC_PACKAGE, body_tmp, 2);
                        break;

                    case UNLOCK_MODIFY_OPERA:
                        break;

                    default:
                        break;
                }

            }

        }
    }

    log_debug0("44444444444444444444444444444=====%s\n",(UINT8*)cJSON_Print(itemdata));

    cJSON_Delete(object);
    cJSON_Delete(tmp);
    cJSON_Delete(list);
    cJSON_Delete(functions);
    cJSON_Delete(get_data);
    cJSON_Delete(itemcode);
 }

 /*****************************************************************************
 **函 数 名: _Wifi_Door_Lock_Get_GadgetFunction_to_Cloud
 **输入参数: 无
 **输出参数: 无
 **返 回 值:
 **功能描述: get_function的接口
 **作     者: wqw
 *****************************************************************************/
 void _Wifi_Door_Lock_Get_GadgetFunction_to_Cloud(void)
 {
    cJSON *send_data_package = cJSON_CreateObject();

    if( send_data_package == NULL)
    {
        log_debug0("insufficient memory\n");
        return;
    }

    cJSON_AddStringToObject(send_data_package,"function_type","get_unlock_id_change");
    cJSON_AddStringToObject(send_data_package,"gadget_id",gadgetid);

    log_debug0("22222222222222222222222222222222222222222222 get_condition = %s\n",(UINT8*)cJSON_Print(send_data_package));

    IOTCloud_GetGadgetFunction(WIFI_DOOR_LOCK_FUNCTION_KEY,send_data_package,Wifi_Door_Lock_Handle_GetGadgetFunction_MsgAck,NULL);
    cJSON_Delete(send_data_package);
 }

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Enter_Softap
**输入参数: 无
**输出参数: 无
**返 回 值: wifi进入softap模式的标志
**功能描述: wifi进入softap模式
**作     者: wqw
*****************************************************************************/
UINT8 Wifi_Door_Lock_Enter_Softap(void)
{
    return wifi_enter_softap;
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Print_Frame
**输入参数: FRAME_STRUCT *frame:帧内存地址
**输出参数: 无
**返 回 值:
**功能描述: 将帧内容输出到log
**作     者: wqw
*****************************************************************************/
static void _Wifi_Door_Lock_Print_Frame(FRAME_STRUCT *frame,UINT8 frame_body_len)
{
    UINT8 i = 0;
    UINT8 *ptr = frame->body;

    log_debug0("Frame head:%04x\n", ENDIAN_SWITCH(frame->magic_number));
    log_debug0("Frame type:%02x\n", frame->frame_type);

    switch(frame->frame_type)
    {
        /*以下为MCU--->WIFI*/
        case MCU_ACK_PACKAGE:
        case MCU_MODULE_START_INFO:
        case MCU_TIME_SERVICE_INFO:
        case MCU_REPORT_DOOR_LOCK_INFO:
        case MCU_RESET_COMMAND_INFO:
        case MCU_DOOR_LOCK_DEPLOY_WITHDRAW_INFO:
        case MCU_DOOR_LOCK_REPORT_SETTING_INFO:
        case MCU_DOOR_LOCK_REPORT_ELECTRIC_QUANTITY_INFO:
        case MCU_DOOR_LOCK_REPORT_SERIAL_CODE_INFO:
        case MCU_DOOR_LOCK_REPORT_ADD_INFO:
        case MCU_DOOR_LOCK_REPORT_DELETE_INFO:
        case MCU_REVERSE_TIME_INFO:
        {
            log_debug0("Frame crc:%02x\n", frame->crc);
            log_debug0("Frame len:%02x\n", frame->length);
            log_debug0("Frame seq:%02x\n", frame->sequence_number);

            if(0 != frame_body_len)
            {
                INT8 *temp_buf = NULL;

                temp_buf = malloc(frame_body_len * 3 + 1);
                temp_buf[frame_body_len] = 0;
                if(NULL == temp_buf)
                {
                    log_debug0("Wifi_Door_Lock print malloc fail\n");
                    break;
                }
                for(i = 0; i < frame_body_len; i++)
                {
                    sprintf(temp_buf + i * 3, "%02x ", ptr[i]);
                }
                log_debug0("Frame body:%s\n", temp_buf);
                free(temp_buf);
            }
            else{
                log_debug0("Frame body is NULL\n");
            }
        }
        break;
        /*以下为WIFI--->MCU*/
        case DEVICE_ACK_PACKAGE:
        case DEVICE_TIME_SERVICE_PACKAGE:
        case DEVICE_TURN_OFF_POWER_PACKAGE:
        case DEVICE_OTA_APPLICATION_PACKAGE:
        case DEVICE_WIFI_STATUS_REPORT_PACKAGE:
        case DEVICE_GET_DOOR_LOCK_SETTING_INFO_PACKAGE:
        case DEVICE_REQUEST_ELECTRIC_QUANTITY_PACKAGE:
        case DEVICE_REQUEST_SERIAL_CODE_PACKAGE:
        case DEVICE_REVERSE_TIME_PACKAGE:
        case DEVICE_APPLICATION_DEPLOY_WITHDRAW_PACKAGE:
        case DEVICE_APPLICATION_ADD_PASSWORD_PACKAGE:
        case DEVICE_APPLICATION_DEL_PASSWORD_FINGERPRINT_IC_PACKAGE:
        case DEVICE_APPLICATION_TIME_START_WIFI_PACKAGE:
        {
            INT8 *temp_buf = NULL;
            UINT16 real_len = frame_body_len;

            log_debug0("Frame crc:%02x\n", frame->crc);
            log_debug0("Frame len:%02x\n", frame->length);
            log_debug0("Frame seq:%02x\n", frame->sequence_number);

            if(0 != real_len)
            {
                temp_buf = malloc(real_len * 3 + 1);
                temp_buf[real_len] = 0;
                if(NULL == temp_buf)
                {
                    log_debug0("Wifi_Door_Lock print malloc fail\n");
                    break;
                }
                for(i = 0; i < real_len; i++)
                {
                    sprintf(temp_buf + i * 3, "%02x ", ptr[i]);
                }
                log_debug0("Frame Body:%s\n", temp_buf);
                free(temp_buf);
            }
            else{
                log_debug0("Frame body is NULL\n");
            }
        }
            break;
        default:
            break;
    }

}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Get_SERIAL_CODE_From_MCU
**输入参数: 无
**输出参数: 无
**返 回 值: serial_code
**功能描述: 获取mcu的串码
**作     者: wqw
*****************************************************************************/
UINT8* Wifi_Door_Lock_Get_SERIAL_CODE_From_MCU(void)
{
    return mcu_serial_code;
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Find_Int_Attribute_Index
**输入参数: const u8 id:属性ID
**输出参数: 无
**返 回 值: static
**功能描述: 查找对应的属性ID
**作     者: wqw
*****************************************************************************/
INT32 _Wifi_Door_Lock_Find_Int_Attribute_Index(UINT32 id)
{
    UINT32 i = 0;

    for(i = 0; i <sizeof(s_Com_Dev_Int_Attr)/sizeof(COM_DEV_INT_ATTR_STRUCT); i++)
    {
        if(s_Com_Dev_Int_Attr[i].id == id)
        {
            return i;
        }
    }

    log_debug0("FindAttributeIndex Error id = %2X\n", id);
    return -1;
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Find_String_Attribute_Index
**输入参数: const u8 id:属性ID
**输出参数: 无
**返 回 值: static
**功能描述: 查找对应的属性ID
**作     者: wqw
*****************************************************************************/
INT32 _Wifi_Door_Lock_Find_String_Attribute_Index(UINT32 id)
{
    UINT32 i = 0;

    for(i = 0; i < sizeof(s_Com_Dev_String_Attr)/sizeof(COM_DEV_STRING_ATTR_STRUCT); i++)
    {
        if(s_Com_Dev_String_Attr[i].id == id)
        {
            return i;
        }
    }

    log_debug0("FindAttributeIndex Error id = %2X\n", id);
    return -1;
}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Get_Int_Attribute
**输入参数: UINT32 attribute_id:属性ID
**输出参数: 无
**返 回 值:
**功能描述: 根据属性ID获取存储的属性信息，用于通过WIFI上传到云
**作     者: wqw
*****************************************************************************/
UINT32 Wifi_Door_Lock_Get_Int_Attribute(UINT32 attribute_id)
{
    UINT32 id = attribute_id;
    int len = sizeof(s_Com_Dev_Int_Attr)/sizeof(COM_DEV_INT_ATTR_STRUCT);
    int i = 0;

    for(i = 0; i < len ; i++)
    {
        if(s_Com_Dev_Int_Attr[i].id == id)
        {
            return s_Com_Dev_Int_Attr[i].value;
        }
    }
    return 0;
}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Get_String_Attribute
**输入参数: UINT32 attribute_id:属性ID
**输出参数: 无
**返 回 值:
**功能描述: 根据属性ID获取存储的属性信息，用于通过WIF                I上传到云
**作     者: wqw
*****************************************************************************/
UINT8* Wifi_Door_Lock_Get_String_Attribute(UINT32 attribute_id)
{
    UINT32 id = attribute_id;
    int len = sizeof(s_Com_Dev_String_Attr)/sizeof(COM_DEV_STRING_ATTR_STRUCT);
    int i = 0;

    for(i = 0; i < len ; i++)
    {
        if(s_Com_Dev_String_Attr[i].id == id)
        {
            return s_Com_Dev_String_Attr[i].value;
        }
    }

    return NULL;
}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_String_Packing
**输入参数: UINT8 a,UINT8 b
**输出参数: 拼接后的buf
**返 回 值: 上报类型是字符串的状态值
**功能描述: 将上报属性是字符串的值进行字符串拼接,
**作     者: wqw
*****************************************************************************/
UINT8* _Wifi_Door_Lock_String_Concatenation(UINT8 a,UINT8 b)
{
    static UINT8 tmp_buf[16]={0};

    sprintf(tmp_buf,"%d,%d",a,b);

    return tmp_buf;
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Deal_Attributes_Report_Cloud
**输入参数: FRAME_STRUCT* frame_info
**输出参数: 无
**返 回 值:
**功能描述: 处理从MCU收到的属性包信息
**作     者: wqw
*****************************************************************************/
void _Wifi_Door_Lock_Deal_Attributes_Report_Cloud(FRAME_STRUCT* frame_info)
{
    UINT32 idx;
    UINT8 count=0;
    UINT8 password_bytes_num=frame_info->length-PROTOCOL_HEAD_LENGTH-BODY_PASSWORD_DATA_HEAD;
    UINT8 subscript=0;


    if(TYPE_EVENT_REPORT_LOCK_STATUS==frame_info->frame_type)
    {
        if(LOCK_CLOSE==frame_info->body[0])/*关闭*/
        {
            switch(frame_info->body[2])
            {
                case DEFAULT_USER:
                case FINGERPRINT_USER:
                case PASSWORD_USER:
                case CARD_USER:
                case KEY_USER:
                case MOBILE_PHONE_USER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_OFFON);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_OFFON;
                    s_Com_Dev_Int_Attr[idx].value=LOCK_CLOSE;
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_OFFON, ASYNC_NOT_UPDATE_FLASH);
                    break;

                default:
                    break;
            }
        }else if(LOCK_OPEN==frame_info->body[0]){/*打开*/
            idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_OFFON);
            s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_OFFON;
            s_Com_Dev_Int_Attr[idx].value=LOCK_OPEN;
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_OFFON, ASYNC_NOT_UPDATE_FLASH);

            switch(frame_info->body[2])
            {
                case DEFAULT_USER:

                    break;

                case FINGERPRINT_USER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT;//01 00 01 00 01 大端存储
                    s_Com_Dev_Int_Attr[idx].value=((frame_info->body[3]<<8 |frame_info->body[4])+1);
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case PASSWORD_USER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_PASSWORD);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_PASSWORD;
                    s_Com_Dev_Int_Attr[idx].value=((frame_info->body[3]<<8 |frame_info->body[4])+1);
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_PASSWORD, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case CARD_USER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_CAR);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_CAR;
                    s_Com_Dev_Int_Attr[idx].value=((frame_info->body[3]<<8 |frame_info->body[4])+1);
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_CAR, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case KEY_USER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_KEY);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_KEY;
                    s_Com_Dev_Int_Attr[idx].value=LOCK_OPEN;
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_KEY, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case MOBILE_PHONE_USER:
                    //idx=_Wifi_Door_Lock_Find_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_KEY);
                    //s_Com_Dev_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_11;
                    //s_Com_Dev_Attr[idx].value=(frame_info->body[3]<<8 |frame_info->body[4]);
                    //sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_5, ASYNC_NOT_UPDATE_FLASH);
                    break;

                default:
                    break;
            }
        }else if(LOCK_WARNING==frame_info->body[0]){/*报警*/
            switch(frame_info->body[1])
            {
                case SMART_LOCK_IS_SMASHED:
                case FORCIBLY_OPEN_LOCK:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_WARNING);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_WARNING;
                    s_Com_Dev_Int_Attr[idx].value=VIOLENCE_OPEN_LOCK;
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_WARNING, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case FINGERPRINT_IS_FROZEN:
                case PASSWORD_IS_FROZEN:
                case CARD_IS_FROZEN:
                case KEY_IS_FROZEN:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_WARNING);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_WARNING;
                    s_Com_Dev_Int_Attr[idx].value=PASSWORD_ERROR_OVERFLOW;
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_WARNING, ASYNC_NOT_UPDATE_FLASH);
                    break;

                case REMOTE_CONTROL_IS_FROZEN:
                    break;

                case DATA_REMIND:
                    break;

                case DURESS_UNLOCKING_ALARM:
                    break;

                default:
                    break;
            }
        }else if(LOCK_RETAIN==frame_info->body[0]){/*保留*/

        }else if(LOCK_REMIND==frame_info->body[0]){/*提醒*/
            switch(frame_info->body[1])
            {
                case FORGET_THE_KEY:
                    break;

                case LOCK_DOOR_REMINDER:
                    break;

                case KNOCKING_DOOR_REMINDER:
                    break;

                case SOS_HELP_REMINDER:
                    break;

                case DOOR_IS_NOT_CLOSED:
                    break;

                case DOOR_HAS_BEEN_LOCKED:
                    break;

                case DOOR_IS_UNLOCKED:
                    break;

                case NORMALLY_OPEN:
                    break;

                case LOW_BATTERY_REMINDER:
                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_WARNING);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_WARNING;
                    s_Com_Dev_Int_Attr[idx].value=LOW_POWER_ALAEM;
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_WARNING, ASYNC_NOT_UPDATE_FLASH);

                    idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL);
                    s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL;
                    s_Com_Dev_Int_Attr[idx].value=frame_info->body[2];
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL, ASYNC_NOT_UPDATE_FLASH);
                    break;

                default:
                    break;
            }
        }
    }else if(TYPE_EVENT_DEPLOY_WITHDRAW_INFO==frame_info->frame_type){
        if(ARMING==frame_info->body[0])/*布防*/
        {
            //s_Com_Dev_Int_Attr[1].id=GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD;
            //s_Com_Dev_Int_Attr[1].value=(frame_info->body[2]);
            //sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD, ASYNC_NOT_UPDATE_FLASH);
        }else if(DISARMING==frame_info->body[0]){/*撤防*/
            //s_Com_Dev_Int_Attr[2].id=GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD;
            //s_Com_Dev_Int_Attr[2].value=(frame_info->body[2]);
            //sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD, ASYNC_NOT_UPDATE_FLASH);
        }
    }else if(TYPE_EVENT_REPORT_SETTING_INFO==frame_info->frame_type){
        if(PASSWORD_EVENT==frame_info->body[0])/*获取设置密码*/
        {
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD;

            for(count=0;count<password_bytes_num*8;count++){//逐步分析每一个字节的每一位的值
                if((frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript])&0x01){
                    s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(count,0);
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD, ASYNC_NOT_UPDATE_FLASH);
                }

                frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]=frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]>>1;

                if(!((count+1)%8)){
                    subscript++;
                }
            }

        }else if(FINGERPRINT_EVENT==frame_info->body[0]){/*获取设置指纹*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT;

            for(count=0;count<password_bytes_num*8;count++){//逐步分析每一个字节的每一位的值
                if((frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript])&0x01){
                    s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(count,0);
                    sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT, ASYNC_NOT_UPDATE_FLASH);
                }

                frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]=frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]>>1;

                if(!((count+1)%8)){
                    subscript++;
                }
            }

        }else if(CARD_EVENT==frame_info->body[0]){/*获取设置卡*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_CARD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_CARD;

            for(count=0;count<password_bytes_num*8;count++){//逐步分析每一个字节的每一位的值
            if((frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript])&0x01){
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(count,0);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_CARD, ASYNC_NOT_UPDATE_FLASH);
            }

            frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]=frame_info->body[BODY_PASSWORD_DATA_HEAD+subscript]>>1;

            if(!((count+1)%8)){
                subscript++;
            }
        }
#ifdef ENADLE_GET_MCU_PASS_INFO
            vSemaphoreDelete(getpass_mutex);
#endif
        }
    }else if(TYPE_EVENT_RESET==frame_info->frame_type){/*恢复出厂设置*/
        idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_WARNING);
        s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_WARNING;
        s_Com_Dev_Int_Attr[idx].value=RESET_FACTORY_SETTING;
        sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_WARNING, ASYNC_NOT_UPDATE_FLASH);
    }else if(TYPE_EVENT_REPORT_ELECTRIC_QUANTITY_INFO==frame_info->frame_type){/*电池电量上报*/
        idx=_Wifi_Door_Lock_Find_Int_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL);
        s_Com_Dev_Int_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL;
        s_Com_Dev_Int_Attr[idx].value=frame_info->body[0];
        sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL, ASYNC_NOT_UPDATE_FLASH);
    }else if(TYPE_EVENT_REPORT_ADD_INFO==frame_info->frame_type){/*MUC添加密码指纹IC卡上报*/
        if(PASSWORD_EVENT==frame_info->body[0])/*获取添加密码编号*/
        {
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD;//00 02 00//小端存储
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD, ASYNC_NOT_UPDATE_FLASH);

        }else if(FINGERPRINT_EVENT==frame_info->body[0]){/*获取添加指纹编号*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT;
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT, ASYNC_NOT_UPDATE_FLASH);
        }else if(CARD_EVENT==frame_info->body[0]){/*获取添加卡编号*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_SET_CARD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_SET_CARD;
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SET_CARD, ASYNC_NOT_UPDATE_FLASH);
        }
    }else if(TYPE_EVENT_REPORT_DELETE_INFO==frame_info->frame_type){/*MUC删除密码指纹IC卡上报*/
        if(PASSWORD_EVENT==frame_info->body[0])/*删除密码编号*/
        {
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD;//00 02 00//小端存储
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD, ASYNC_NOT_UPDATE_FLASH);

        }else if(FINGERPRINT_EVENT==frame_info->body[0]){/*删除指纹编号*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT;
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT, ASYNC_NOT_UPDATE_FLASH);
        }else if(CARD_EVENT==frame_info->body[0]){/*删除IC卡编号*/
            idx=_Wifi_Door_Lock_Find_String_Attribute_Index(GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD);
            s_Com_Dev_String_Attr[idx].id=GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD;
            s_Com_Dev_String_Attr[idx].value=_Wifi_Door_Lock_String_Concatenation(((frame_info->body[2]<<8 | frame_info->body[1])+1),frame_info->body[3]);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD, ASYNC_NOT_UPDATE_FLASH);
        }
    }else if(TYPE_EVENT_REPORT_SERIAL_CODE_INFO==frame_info->frame_type){/*获取门锁串码上报*/
        log_debug0("frame_info->body=%s\n",frame_info->body);
        iots_strncpy(mcu_serial_code,frame_info->body,RECV_SERIAL_CODE_SIZE);
        log_debug0("mcu_serial_code=%s\n",mcu_serial_code);
        sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_SERIAL_CODE, ASYNC_NOT_UPDATE_FLASH);
    }
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Cache_Attributes_Report_Cloud
**输入参数: 无
**输出参数: 无
**返 回 值:
**功能描述: 连云成功,进行属性上报
**作     者: wqw
*****************************************************************************/

void _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(FRAME_STRUCT* frame_info)
{
    UINT8 i;
    UINT8 max_num;
    UINT8 body_len=0;

    if(REPORT_CLOUD_COUNT == frame_info_count)
    {
        frame_info_count=0;
        frame_info_report_times=REPORT_CLOUD_COUNT;
    }

    if(NULL!=frame_info){
        body_len=frame_info->length-PROTOCOL_HEAD_LENGTH;
        frame_info_buf[frame_info_count] = (FRAME_STRUCT*)PROTOCOL_MALLOC(sizeof(FRAME_STRUCT));
        if(NULL == frame_info_buf[frame_info_count])
        {
            PROTOCOL_DEBUG_PRINTF("Frame malloc failed\n");
            return;
        }
        if(body_len!=0){
            frame_info_buf[frame_info_count]->body = (UINT8*)PROTOCOL_MALLOC(body_len);
            if(NULL == frame_info_buf[frame_info_count]->body)
            {
                PROTOCOL_DEBUG_PRINTF("data_recv->frame->body---Frame malloc failed\n");
                return;
            }
        }
        frame_info_buf[frame_info_count]->frame_type = frame_info->frame_type;
        frame_info_buf[frame_info_count]->length = frame_info->length;
        PROTOCOL_MEMCPY(frame_info_buf[frame_info_count]->body, frame_info->body, body_len);
        frame_info_count++;
    }

    max_num = (frame_info_report_times > frame_info_count) ? frame_info_report_times : frame_info_count;

    if(wifi_connect_cloud_success){
        for(i=0;i<max_num;i++){
            _Wifi_Door_Lock_Deal_Attributes_Report_Cloud(frame_info_buf[i]);
            PROTOCOL_FREE(frame_info_buf[i]->body);
            PROTOCOL_FREE(frame_info_buf[i]);
            frame_info_buf[i] = NULL;
        }
        frame_info_count=0;
        frame_info_report_times=0;
    }
}

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Send_Frame
**输入参数: FRAME_STRUCT *frame:帧地址
**UINT8 *body 帧内容
**UINT8 body_len 帧长度
**UINT8 event_type  0:ACK包，1:事件包,2:动作包
**输出参数: 无
**返 回 值:
**功能描述: 发送并释放帧空间
**作     者: wqw
*****************************************************************************/
static void _Wifi_Door_Lock_Send_Frame(UINT8 frame_type, UINT8 *body, UINT8 body_len,UINT8 event_type)
{
    static UINT8 Sequence_Number = 0; /*发送帧序号，每发送一次加1，超出0xFF自动归0*/
    FRAME_STRUCT *frame = NULL;
    UINT8 *psend_buf=NULL;

    frame = (FRAME_STRUCT*)PROTOCOL_MALLOC(sizeof(FRAME_STRUCT));
    if(NULL == frame)
    {
        PROTOCOL_DEBUG_PRINTF("Frame malloc failed\n");
        return;
    }
    if(body_len!=0){
        frame->body = (UINT8*)PROTOCOL_MALLOC(body_len);
        if(NULL == frame->body)
        {
            PROTOCOL_DEBUG_PRINTF("data_recv->frame->body---Frame malloc failed\n");
            return;
        }
        PROTOCOL_DEBUG_PRINTF("_Wifi_Door_Lock_Send_Frame frame->body len=%d\n",body_len);
    }else{
        PROTOCOL_DEBUG_PRINTF("_Wifi_Door_Lock_Send_Frame frame->body len=%d\n",body_len);
    }

    if(WIFI_ACK_PACKAGE==event_type)//被动发送数据,包序号和mcu的保持一致
    {
        psend_buf=Protocol_Package_Frame(frame, frame_type, body, body_len,Mcu_Seq_Number);
    }else if(WIFI_EVENT_PACKAGE==event_type){    //主动发送数据，序号包自动加1
        PROTOCOL_DEBUG_PRINTF("Sequence_Number=%d\n",Sequence_Number);
        psend_buf=Protocol_Package_Frame(frame, frame_type, body, body_len,Sequence_Number);
        Seq_Number=Sequence_Number;
        Sequence_Number++;
    }
    PROTOCOL_DEBUG_PRINTF("frame->sequence_number=0x%x\n",Sequence_Number,frame->sequence_number);

    Uart_Send(psend_buf, body_len + PROTOCOL_HEAD_LENGTH);
    Protocol_Free_Frame(psend_buf);
    _Wifi_Door_Lock_Print_Frame(frame,body_len);
    Protocol_Free_Frame(frame->body);
    Protocol_Free_Frame(frame);
    frame = NULL;
}

/*****************************************************************************
**
**wifiqueue
**
*****************************************************************************/

/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Data_Queue_Loading
**输入参数:
**输出参数: 无
**返 回 值:
**功能描述: 主动向MCU发送事件消息帧，--queue装载
**作     者: wqw
*****************************************************************************/
static void _Wifi_Door_Lock_Data_Queue_Loading(UINT8 frame_type, UINT8 *body, UINT16 body_len)
{
    UINT8 queue_send_buf[RECV_QUEUE_BUF_SIZE]={0};
    queue_send_buf[0]=frame_type;
    queue_send_buf[1]=body_len;
    memcpy(&queue_send_buf[2],body,body_len);

    if(pdTRUE!=Message_Queue_QueueSend(Wifi_Send_Mes_Queue_Info->message_queue_handle_t,queue_send_buf,0)){
        log_debug0("mes_queue_handle--xQueueSend error\n");
    }else{
        log_debug0("xQueueSend ok--Wifi_Send_Mes_Queue_Info->message_queue_handle_t=%d\n",Wifi_Send_Mes_Queue_Info->message_queue_handle_t);
    }
}

/*****************************************************************************
**函 数 名: Wifi_Consume_Queue_Data_Handle
**输入参数: void*arg1,void*arg2
**          arg1:接收数据,arg2:队列句柄
**输出参数:
**返 回 值:
**功能描述: wifi主动下发数据处理任务,处理从queue收到wif-mcu的数据--queue消费
**作     者: wqw
*****************************************************************************/
void Wifi_Consume_Queue_Data_Handle(void*arg1,void*arg2)
{
    PROTOCOL_DEBUG_PRINTF("enter Queue_Data_CB \n");
    UINT8 receive_quent_data[RECV_QUEUE_BUF_SIZE]={0};
    UINT8 *p=(UINT8*)arg1;
    UINT32 queue_handle=(UINT32)arg2;

    memset(receive_quent_data, 0, sizeof(receive_quent_data));
    memcpy(receive_quent_data, p, sizeof(receive_quent_data));

    PROTOCOL_DEBUG_PRINTF("Queue_Data_CB_queue_handle=%d---receive_quent_data[0]=0x%x\n",queue_handle,receive_quent_data[0]);

    initiative_resend_count=TIMEOUT_SEND_COUNT;
    while(initiative_resend_count > 0)
    {
        PROTOCOL_DEBUG_PRINTF("initiative_resend_count=%d\n",initiative_resend_count);
        switch(receive_quent_data[0])
        {
            case DEVICE_READY_PACKAGE:/*该数据包用于 WiFi 模组向 MCU 请求授时,收到指令81,然后mcu发送数据包*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(WIFI_READY_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_TURN_OFF_POWER_PACKAGE:/*该数据包用于 WiFi 模组告诉 MCU 可以断电了*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(WIFI_READY_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_OTA_APPLICATION_PACKAGE:/*该数据包用于WiFi模组告诉MCU当前准备进行OTA操作请180秒后才做强制断电*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_WIFI_STATUS_REPORT_PACKAGE:/*该数据包用于 WiFi 模组将自己的状态给到MCU,MCU可以控制喇叭播放出来*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_GET_DOOR_LOCK_SETTING_INFO_PACKAGE:/*该数据包是 WiFi 模组发送命令要求 MCU 上报门锁当前设置*/
#ifdef ENADLE_GET_MCU_PASS_INFO
                xSemaphoreTake(getpass_mutex, portMAX_DELAY);
#endif
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
#ifdef ENADLE_GET_MCU_PASS_INFO
                xSemaphoreGive(getpass_mutex);
#endif
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_REQUEST_ELECTRIC_QUANTITY_PACKAGE:/*该数据包用于 WiFi 模组告诉 MCU 获取电池电量了*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_REQUEST_SERIAL_CODE_PACKAGE:/*该数据包用于 WiFi 模组告诉 MCU 获取串码了*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

#ifndef Wifi_Door_Lock_Open_Ignore_Event
            case DEVICE_REVERSE_TIME_PACKAGE:/*该数据包用于 WiFi 模组向 MCU 请求授时,收到指令83,然后mcu发送数据包*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_APPLICATION_DEPLOY_WITHDRAW_PACKAGE:/*WiFi 模组申请布防/撤防数据包*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_APPLICATION_ADD_PASSWORD_PACKAGE:/*WiFi 模组申请添加密码*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_APPLICATION_DEL_PASSWORD_FINGERPRINT_IC_PACKAGE:/*WiFi 模组申请删除密码/指纹/IC 卡*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;

            case DEVICE_APPLICATION_TIME_START_WIFI_PACKAGE:/*WiFi 模组申请定时启动 WiFi 模组*/
                _Wifi_Door_Lock_Send_Frame(receive_quent_data[0], &receive_quent_data[2], receive_quent_data[1],WIFI_EVENT_PACKAGE);
                vTaskDelay(INITIATIVE_RESEND_TIMEOUT/portTICK_RATE_MS);
                break;
#endif
            default:
                log_debug0("fram-type-receive_quent_data[0]=0x%x\n",receive_quent_data[0]);
                break;
        }
        initiative_resend_count--;
    }
}

/*****************************************************************************
**
**串口queue
**
*****************************************************************************/
/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu
**输入参数: FRAME_STRUCT* frame_info:帧内容
**输出参数: 无
**返 回 值: static
**功能描述: 处理从MCU收到的事件
**作	  者: wqw
*****************************************************************************/
static void _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(FRAME_STRUCT* frame_info)
{
    log_debug0("Recv MCU event, event type:0x%x\n", frame_info->frame_type);

    UINT8* buff = frame_info->body;

    switch(frame_info->frame_type)
    {
        case TYPE_EVENT_MODULE_START:
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL,0,WIFI_ACK_PACKAGE);
            break;

        case TYPE_EVENT_REQ_TIME:   /*发送时间*/
        {
            struct tm *utc_time = NULL;
            time_t sec_time = 0;
            UINT16 year = 0;
            INT32 relative_year = 0;
            UINT8 temp[7] = {0};

            if(IOTSysP_SntpAsync()){
                temp[0] = 0x01;         //错误值， 0 表示本次授时有效， 1 表示本次授时无效
            }else{
                utc_time = (struct tm *)IOTSys_localtime(NULL);
                log_debug0("TYPE_EVENT_REQ_TIME--utc_time->tm_year=%d\n",utc_time->tm_year);
                year = utc_time->tm_year;
                log_debug0("TYPE_EVENT_REQ_TIME--year=%d\n",year);
                relative_year=year-2000;

                temp[0] = 0x00;         //错误值， 0 表示本次授时有效， 1 表示本次授时无效
                temp[1] = relative_year;
                temp[2] = utc_time->tm_mon ;
                temp[3] = utc_time->tm_mday;
                temp[4] = utc_time->tm_hour;
                temp[5] = utc_time->tm_min;
                temp[6] = utc_time->tm_sec;

                log_debug0("temp[1]:0x%x,temp[2]:0x%x,temp[3]:0x%x,temp[4]:0x%x,temp[5]:0x%x,temp[6]:0x%x\n", temp[1],temp[2], temp[3],
                temp[4], temp[5], temp[6]);

                log_debug0("Y:%d,M:%d,D:%d,H:%d,M:%d,S:%d\n", year, utc_time->tm_mon, utc_time->tm_mday,
                utc_time->tm_hour, utc_time->tm_min, utc_time->tm_sec);
            }
            _Wifi_Door_Lock_Send_Frame(DEVICE_TIME_SERVICE_PACKAGE, temp,7,WIFI_ACK_PACKAGE);
        }
            break;

        case TYPE_EVENT_REPORT_LOCK_STATUS:
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

        case TYPE_EVENT_RESET:
            if(frame_info->body[0]){
                _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
                _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
                IOTWifi_Reset();

            }else{
                _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
                IOTWifi_Reset();
                IOTDM_Exit();
                wifi_enter_softap=1;
            }
            break;

        case TYPE_EVENT_DEPLOY_WITHDRAW_INFO:
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL,0,WIFI_ACK_PACKAGE);
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

        case TYPE_EVENT_REPORT_SETTING_INFO://wifi主动向mcu发送0x89指令，mcu回复0x09
            //将锁的状态发送到云端
#ifdef ENADLE_GET_MCU_PASS_INFO
            xSemaphoreTake(getpass_mutex, portMAX_DELAY);
#endif
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);

#ifdef ENADLE_GET_MCU_PASS_INFO
            xSemaphoreGive(getpass_mutex);
#endif
            break;

        case TYPE_EVENT_REPORT_ELECTRIC_QUANTITY_INFO://wifi主动向mcu发送0x8f指令,mcu回复0x00,之后mcu会发送0x0f指令,wifi回复0x80
            //将电池电量发送到云端
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

        case TYPE_EVENT_REPORT_SERIAL_CODE_INFO://wifi主动向mcu发送0x90指令,mcu回复0x10,获取门锁的串码
            //将门锁串码发送到云端
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

        case TYPE_EVENT_REPORT_ADD_INFO:
            //将MCU添加密码指纹IC卡信息发送到云端
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

        case TYPE_EVENT_REPORT_DELETE_INFO:
            //将MCU删除密码指纹IC卡信息发送到云端
            _Wifi_Door_Lock_Send_Frame(DEVICE_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(frame_info);
            break;

#ifndef Wifi_Door_Lock_Open_Ignore_Event
        case TYPE_EVENT_GET_TIME:/*处理从MCU获取的时间*/
        {
            struct tm *utc_time_temp = NULL;
            time_t sec_time_temp = 0;
            UINT32 temp = 0;

            if(frame_info->body[0]==0){
                utc_time_temp->tm_year=frame_info->body[1];
                utc_time_temp->tm_mon=frame_info->body[2];
                utc_time_temp->tm_mday=frame_info->body[3];
                utc_time_temp->tm_hour=frame_info->body[4];
                utc_time_temp->tm_min=frame_info->body[5];
                utc_time_temp->tm_sec=frame_info->body[6];
                sec_time_temp=mktime(utc_time_temp);
                rtc_write(sec_time_temp);//Set RTC time
            }
            break;
        }
#endif
        default:
            break;
    }
}

/*****************************************************************************
**函 数 名: Uart_Free_Recv_Data
**输入参数: UART_RECV_DATA_STRUCT *uart_data
**输出参数: 无
**返 回 值:
**功能描述: 释放_Uart_Handle_Recv_Data()申请的空间
**作     者: wqw
*****************************************************************************/
static void _Uart_Free_Recv_Data(UINT8 *uart_data)
{
    PROTOCOL_DEBUG_PRINTF("UART free data:%p\n", uart_data);
    PROTOCOL_FREE(uart_data);
    uart_data =NULL;
}

/*****************************************************************************
**函 数 名: _Uart_Malloc_Recv_Data
**输入参数: UINT32 size
**输出参数: 无
**返 回 值: static
**功能描述: 为收到的数据分配空间，包括结构体和帧内容占用的空间
**作     者: wqw
*****************************************************************************/
static UINT8* _Uart_Malloc_Recv_Data(UINT32 size)
{
    PROTOCOL_PRINTF("enter _Uart_Malloc_Recv_Data -----------!!!");
     UINT8* uart_data = NULL;

    uart_data = PROTOCOL_MALLOC(size);
    if(NULL == uart_data)
    {
        PROTOCOL_FREE(uart_data);
        uart_data = NULL;
        PROTOCOL_PRINTF("data malloc fail\n");
        return NULL;
    }
    PROTOCOL_DEBUG_PRINTF("UART data malloc:%p\n", uart_data);

    return uart_data;
}

/*****************************************************************************
**函 数 名: _Uart_Data_Queue_Loading
**输入参数: UINT8 *buf:接收的数据缓冲区
            UINT32 size:接收的数据长度
**输出参数: 无
**返 回 值: static
**功能描述: 将数据送入消息队列
**作     者: wqw
*****************************************************************************/
static void _Uart_Data_Queue_Loading(UINT8 *buf, UINT32 size)
{
    PROTOCOL_PRINTF("enter _Uart_Data_Queue_Loading,buf[0]=0x%x--------size=%d!!!\n",buf[0],size);
    UINT8* uart_data = NULL;

    if((NULL == buf) || (0 == size))
    {
        PROTOCOL_PRINTF("_Uart_Handle_Recv_Data error------buf=%s---size=%d--!!!\n",buf,size);
        return;
    }

    uart_data = _Uart_Malloc_Recv_Data(size);
    if(NULL == uart_data)
    {
        PROTOCOL_PRINTF("Uart mem malloc fail\n");
        return;
    }
    memcpy(uart_data, buf, size);

    if(pdTRUE != Message_Queue_QueueSend(Uart_Data_Queue_Info->message_queue_handle_t,uart_data, 0))
    {
        log_debug0("Uart_Data_Queue_Info--xQueueSend error\n");
    }else{
        log_debug0("xQueueSend ok--Uart_Data_Queue_Info->message_queue_handle_t=%d\n",Uart_Data_Queue_Info->message_queue_handle_t);
    }
    _Uart_Free_Recv_Data(uart_data);
}

/*****************************************************************************
**函 数 名: MCU_To_WIFI_Data_CB
**输入参数: void*arg1,void*arg2
            arg1:接收数据，arg2:数据长度
**输出参数:
**返 回 值:
**功能描述: 数据处理任务，处理mcu发给wifi的数据放在queue中--queue装载
**作     者: wqw
*****************************************************************************/
void MCU_To_WIFI_Data_CB(void*arg1,void*arg2)
{
    PROTOCOL_DEBUG_PRINTF("(UINT32)arg2-len=0x%x\n",(UINT32)arg2);
    _Uart_Data_Queue_Loading((UINT8*)arg1,(UINT32)arg2);
}

/*****************************************************************************
**函 数 名: Uart_Consume_Queue_Data_Handle
**输入参数: void *pvParameters
**输出参数: 无
**返 回 值:
**功能描述: 数据处理任务，处理从queue收到MCU的数据--queue消费
**作     者: wqw
*****************************************************************************/
void Uart_Consume_Queue_Data_Handle(void*arg1,void*arg2)
{
    PROTOCOL_LIST_STRUCT *frame_head = NULL;    /*协议帧链表*/
    FRAME_STRUCT *frame_info = NULL;            /*协议帧内容*/
    UINT8 *uart_data=(UINT8*)arg1;              /*原始数据*/
    UINT8 body_len=0;
    UINT8 frame_len=0;
    UINT32 queue_handle=(UINT32)arg2;

    PROTOCOL_DEBUG_PRINTF("Uart_Queue_Data_CB_queue_handle=%d---uart_data[0]=0x%x\n",queue_handle,uart_data[0]);

    frame_len=uart_data[UART_DATA_LEN_BYTE_LOCATION];
    body_len=frame_len-PROTOCOL_HEAD_LENGTH;
    PROTOCOL_DEBUG_PRINTF("Recv frame from MCU, uart_data:%p, size:%d----body_len=%d\n", uart_data, frame_len,body_len);

    frame_head = Protocol_Package_Recv_Data(uart_data, frame_len & 0xFFFF);

    while(NULL != frame_head)
    {
        frame_info = frame_head->frame;
        _Wifi_Door_Lock_Print_Frame(frame_info,body_len);
        if(Protocol_Calc_Checksum(uart_data, frame_len) == frame_info->crc)
        {/*检测校验和*/
            log_debug0("Recv MCU data, frame type:0x%x--frame_info->length=%d\n", frame_info->frame_type,frame_info->length);
            Mcu_Seq_Number=frame_info->sequence_number;
            switch(frame_info->frame_type)
            {
                //case MCU_MODULE_START_INFO:                 /*确认模组是否启动*/
                //    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                //    break;

                case MCU_TIME_SERVICE_INFO:                 /* MCU向WiFi模组请求授时*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);

                    break;
                case MCU_REPORT_DOOR_LOCK_INFO:             /*MCU上报门锁状态*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_RESET_COMMAND_INFO:                /* MCU告诉WiFi模组当前进行复位需要进入待配置状态*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_DEPLOY_WITHDRAW_INFO:     /*该数据包是WiFi模组和MCU之间通信 设置/通知 布防/撤防-Deploy/withdraw*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_REPORT_SETTING_INFO:     /*MCU上报门锁的当前保存的状态到WIFI上*/
                    initiative_resend_count=0;
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_REPORT_ELECTRIC_QUANTITY_INFO:     /*MCU上报门锁的当前电池电量到WIFI上*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_REPORT_ADD_INFO:     /*MCU上报mcu添加密码指纹IC卡信息到WIFI上*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_REPORT_DELETE_INFO:     /*MCU上报mcu删除密码指纹IC卡信息到WIFI上*/
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                case MCU_DOOR_LOCK_REPORT_SERIAL_CODE_INFO:     /*MCU上报门锁串码到WIFI上*/
                    initiative_resend_count=0;
                    _Wifi_Door_Lock_Handle_Event_Frame_From_Mcu(frame_info);
                    break;

                //添加的指令,收到mcu的应答停止重发机制
                case MCU_ACK_PACKAGE:           /*MCU应答WIFI的指令*/
                case MCU_INVALID_ACK_PACKAGE:
                    log_debug0("Seq_Number=%d-----MCU_ACK_PACKAGE--frame_info->sequence_number=%d\n",Seq_Number,frame_info->sequence_number);
                    //完成任务释放信号量
                    initiative_resend_count=0;
                    break;//end MCU_ACK_PACKAGE:

                default:
                    log_debug0("Recv MCU data, frame type:0x%x\n", frame_info->frame_type);
                    _Wifi_Door_Lock_Send_Frame(DEVICE_INVALID_ACK_PACKAGE, NULL, 0,WIFI_ACK_PACKAGE);
                    break;
            }
        }
        log_debug0("frame_head:%p, next:%p\n",frame_head, frame_head->next);
        frame_head = Protocol_Get_Next_Frame(frame_head);
        log_debug0("frame_head:%p\n",frame_head);
    }
    log_debug0("55555555555555555555555555555555555555555--MIN FREE MEMORY: %d\n", IOTSysP_GetFreeHeapSize());
}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Send_IOT_Action_Frame_To_Mcu
**输入参数:     UINT32 attr_id:action ID
            UINT16 para :action 参数
**输出参数: 无
**返 回 值:
**功能描述: 向MCU发送action帧，用于操作WIFI门锁。
**作     者: wqw
*****************************************************************************/
void Wifi_Door_Lock_Send_IOT_Action_Frame_To_Mcu(UINT32 action_id, UINT16 para)
{
    UINT8 body[3] = {0};

    body[0] = action_id & 0xFF;
    body[1] = (para >> 8) & 0xFF;
    body[2] = para & 0xFF;

    //_Wifi_Door_Lock_Send_Frame(DEVICE_FUNC_PACKAGE, body, sizeof(body));
}
/*****************************************************************************
**函 数 名: _Wifi_Door_Lock_Get_MCU_Pass_Task
**输入参数: 无
**输出参数: 无
**返 回 值: 无
**功能描述: wifi模组连接云成功后向muc获取密码指纹IC卡编号上报云端
**作     者: wqw
*****************************************************************************/
#ifdef ENADLE_GET_MCU_PASS_INFO
void _Wifi_Door_Lock_Get_MCU_Pass_Task(void *arg)
{
    UINT8 i=0;

    //vTaskDelay(DELAY_TIME_GET_MCU_PASS/portTICK_RATE_MS);
    getpass_mutex= xSemaphoreCreateMutex();

    for(i=0;i<GET_PASSWORD_TYPE_NUM;i++){
        //_Wifi_Door_Lock_Data_Queue_Loading(DEVICE_GET_DOOR_LOCK_SETTING_INFO_PACKAGE, &i, 1);
    }
    vTaskDelete(NULL);
}
#endif

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu
**输入参数: UINT8 event_type
            INT8 state
**输出参数: 无
**返 回 值: Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu
**功能描述: 外部调用，向MCU发送联网状态-OTA升级/主动发送启动定时器
            无响应300ms再次发送
**作     者: wqw
*****************************************************************************/
void Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(UINT8 event_type, INT8 state)
{
    UINT8 result = NETWORK_CONFIGURE_DEVICE_STAR_SUCCESS;

    switch(event_type)
    {
        case IOTDM_EVENT_NETWORK_CONFIG:
            if(state == 1)//state==1, network NOT configed, start config  0
            {
                result = NETWORK_CONFIGURE_DEVICE_STAR_SUCCESS;
                 s_Network_Status = result;
                 _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state == 3){//network, receive ssid passwd   1
                result = NETWORK_CONFIGURE_SEND_SSID_PWD_SUCCESS;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state == -1){//network config password error, failed     4
                result = NETWORK_CONFIGURE_PWD_ERROR;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state == -2){//network config can't find special ssid, failed   3
                result = NETWORK_CONFIGURE_SSID_ERROR;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }
            break;

        case IOTDM_EVENT_NETWORK_CONNECT:
            if(state == 1)//state=1, network configured and is currently connected  2
            {
                result = NETWORK_CONFIGURE_CONNECT_ROUTER_SUCCESS;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state == -1){//state=-1, network connect failed  6
                result = NETWORK_NORMAL_CONNECT_ROUTER_ERROR;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }
            break;

        case IOTDM_EVENT_CLOUD_CONNECT:
            if(state == 1)//state=1, connected with the server  5
            {
                result = NETWORK_NORMAL_CLOUD_SUCCESS;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state == -1){//state=-1, connected with the server falied  7
                result = NETWORK_NORMAL_CONNECT_CLOUD_ERROR;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }
            break;

        case IOTDM_EVENT_AUTH:
            if(state < 0)//state<0, auth failed  8
            {
                result = NETWORK_NORMAL_CLOUD_AUTHORIZATION_ERROR;
                s_Network_Status = result;
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }
            break;

        case IOTDM_EVENT_OTA_STATUS://目前只用进行OTA申请的命令，没有升级成功失败的判断
            if(state == 1)//status=1, ota started
            {
                _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_OTA_APPLICATION_PACKAGE, NULL, 0);
            }else if(state == 0){//status=0, ota finished,success
                //_Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }else if(state < 0){//status<0, ota finished,failed
                //_Wifi_Door_Lock_Data_Queue_Loading(DEVICE_WIFI_STATUS_REPORT_PACKAGE, &result, 1);
            }
            break;

        case DEVICE_READY_PACKAGE://设备一上电，发送0x81与mcu通信
            _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_READY_PACKAGE, NULL, 0);
            break;

        case IOTDM_EVENT_CREATEGADGET://创建creategadget事件,进行缓存数据上报
            wifi_connect_cloud_success = state;
            _Wifi_Door_Lock_Cache_Attributes_Report_Cloud(NULL);
            _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_REQUEST_ELECTRIC_QUANTITY_PACKAGE, NULL, 0);//设备连云成功后获取mcu的电量
            _Wifi_Door_Lock_Data_Queue_Loading(DEVICE_REQUEST_SERIAL_CODE_PACKAGE, NULL, 0);//设备连云成功后获取mcu的串码
            _Wifi_Door_Lock_Get_GadgetFunction_to_Cloud();
#ifdef ENADLE_GET_MCU_PASS_INFO
            xTaskCreate(_Wifi_Door_Lock_Get_MCU_Pass_Task,"_Wifi_Door_Lock_Get_MCU_Pass_Task",512,NULL, 4,NULL);
#endif
            break;

        default:
            break;
    }
}

/*****************************************************************************
**函 数 名: Wifi_Door_Lock_Init
**输入参数: 无
**输出参数: 无
**返 回 值:
**功能描述: 初始化wifi门锁相关功能
**作     者: wqw
*****************************************************************************/
UINT8 Wifi_Door_Lock_Init(void)
{
    log_debug0("enter Wifi_Door_Lock_Init\n");

    if(UART_SUCCESS != Uart_Init(MCU_To_WIFI_Data_CB,LEN_UART_TX,LEN_UART_RX,LEN_BAUD_RATE,LEN_BIT_LEN,LEN_PARITY_MODE,LEN_STOP_BIT))
    {
        goto Wifi_Door_Lock_ERR;
    }

    Uart_Data_Queue_Info=Message_Queue_Create(Uart_Consume_Queue_Data_Handle,"Uart_Consume_Queue_task",MESSAGE_QUEUE_COUNT,RECV_QUEUE_BUF_SIZE);
    if(NULL == Uart_Data_Queue_Info->message_queue_handle_t)
    {
        goto Wifi_Door_Lock_ERR;
    }
    if(PROTOCOL_SUCCESS !=Message_StartLoop(Uart_Data_Queue_Info))
    {
        goto Wifi_Door_Lock_ERR;
    }

    if(PROTOCOL_SUCCESS != Protocol_Init())
    {
        goto Wifi_Door_Lock_ERR;
    }

    Wifi_Send_Mes_Queue_Info=Message_Queue_Create(Wifi_Consume_Queue_Data_Handle,"Wifi_Consume_Queue_task",MESSAGE_QUEUE_COUNT,RECV_QUEUE_BUF_SIZE);
    if(NULL == Wifi_Send_Mes_Queue_Info->message_queue_handle_t)
    {
        log_debug0("Queue was not created and must not be used.\n");
    }

    if(PROTOCOL_SUCCESS !=Message_StartLoop(Wifi_Send_Mes_Queue_Info))
    {
        goto Wifi_Door_Lock_ERR;
    }

    Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(DEVICE_READY_PACKAGE,0);

    return Wifi_Door_Lock_SUCCESS;
    Wifi_Door_Lock_ERR:
    return Wifi_Door_Lock_FAILURE;
}

