#ifndef __VECTOR_H__
#define __VECTOR_H__
#include "stdint.h"
typedef struct Vector vector;

struct Vector
{
    void *data;       // 数据区
    size_t elem_size; // 单个元素大小
    size_t size;      // 当前已有元素数
    size_t capacity;  // 容量
};

typedef enum
{
    VECTOR_OK = 0,
    VECTOR_MEMORY_ERROR = 1,
    VECTOR_PUSH_ERROR = 2,
}VECTOR_ERROR;

VECTOR_ERROR vector_init(vector * vec,size_t elem_size);
// 释放
void vector_free(vector *vec);
//扩容
VECTOR_ERROR vector_expand(vector *vec);
//添加数据
VECTOR_ERROR vector_push_back(vector *vec,void *elem);
//获取指定数据指针
void *vector_get(vector *vec, size_t index);
// 删除最后一个元素
void vector_pop_back(vector *vec);
// 获取当前元素个数
size_t vector_size(vector *vec);
// 获取容量
size_t vector_capacity(vector *vec);

vector *vector_create(void);
#endif