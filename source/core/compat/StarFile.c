//
// Created by yaroha on 19.08.24.
//

#include "StarFile.h"

int ftruncate(int __fd, off_t __length) {
  return ftruncate(__fd, __length);
}

char * realpath (const char *__restrict path, char *__restrict resolved_path){
  return realpath(path, resolved_path);
}

