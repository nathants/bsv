/* source: https://github.com/nathants/c-argh/raw/d32113e538b8be503e5c483e86a1b77de8f57835/argh.h */
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGH_PARSE                                                                      \
    if (strcmp(argv[argc - 1], "-h") == 0 || strcmp(argv[argc - 1], "--help") == 0) {   \
        fprintf(stderr, "usage: %s\n", USAGE);                                          \
        exit(1);                                                                        \
    }                                                                                   \
    bool argh_used = true;                                                              \
    bool argh_repeat = false;                                                           \
    bool argh_val = false;                                                              \
    char *ARGH_ARGV[argc];                                                              \
    char *argh_name;                                                                    \
    int ARGH_ARGC = 0;                                                                  \
    int argh_diff = 0;                                                                  \
    int argh_offset = 0;                                                                \
    while(argh_offset < argc)

// check for a flag which is either present or not
#define ARGH_BOOL(short_name, long_name)                                                                                                                                  \
    /* ARGH_VAL cannot be used with ARGH_BOOL */                                                                                                                          \
    (argh_name = NULL,                                                                                                                                                    \
          /* short name exact match */                                                                                                                                    \
     ((    0 ==  strcmp(argv[argh_offset], short_name)                                                                                                                    \
          /* short name prefix match, for specify multiple short names with a single param, -xyz instead -x -y -z. this mutates argv for the next pass, -xyz to -yz  */   \
       || ((0 == strncmp(argv[argh_offset], short_name, 2) && strlen(argv[argh_offset]) > 2 && (argv[argh_offset]++, argv[argh_offset][0] = '-', argh_repeat = true)))    \
          /* long name exact match */                                                                                                                                     \
        ||  0 ==  strcmp(argv[argh_offset], long_name))                                                                                                                   \
      /* set argh_used to avoid adding to ARGH_ARGV, return true */                                                                                                       \
      ? argh_used = true                                                                                                                                                  \
      /* return false */                                                                                                                                                  \
      : false))

// check for a flag which if present is followed by a value
#define ARGH_FLAG(short_name, long_name)                                                              \
    /* set argh_name so we can print it later as an error if no value is next in argv */              \
    (argh_name = long_name,                                                                           \
         /* maybe set arg_diff=strlen(short_name) in case flag is "--nameVALUE" not "--name VALUE" */ \
     (   (argh_diff = strlen(argv[argh_offset]) != strlen(short_name) ? strlen(short_name) : 0,       \
         /* short name prefix match, if argh_diff == 0 then this is an exact match */                 \
          0 == strncmp(argv[argh_offset], short_name, strlen(short_name)))                            \
         /* maybe set arg_diff=strlen(long_name) in case flag is "--nameVALUE" not "--name VALUE" */  \
      || (argh_diff = strlen(argv[argh_offset]) != strlen(long_name) ? strlen(long_name) : 0,         \
         /* long name prefix match, if argh_diff == 0 then this is an exact match */                  \
          0 == strncmp(argv[argh_offset], long_name, strlen(long_name))))                             \
     /* set argh_used to avoid adding to ARGH_ARGV, return true */                                    \
     ? argh_used = true                                                                               \
     /* return false */                                                                               \
     : false)

// get value for the current flag
#define ARGH_VAL()                                                                                          \
    /* assert can only be used with ARGH_FLAG */                                                            \
    ((argh_name == NULL)                                                                                    \
     ? (fprintf(stderr, "fatal: ARGH_VAL can only be used with ARGH_FLAG\n", argh_name), exit(1), NULL)     \
     /* if argh_diff, use suffix of current argv instead of next argv */                                    \
     : (argh_diff                                                                                           \
       ? (argv[argh_offset] + argh_diff)                                                                    \
       /* else use next argv as long as it exists and does not start with "-" */                            \
       : ((argh_offset < argc - 1 && strlen(argv[argh_offset + 1]) > 0 && argv[argh_offset + 1][0] != '-')  \
          /* set argh_val to offset argv one additional space, return val */                                \
          ? (argh_val = true, argv[argh_offset + 1])                                                        \
          : (fprintf(stderr, "fatal: needs value %s\n", argh_name), exit(1), NULL))))

#define ARGH_NEXT()                                                                             \
    /* reset argh name, which is set by ARGH_FLAG  */                                           \
    argh_name = NULL;                                                                           \
    /* if not argh_used, save as a positional argument for later, then reset argh_used  */      \
    if (!argh_used)                                                                             \
        ARGH_ARGV[ARGH_ARGC++] = argv[argh_offset];                                             \
    argh_used = false;                                                                          \
    /* if argh_val, offset argv an additional space because of ARGH_VAL, then reset argh_val */ \
    if (argh_val)                                                                               \
        argh_offset++;                                                                          \
    argh_val = false;                                                                           \
    /* if not argh_repeat, offset argv, then reset */                                           \
    if (!argh_repeat)                                                                           \
        argh_offset++;                                                                          \
    argh_repeat = false;                                                                        \
    /* check if we are done with all of argv and break is so */                                 \
    if (argh_offset == argc)                                                                    \
        break;
