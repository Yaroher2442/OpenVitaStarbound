//
// Created by yaroha on 19.08.24.
//
#pragma once
#ifndef STARFILE_H
#include <sys/types.h>
#include <stdlib.h>
#define STARFILE_H

#endif//STARFILE_H

#ifdef __cplusplus
extern "C" {
#endif
int ftruncate(int __fd, off_t __length);
char * realpath (const char *__restrict path, char *__restrict resolved_path);
#ifdef __cplusplus
}
#endif
