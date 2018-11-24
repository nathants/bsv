#ifndef LOAD_DUMP_H
#define LOAD_DUMP_H

#include "load.h"
#include "dump.h"

#define LOAD_DUMP_INIT()                        \
    FILE *load_files[1] = {stdin};              \
    FILE *dump_files[1] = {stdout};             \
    LOAD_INIT_VARS(load_files, 1);              \
    DUMP_INIT_VARS(dump_files, 1);

#endif
