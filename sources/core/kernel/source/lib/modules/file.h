#ifndef _MODULES_FILE_H
#define _MODULES_FILE_H 1


typedef struct file_t{
    size_t seek_position;
    void* internal_data;
    size_t (*read)(void*, size_t, struct file_t*);
    size_t (*write)(void*, size_t, struct file_t*);
} file_t;

#endif // _MODULES_FILE_H