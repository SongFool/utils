#include "vector.h"
typedef struct String string;
struct String{
    vector vec;  // 使用你的动态数组作为底层存储
};
