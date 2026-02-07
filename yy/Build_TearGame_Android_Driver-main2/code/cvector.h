#ifndef CVECTOR_H
#define CVECTOR_H

#include <linux/kernel.h>
#include <linux/slab.h>

// 成功返回值
#define CVESUCCESS 0

// 动态数组结构
typedef struct {
    void *data;         // 数据指针
    size_t element_size; // 单个元素大小
    size_t size;        // 当前元素数量
    size_t capacity;    // 容量
} *cvector;

// 创建动态数组
__always_inline cvector cvector_create(size_t element_size) {
    cvector vec = kmalloc(sizeof(*vec), GFP_KERNEL);
    if (!vec) {
        return NULL;
    }
    
    vec->element_size = element_size;
    vec->size = 0;
    vec->capacity = 4; // 初始容量为4
    
    vec->data = kmalloc(element_size * vec->capacity, GFP_KERNEL);
    if (!vec->data) {
        kfree(vec);
        return NULL;
    }
    
    return vec;
}

// 销毁动态数组
__always_inline void cvector_destroy(cvector vec) {
    if (vec) {
        kfree(vec->data);
        kfree(vec);
    }
}

// 获取数组长度
__always_inline size_t cvector_length(cvector vec) {
    return vec ? vec->size : 0;
}

// 重新分配容量
__always_inline int cvector_reserve(cvector vec, size_t new_capacity) {
    void *new_data;
    
    if (!vec || new_capacity <= vec->capacity) {
        return CVESUCCESS;
    }
    
    new_data = krealloc(vec->data, vec->element_size * new_capacity, GFP_KERNEL);
    if (!new_data) {
        return -ENOMEM;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    return CVESUCCESS;
}

// 向数组添加元素
__always_inline int cvector_pushback(cvector vec, void *element) {
    char *dest;
    
    if (!vec || !element) {
        return -EINVAL;
    }
    
    // 检查容量是否足够
    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity * 2;
        if (cvector_reserve(vec, new_capacity) != CVESUCCESS) {
            return -ENOMEM;
        }
    }
    
    // 复制元素
    dest = (char *)vec->data + vec->size * vec->element_size;
    memcpy(dest, element, vec->element_size);
    vec->size++;
    
    return CVESUCCESS;
}

// 获取指定位置元素
__always_inline int cvector_val_at(cvector vec, size_t index, void *out) {
    char *src;
    
    if (!vec || !out || index >= vec->size) {
        return -EINVAL;
    }
    
    src = (char *)vec->data + index * vec->element_size;
    memcpy(out, src, vec->element_size);
    return CVESUCCESS;
}

#endif // CVECTOR_H
