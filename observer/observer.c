#include "observer.h"

void observer_manager_init(pub_manager * mgr) {

    INIT_LIST_HEAD(&(mgr->publisher_list_head));
    mgr->publisher_count = 0;

}

void observer_publisher_init(publisher * pub,char * pub_name)
{
    INIT_LIST_HEAD(&(pub->subscribe_list_head));
    pub->pub_flag = PUB_IDLE;
    strncpy(pub->name, pub_name, sizeof(pub->name));
    pub->pub_data = NULL;
}

void observer_subscribe_init(subscribe * sub,char * sub_name,void(*callback)(void * data))
{
    sub->callback = callback;
    strncpy(sub->name, sub_name, sizeof(sub->name));
}

void observer_add_publisher(pub_manager *mgr, publisher *pub) {
    list_add_tail(&pub->node, &mgr->publisher_list_head);
    mgr->publisher_count++;
}

void observer_del_publisher(pub_manager *mgr,publisher *pub)
{
    list_del(&pub->node);
    mgr->publisher_count--;
}

publisher * observer_find_publisher(pub_manager *mgr,char * pub_name)
{
    struct list_head * pos;
    list_for_each(pos,&(mgr->publisher_list_head)){
        publisher *pub = list_entry(pos,publisher,node);
        if (strcmp(pub->name, pub_name) == 0) {
            return pub;
        }
    }
    return NULL;
}

void observer_add_subscribe(publisher *pub,subscribe *sub) {
    list_add_tail(&sub->node, &pub->subscribe_list_head);
}

void observer_del_subscribe(subscribe *sub)
{
    list_del(&sub->node);
}

subscribe * observer_find_subscribe(publisher *pub,char * sub_name)
{
    struct list_head * pos;
    list_for_each(pos,&(pub->subscribe_list_head)){
        subscribe *sub = list_entry(pos,publisher,node);
        if (strcmp(sub->name, sub_name) == 0) {
            return sub;
        }
    }
    return NULL;
}

void _observer_publish(publisher *pub, void * data)
{
    struct list_head *pos;
    pub->pub_data = data;

    // 通知订阅者
    list_for_each(pos, &pub->subscribe_list_head) {
        subscribe *sub = list_entry(pos, subscribe, node);
        if (sub->callback) {
            sub->callback(data);
        }
    }
}

int observer_publish(pub_manager * mgr,char * name,void * data)
{
    publisher * pos = observer_find_publisher(mgr,name);
    if(pos == NULL)
        return -1;

    _observer_publish(pos ,data);
    return 0;
}

