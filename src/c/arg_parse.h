#ifndef __ARG_PARSE_H
#define __ARG_PARSE_H

#include "ds_vector.h"
#include "ds_string.h"
#include "cstr_manager.h"

typedef struct ARG_OPTION
{
    const char *name;
    const char *full_name;
    const char *description;
    Bool required;
    int least_params;

    int idx;
    int params;
} ARG_OPTION;

typedef struct ARG_PARSER
{
    int argc;
    const char **argv;
    const char *description;
    Vector *args_list;

    Signal (*addArgOpt)(struct ARG_PARSER*, const char* name, const char* full_name, const char *description, Bool required, int least_params);
    Signal (*parseArgs)(struct ARG_PARSER*, int argc, const char **argv);
    Signal (*showHelpMsg)(struct ARG_PARSER*);

    ARG_OPTION* (*findOption)(struct ARG_PARSER*, const char* name);

    void (*delete)(struct ARG_PARSER*);
} ARG_PARSER;

ARG_PARSER * ARG_PARSER_GetInstance(const char *description);

int findOption(int argc, const char **argv, ...);
int findOptionParams(int argc, const char **argv, int *params, ...);

#endif