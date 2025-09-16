
#include "vector.h"
#include "stdlib.h"
#include "string.h"

#define LOG(fmt, ...)  printf("[LOG] " fmt "\n", ##__VA_ARGS__)  





// 初始化

VECTOR_ERROR vector_init(vector * vec,size_t elem_size)
{
    VECTOR_ERROR code = VECTOR_OK;
    vec->elem_size = elem_size;
    vec->size = 0;
    vec->capacity = 4;
    vec->data = malloc(vec->capacity * elem_size);
    if (vec->data == NULL) {
        LOG("vector Failed to allocate space \r\n");
        code = VECTOR_MEMORY_ERROR;
    }
    return code;
}

vector *vector_create(void) {
    vector *v = malloc(sizeof(vector));
    if(v != NULL){
        memset(v,0x00,sizeof(vector));
    }
    return v;
}

// 释放
void vector_free(vector *vec) 
{
    if(vec->data != NULL){
        free(vec->data);
        vec->data = NULL;
    }
    vec->size = 0;
    vec->capacity = 0;
}

//扩容
VECTOR_ERROR vector_expand(vector *vec)
{
    VECTOR_ERROR code = VECTOR_OK;
    void * new_data = NULL;
    vec->capacity = vec->capacity * 2;
    new_data = realloc(vec->data,vec->capacity * vec->elem_size);
    if (new_data == NULL) {
        LOG("vector Failed to allocate space \r\n");
        code = VECTOR_MEMORY_ERROR;

    }else{
        vec->data = new_data;
    }
    return code;
}

//添加数据
VECTOR_ERROR vector_push_back(vector *vec,void *elem)
{
    VECTOR_ERROR code = VECTOR_OK;
    if(vec->size == vec->capacity){
        code = vector_expand(vec);
        if(code == VECTOR_MEMORY_ERROR){
            code = VECTOR_PUSH_ERROR;
            return code;
        }
    }
    memcpy(vec->data + (vec->size * vec->elem_size),elem,vec->elem_size);
    vec->size++;
    return code;
}

//获取指定数据指针
void *vector_get(vector *vec, size_t index) {
    if (index >= vec->size) {
        return NULL;
    }
    return vec->data + index * vec->elem_size;
}

// 删除最后一个元素
void vector_pop_back(vector *vec) {
    if (vec->size > 0) {
        vec->size--;
    }
}

// 获取当前元素个数
size_t vector_size(vector *vec) {
    return vec->size;
}

// 获取容量
size_t vector_capacity(vector *vec) {
    return vec->capacity;
}