#pragma once

#include <stdbool.h>

//
// /* usage: bcat [-l|--lz4] [-h N|--head N] [-p|--prefix] FILE1 ... FILEN */
//
// bool prefix = false;
// bool lz4 = false;
// int head = 0;
// ARGH_PARSE {
//     ARGH_SETUP();
//     if      ARGH_BOOL("-p", "--prefix") { prefix = true;}
//     else if ARGH_BOOL("-l", "--lz4")    { lz4 = true; }
//     else if ARGH_FLAG("-h", "--head")   { ASSERT(isdigits(ARGH_VAL()), "fatal: should have been `--head INT`, not `--head %s`\n", ARGH_VAL());
//                                          head = atol(ARGH_VAL()); }
// }
//
// for (int i = 0; i < ARGH_ARGC; i++)
//     printf("positional arg %d: %s\n", i, ARGH_ARGV[i]);
//

#define ARGH_PARSE                              \
    bool argh_used = true;                      \
    bool argh_val = false;                      \
    char *ARGH_ARGV[argc];                      \
    char *argh_name;                            \
    int ARGH_ARGC = 0;                          \
    int argh_diff = 0;                          \
    int argh_offset = 0;                        \
    while(argh_offset < argc)

#define ARGH_BOOL(short_name, long_name)            \
    ((   0 == strcmp(argv[argh_offset], short_name) \
      || 0 == strcmp(argv[argh_offset], long_name)) \
     ? argh_used = true                             \
     : false)

#define ARGH_FLAG(short_name, long_name)                                                                    \
    (argh_name = long_name,                                                                                 \
     (   (argh_diff = strlen(argv[argh_offset]) != strlen(short_name) ? strlen(short_name) : 0,             \
          0 == strncmp(argv[argh_offset], short_name, strlen(short_name)))                                  \
      || (argh_diff = strlen(argv[argh_offset]) != strlen(long_name)  ? strlen(long_name) : 0,              \
          0 == strncmp(argv[argh_offset], long_name,  strlen(long_name))))                                  \
     ? argh_used = true                                                                                     \
     : false)

#define ARGH_VAL()                                                                  \
    (argh_diff                                                                      \
     ? (argv[argh_offset] + argh_diff)                                              \
     : ((argh_offset < argc - 1)                                                    \
        ? (argh_val = true, argv[argh_offset + 1])                                  \
        : (fprintf(stderr, "fatal: needs value %s\n", argh_name), exit(1), NULL)))

#define ARGH_NEXT()                                 \
    if (!argh_used)                                 \
        ARGH_ARGV[ARGH_ARGC++] = argv[argh_offset]; \
    if (argh_val)                                   \
        argh_offset++;                              \
    argh_val = false;                               \
    argh_used = false;                              \
    argh_offset++;                                  \
    if (argh_offset == argc)                        \
        break;
