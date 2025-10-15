#include "vstring.h"

VECTOR_ERROR string_init(string *str,char * cstr) {
    VECTOR_ERROR err = vector_init(&str->vec, sizeof(char));
    if (err != VECTOR_OK) return err;

    // 添加一个 '\0'，保证字符串合法
    if(cstr != NULL){
        for(int i = 0; i < strlen(cstr); i++){
            vector_push_back(&str->vec, cstr[i]);
        }
    }
    char zero = '\0';
    vector_push_back(&str->vec, &zero);
    return VECTOR_OK;
}


string * string_create(char * cstr)
{
    string * str = vector_create();
    if(cstr != NULL){
        for(int i = 0; i < strlen(cstr); i++){
            vector_push_back(&str->vec, cstr[i]);
        }
    }
    char zero = '\0';
    vector_push_back(&str->vec, &zero);
    return str;
}

void string_free(string *str) {
    vector_free(&str->vec);
}

size_t string_length(string *str) {
    if (str->vec.size == 0) return 0;
    return str->vec.size - 1;
}

// 清空字符串
void string_clear(string *str) {
    str->vec.size = 0;
    char zero = '\0';
    vector_push_back(&str->vec, &zero);
 
}

// 追加 C 字符串
VECTOR_ERROR string_append_cstr(string *str, const char *cstr) {
    size_t len = strlen(cstr);
    // 删除末尾的 '\0'
    if (str->vec.size > 0) str->vec.size--;

    for (size_t i = 0; i < len; i++) {
        vector_push_back(&str->vec, (void *)&cstr[i]);
    }

    char zero = '\0';
    vector_push_back(&str->vec, &zero);
    return VECTOR_OK;
}

VECTOR_ERROR string_append_char(string *str, char c) {
    if (str->vec.size > 0) str->vec.size--;
    vector_push_back(&str->vec, &c);
    char zero = '\0';
    vector_push_back(&str->vec, &zero);
    return VECTOR_OK;
}

// 比较两个字符串（类似 strcmp）
int string_compare(string *a, string *b) {
    return strcmp((char *)a->vec.data, (char *)b->vec.data);
}

// 获取底层 C 字符串
const char *string_c_str(string *str) {
    return (const char *)str->vec.data;
}

// 拷贝
void string_copy(string *dst, const char *src) {
    string_clear(dst);
    string_append_cstr(dst, src);
}

// 查找字符 c 第一次出现的位置，找不到返回 -1
int string_find_char(const string *str, char c) {
    const char *s = (const char *)str->vec.data;
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == c) return (int)i;
    }
    return -1;
}

// 查找字符 c 最后一次出现的位置，找不到返回 -1
int string_find_last_char(const string *str, char c) {
    const char *s = (const char *)str->vec.data;
    int pos = -1;
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == c) pos = (int)i;
    }
    return pos;
}

// 查找子串 sub 第一次出现的位置，找不到返回 -1
int string_find(const string *str, const char *sub) {
    const char *s = (const char *)str->vec.data;
    const char *p = strstr(s, sub);
    if (p == NULL) return -1;
    return (int)(p - s);
}

// 从指定位置开始查找子串
int string_find_from(const string *str, const char *sub, size_t start) {
    const char *s = (const char *)str->vec.data;
    if (start >= string_length(str)) return -1;
    const char *p = strstr(s + start, sub);
    if (p == NULL) return -1;
    return (int)(p - s);
}

// 判断是否包含子串
int string_contains(const string *str, const char *sub) {
    return (string_find(str, sub) != -1);
}



// 从 [start, start+len) 提取子串，存入 dst
void string_substr(string *dst, const string *src, size_t start, size_t len) {
    string_clear(dst);

    size_t src_len = string_length(src);
    if (start >= src_len) return;  // 超出范围

    const char *s = string_c_str(src);
    size_t copy_len = (start + len > src_len) ? (src_len - start) : len;

    for (size_t i = 0; i < copy_len; i++) {
        string_append_char(dst, s[start + i]);
    }
}

// ===================== 去除首尾空白 =====================
void string_trim(string *str) {
    const char *s = string_c_str(str);
    size_t len = string_length(str);

    size_t start = 0, end = len;

    // 找到首个非空白字符
    while (start < len && isspace((unsigned char)s[start])) start++;
    // 找到末尾非空白字符
    while (end > start && isspace((unsigned char)s[end - 1])) end--;

    string tmp;
    string_init(&tmp,NULL);
    string_substr(&tmp, str, start, end - start);

    // 拷贝回原对象
    string_clear(str);
    string_append_cstr(str, string_c_str(&tmp));

    string_free(&tmp);
}

// ===================== 替换子串 =====================
// 将所有 old_str 替换为 new_str
void string_replace(string *str, const char *old_str, const char *new_str) {
    const char *src = string_c_str(str);
    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);

    if (old_len == 0) return;  // 防止死循环

    string result;
    string_init(&result,NULL);

    const char *pos = src;
    const char *found;

    while ((found = strstr(pos, old_str)) != NULL) {
        // 先拷贝 [pos, found)
        size_t segment_len = found - pos;
        for (size_t i = 0; i < segment_len; i++) {
            string_append_char(&result, pos[i]);
        }
        // 插入 new_str
        string_append_cstr(&result, new_str);

        pos = found + old_len;
    }

    // 拷贝剩余部分
    string_append_cstr(&result, pos);

    // 覆盖回原字符串
    string_clear(str);
    string_append_cstr(str, string_c_str(&result));

    string_free(&result);
}

// ===================== 分割字符串 =====================
// 把 str 按 sep 分割，结果存入 vector<string_t> 中
// 用完后需释放每个 string_t + vector 本身
void string_split(const string *str, char sep, vector *out_list)
{
    vector_init(out_list, sizeof(string)); // out_list 是 vector<string_t>

    const char *s = string_c_str(str);
    const char *p = s;
    const char *start = p;

    while (*p != '\0') {
        if (*p == sep) {
            if (p > start) { // 非空段
                string token;
                string_init(&token,NULL);
                string_substr(&token, str, start - s, p - start);
                vector_push_back(out_list, &token);
            }
            start = p + 1;
        }
        p++;
    }

    // 最后一段
    if (p > start) {
        string token;
        string_init(&token,NULL);
        string_substr(&token, str, start - s, p - start);
        vector_push_back(out_list, &token);
    }
}

// ===================== 拼接字符串 =====================
// 将 vector<string_t> 中的所有字符串，用 sep 拼接成 dst
void string_join(string *dst, const vector *list, char sep)
{
    string_clear(dst);
    size_t n = list->size;
    for (size_t i = 0; i < n; i++) {
        string *s = (string *)vector_get((vector *)list, i);
        string_append_cstr(dst, string_c_str(s));
        if (i < n - 1) {
            string_append_char(dst, sep);
        }
    }
}

// ===================== 释放 split 结果 =====================
// 每个元素都是 string_t，需要手动释放
void string_split_free(vector *list)
{
    for (size_t i = 0; i < list->size; i++) {
        string *s = (string *)vector_get(list, i);
        string_free(s);
    }
    vector_free(list);
}

