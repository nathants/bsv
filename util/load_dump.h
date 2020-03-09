#ifndef LOAD_DUMP_H
#define LOAD_DUMP_H

#include "load.h"
#include "dump.h"

#define LOAD_DUMP_INIT()                        \
    FILE *load_files[1] = {stdin};              \
    LOAD_INIT(load_files, 1);                   \
    FILE *dump_files[1] = {stdout};             \
    DUMP_INIT(dump_files, 1);

#endif
