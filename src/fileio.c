#include "fileio.h"

usize file_size(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s", path);
        exit(1);
    }
    usize num = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        num++;
    }
    fclose(fp);
    return num;
}

usize file_binary_size(char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s", path);
        exit(1);
    }
    usize num = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        num++;
    }
    fclose(fp);
    return num;
}

String *cat(char *path) {
    usize size = file_size(path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s", path);
        exit(1);
    }
    char *content = safe_malloc((size + 1) * sizeof(char));
    for (usize i = 0; i < size; i++) {
        content[i] = fgetc(fp);
    }
    fclose(fp);
    String *s = string_init_with_length(content, size);
    free(content);
    return s;
}

void core_write(char *path, char *content) {
    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s", path);
        exit(1);
    }
    fputs(content, fp);
    fclose(fp);
}

char *read_binary(char *path, usize *size_pointer) {
    usize size = file_binary_size(path);
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file: %s", path);
        exit(1);
    }
    char *content = safe_malloc(size * sizeof(char));
    for (usize i = 0; i < size; i++) {
        content[i] = fgetc(fp);
    }
    fclose(fp);
    *size_pointer = size;
    return content;
}
