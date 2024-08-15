#ifndef __C_STR_MANAGER_H
#define __C_STR_MANAGER_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int intlen(int num);

int cstringFormater(char **Dst, const char *format, ...);
int cstringFormaterValist(char **Dst, const char *format, __builtin_va_list list);
int wcstringFormater(wchar_t **Dst, const wchar_t *format, ...);
int cstr2wcstr(wchar_t **Dst, const char *Src);

int cstringUpperCase(char **Dst, const char *Src);

#endif