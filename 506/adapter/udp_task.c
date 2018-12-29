//Standard head file
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include <lwip/sockets.h>
#include "cJSON.h"
#include "queue.h"
//IOT SDK head
#include "iotsdk.h"
#include "datatype.h"
#include "security_func.h"
#include "log.h"
//Adapter head file
#include "udp_task.h"
#include "wifi_utils.h"
#include "version.h"
#include "product_config.h"

static UINT8 send_message_flag=1;
cJSON * cJSON_data,*cJSON_cmd_val;
UINT8 mac_add[16]={0};

xQueueHandle key_queue_handle;

int led_relay_onoff=0;
UINT8* get_equipment_mac(void)
{
    //wifi_get_macaddr(STATION_IF,yladdr);//查询MAC地址
    wifi_get_mac_address(mac_add);
    //sprintf(mac_add,MACSTR,MAC2STR(yladdr));
    return mac_add;
}
UINT8 factory_key_event_task_flag=0;
void SendKeyStatusToUDP(UINT8 key_status)
{
    log_debug2("get_key_status=%d----factory_key_event_task_flag=%d\n",key_status,factory_key_event_task_flag);
    if(factory_key_event_task_flag){
    if(xQueueSend(key_queue_handle,&key_status,0)!=pdTRUE)
        log_debug2("key_queue_handle1--xQueueSend error\n");
    }
}

void _udp_init(int *sockfd,UINT8 *prot_flag,struct sockaddr_in *my_con)
{
    log_debug0("udp_init_func---prot_flag=%d\n",*prot_flag);
    //创建socket
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        perror("socket error!\n");
    }
    //创建套接口
    memset(my_con,0,sizeof(struct sockaddr_in));
    my_con->sin_family = AF_INET;
    if(*prot_flag==1)
    {
        my_con->sin_port = htons(SEND_PORT);
        my_con->sin_addr.s_addr =  inet_addr(SEND_IP);
    }else{
        my_con->sin_port = htons(ListenPort);
        my_con->sin_addr.s_addr =  htonl(INADDR_ANY);//inet_addr
        //绑定套接口
        if(bind(*sockfd, (struct sockaddr *)my_con, sizeof(struct sockaddr_in)) < 0)
        {
            log_debug0("bind error!=%d\n",errno);
        }
        log_debug0("bind ok!\n");
    }
    log_debug0("_udp_init end!!!\n");
}
//接收json数据包进行解析
UINT8 json_analysis(UINT8 *p)
{
    cJSON_data= cJSON_Parse(p);
    cJSON_cmd_val=cJSON_GetObjectItem(cJSON_data,"cmd");
    return cJSON_cmd_val->valueint;
}
//创建json 对象
void json_create(UINT8* *buf,UINT8* ip,UINT8* adapter_ver,SDKInformation *broadcast_data)
{
    cJSON * root = cJSON_CreateObject();
    int rssi_val;
    wifi_get_rssi(&rssi_val);
    if(NULL == root)
    {
        printf("create json array faild\n");
    }
    cJSON_AddItemToObject(root, "module_type",cJSON_CreateString(broadcast_data->module_type));
    cJSON_AddItemToObject(root, "pn",cJSON_CreateString(broadcast_data->pn));
    cJSON_AddItemToObject(root, "gadget_type_id",cJSON_CreateNumber(broadcast_data->gadget_type_id));
    cJSON_AddItemToObject(root, "mac",cJSON_CreateString(get_equipment_mac()));
    cJSON_AddItemToObject(root, "ip",cJSON_CreateString(ip));
    cJSON_AddItemToObject(root, "sdk_ver",cJSON_CreateString(broadcast_data->sdk_version));
    cJSON_AddItemToObject(root, "adapter_ver",cJSON_CreateString(adapter_ver));
    cJSON_AddItemToObject(root, "cloud",cJSON_CreateString(broadcast_data->cloudstr));
    cJSON_AddItemToObject(root, "rssi",cJSON_CreateNumber(rssi_val));
    *buf =(UINT8*)cJSON_Print(root);
    cJSON_Delete(root);
}
//创建key_json 对象
void key_json_create(UINT8* *buf,UINT8 *str_buf,int led_relay_status)
//void key_json_create(UINT8* *buf,UINT8 *str_buf)
{
    cJSON * key_root = cJSON_CreateObject();
    if(NULL == key_root)
    {
        printf("create json array faild\n");
    }
    cJSON_AddItemToObject(key_root, "key_event",cJSON_CreateString(str_buf));
    if(strcmp(str_buf,"key action up")==0){
        cJSON_AddItemToObject(key_root, "relay_status",cJSON_CreateNumber(led_relay_status));
        cJSON_AddItemToObject(key_root, "led_status",cJSON_CreateNumber(led_relay_status));
    }
    *buf = (UINT8*)cJSON_Print(key_root);
    cJSON_Delete(key_root);
}
//创建relay_led_json 对象
void relay_led_json(UINT8* *buf,int led_relay_status,int wifi_led_status)
{
    cJSON * relay_led_root = cJSON_CreateObject();
    if(NULL == relay_led_root)
    {
        printf("create json array faild\n");
    }
    cJSON_AddItemToObject(relay_led_root, "relay_status",cJSON_CreateNumber(led_relay_status));
    cJSON_AddItemToObject(relay_led_root, "power_led_status",cJSON_CreateNumber(led_relay_status));
    cJSON_AddItemToObject(relay_led_root, "wifi_led_status",cJSON_CreateNumber(wifi_led_status));

    *buf = (UINT8*)cJSON_Print(relay_led_root);
    cJSON_Delete(relay_led_root);
}

