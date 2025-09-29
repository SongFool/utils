#include "list.h"
#include "stdint.h"

typedef enum {
    PUB_IDLE = 0,   // 空闲，没有新数据
    PUB_DONE = 1,
}pub_state_t;


typedef struct _publisher
{
    struct list_head subscribe_list_head;   //订阅者列表 
    void * pub_data;                            //数据
    pub_state_t pub_flag;                       //发布标志位
    char name[16];                         //名字
    struct list_head node;                  // 挂接到 manager 的链表
}publisher;


typedef struct _subscribe
{
    struct list_head node;                // 挂接到 publisher 的链表
    void (*callback)  (void * data);        //订阅者收到数据后动作
    char name[16];                         //名字
}subscribe;


// 管理器：统一管理多个发布者
typedef struct _pub_manager {
    struct list_head publisher_list_head;   // 发布者链表
    int publisher_count;                    // 发布者数量
} pub_manager;