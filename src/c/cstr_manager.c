#include "cstr_manager.h"

int intlen(int num)
{
    int length = num > 0 ? 0 : 1;
    while (num != 0) {
        num /= 10;
        length++;
    }
    return length;
}

int cstringFormater(char **Dst, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int length = vsnprintf(NULL, 0, format, args) + 1;
    if (length < 0) {
        va_end(args);
        return EXIT_FAILURE;
    }
    *Dst = malloc(sizeof(char) * length);
    vsnprintf(*Dst, length, format, args);

    va_end(args);
    return EXIT_SUCCESS;
}

int cstringFormaterValist(char **Dst, const char *format, __builtin_va_list list) {
    int length = vsnprintf(NULL, 0, format, list) + 1;
    if (length < 0) return EXIT_FAILURE;
    *Dst = malloc(sizeof(char) * length);
    vsnprintf(*Dst, length, format, list);
    return EXIT_SUCCESS;
}

int cstringUpperCase(char **Dst, const char *Src)
{
    size_t length = strlen(Src);
    *Dst = malloc(sizeof(char) * (length + 1));
    for (size_t i = 0; i < length; i++)
    {
        (*Dst)[i] = toupper(Src[i]);
    }
    (*Dst)[length] = '\0';
    return EXIT_SUCCESS;
}

int wcstringFormater(wchar_t **Dst, const wchar_t *format, ...) {
    va_list args;
    va_start(args, format);

#ifndef _MSC_VER
    int length = vsnwprintf(NULL, 0, format, args) + 1;
#else
    int length = _vsnwprintf_s (NULL, 0, _TRUNCATE, format, args) + 1;
#endif
    if (length < 0) {
        va_end(args);
        return EXIT_FAILURE;
    }
    *Dst = malloc(sizeof(wchar_t) * length);
#ifndef _MSC_VER
    vsnwprintf(*Dst, length, format, args);
#else
    _vsnwprintf_s(*Dst, length, _TRUNCATE, format, args) + 1;
#endif

    va_end(args);
    return EXIT_SUCCESS;
}

int cstr2wcstr(wchar_t **Dst, const char *Src) {
    size_t size = sizeof(wchar_t) * (strlen(Src) + 1);
    *Dst = malloc(size);
	memset(*Dst, 0, size);
	mbstowcs(*Dst, Src, strlen(Src));
	return 0;
}