void factory_key_event_task(void *arg)
{
    UINT8 prot_flag=1; //send 接口
    int sockfd,so_broadcast=1;
    struct sockaddr_in servaddr;
    static UINT8 key_flag=0;
    UINT8 key_event_flag=1;
    UINT8 *key_action;
    UINT8 key_action_down[]={"key action down"};
    UINT8 key_action_up[]={"key action up"};
    socklen_t addrlen = sizeof(struct sockaddr_in);
    _udp_init(&sockfd,&prot_flag,&servaddr);
    key_queue_handle = xQueueCreate(1, sizeof(char));
    if(key_queue_handle==0)
        log_debug0("Queue was not created and must not be used.");
    else{
        log_debug0("Queue was created success!!!");
        factory_key_event_task_flag=1;
    }

   while(1)
   {
        if(xQueueReceive(key_queue_handle,&key_event_flag,0)==pdTRUE)
        {
            if(key_event_flag)
            {
                key_flag=1;
                key_json_create(&key_action,key_action_down,led_relay_onoff);
                if(sendto(sockfd, key_action, strlen(key_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                    log_debug0("key_action send error!errno=%d\n",errno);
            }else if(!key_event_flag && key_flag){
                key_flag=0;
                key_json_create(&key_action,key_action_up,led_relay_onoff);
                if(sendto(sockfd, key_action, strlen(key_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                    log_debug0("key_action send error!errno=%d\n",errno);
            }
        }
       vTaskDelay(100/portTICK_RATE_MS);
   }
}
UINT8 factory_auto_function_mode=0;
void factory_mode_auto_task(void *arg)
{
    log_debug0("factory_model_auto_task....\n");
    UINT8 prot_flag=1; //send 接口
    int sockfd,so_broadcast=1;
    struct sockaddr_in servaddr;
    UINT8 *relay_led_action;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    _udp_init(&sockfd,&prot_flag,&servaddr);
    while(1)
    {
        if(factory_auto_function_mode)
        {
            relay_trigger(false);
            led_power_trigger(false);
            led_wifi_trigger(true);

            relay_led_json(&relay_led_action,false,true);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);

            vTaskDelay(1000/portTICK_RATE_MS);
            relay_trigger(true);
            led_power_trigger(false);
            led_wifi_trigger(true);

            relay_led_json(&relay_led_action,true,false);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);

            vTaskDelay(1000/portTICK_RATE_MS);
            relay_trigger(false);
            led_power_trigger(false);
            led_wifi_trigger(true);

            relay_led_json(&relay_led_action,false,true);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);

            vTaskDelay(1000/portTICK_RATE_MS);
            relay_trigger(true);
            led_power_trigger(true);
            led_wifi_trigger(false);

            relay_led_json(&relay_led_action,true,false);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);

            vTaskDelay(1000/portTICK_RATE_MS);
            relay_trigger(false);
            led_power_trigger(false);
            led_wifi_trigger(true);

            relay_led_json(&relay_led_action,false,true);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);

            vTaskDelay(1000/portTICK_RATE_MS);
            relay_trigger(true);
            led_power_trigger(true);
            led_wifi_trigger(false);
            led_relay_onoff=true;

            relay_led_json(&relay_led_action,true,false);
            if(sendto(sockfd, relay_led_action, strlen(relay_led_action), 0, (struct sockaddr *) &servaddr,addrlen)<0)
                log_debug0("key_action send error!errno=%d\n",errno);
            vTaskDelay(500/portTICK_RATE_MS);

            factory_auto_function_mode=0;
        }else{
            log_debug0(" wite factory_mode_auto_task \n");
            vTaskDelay(2000/portTICK_RATE_MS);
        }
    }
}

void _udp_receive(void)
{
    UINT8 prot_flag=0; //接收接口
    int sockfd,size;
    struct sockaddr_in my_addr;
    UINT8 receive_data[64]={0};
    socklen_t addrlen = sizeof(struct sockaddr_in);
    _udp_init(&sockfd,&prot_flag,&my_addr);

    while(1)
    {
        size=sizeof(my_addr);
        memset(receive_data,0,sizeof(receive_data));
        if(recvfrom(sockfd,receive_data,sizeof(receive_data),0,(struct sockaddr *)&my_addr,&size)<0)
        {
            log_debug0("recvfrom error!!!\n");
        }
        json_analysis(receive_data);
        if(json_analysis(receive_data)==1)  //执行失败的命令
        {
            send_message_flag=1;
            factory_auto_function_mode=1;
            log_debug0("json_analysis(receive_data)=%d\n",json_analysis(receive_data));
        }else if(json_analysis(receive_data)==0){  //收到cmd=0，关闭继电器，指示灯
            send_message_flag=0;
            factory_auto_function_mode=0;
            relay_trigger(false);
            led_power_trigger(false);
            led_wifi_trigger(false);

            log_debug0("json_analysis(receive_data)=%d\n",json_analysis(receive_data));
        }
        cJSON_Delete(cJSON_cmd_val);
    }
    //close(sockfd);
}
void udp_recive_task(void *arg)
{
    log_debug0("udp_recive_task...\n");
    _udp_receive();
}
void _udp_send(void)
{
    log_debug0("udp_send_data....\n");
    UINT8 prot_flag=1; //send 接口
    UINT8 queue_flag=true;
    int sockfd,so_broadcast=1;
    struct sockaddr_in servaddr;
    UINT8 * send_buf;
    UINT8 equipment_ip[16]={0};
    UINT8 adapter_ver[8]={0};
    SDKInformation broadcast_data;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    _udp_init(&sockfd,&prot_flag,&servaddr);
    sprintf(adapter_ver,"%d",ADA_VERSION);//获取 ADA_VERSION

    while(1)
    {
        iots_strcpy(equipment_ip, IOTWifi_LocalIP());
        if(IOTSDK_GetInformation(&broadcast_data)<0)
        {
            log_debug2("broadcast_data error\n");
        }else{
            json_create(&send_buf,equipment_ip,adapter_ver,&broadcast_data);
        }
        //数据传输处理 send_message_flag判断周期广播设备信息是否要关闭
        if(send_message_flag)
        {
            if(sendto(sockfd, send_buf, strlen(send_buf), 0, (struct sockaddr *) &servaddr, addrlen)<0)
                log_debug0("send error!errno=%d\n",errno);
            log_debug0("client: IPAddr = %s, Port = %d, send_buf = %s\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port),send_buf);
            free(send_buf);
        }
        vTaskDelay(2000/portTICK_RATE_MS);
    }
    //close(sockfd);
}
void udp_send_task(void *arg)
{
    _udp_send();
}

