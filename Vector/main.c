#include "vector.h"
int main() {

    vector vec;
    vector_init(&vec, sizeof(int));

    // 提前申请 100 个空间，避免频繁 realloc

    for (int i = 0; i < 10; i++) {
        vector_push_back(&vec, &i);
    }

    for(int i = 0; i < vector_size(&vec); i ++){
        printf("data : %d\r\n",((int*) vec.data)[i]);
    }
    printf("size = %zu, capacity = %zu\n", vector_size(&vec), vector_capacity(&vec));

    vector_free(&vec);
    return 0;
}
