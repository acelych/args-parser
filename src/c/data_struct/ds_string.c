#include "ds_string.h"

Signal append_string(String* this, const char* format, ...) {
	char *temp_str;
	va_list args;

    va_start(args, format);
    int length = vsnprintf(NULL, 0, format, args) + 1;
    if (length < 0) {
        va_end(args);
        return Error;
    }
    temp_str = malloc(sizeof(char) * length);
    vsnprintf(temp_str, length, format, args);
    va_end(args);

	Signal res = this->parent->appendMulti(this->parent, temp_str);
	free(temp_str);
	return res;
}

Signal appendChar_string(String* this, const char ch) {
	char merged[2] = { ch, '\0' };
	return this->append(this, merged);
}

Signal clear_string(String* this) {
	return this->parent->clear(this->parent);
}

unsigned long long length_string(String* this) {
	return this->parent->length;
}

char *begin_string(String* this) {
	return this->parent->begin(this->parent);
}

char *end_string(String* this) {
	return this->parent->end(this->parent);
}

char *chrCopy_string(String* this) {
	char *newArray = malloc(this->length(this) + 1);
	memset(newArray, 0, this->length(this) + 1);
	memcpy(newArray, this->begin(this), this->length(this));
	return newArray;
}

void delete_string(String* this) {
	this->parent->delete(this->parent);
	free(this);
}

String *String_GetInstance(const char *content) {
	String *instance = malloc(sizeof(String));
	instance->parent = Vector_GetInstance(sizeof(char));

	instance->append = &append_string;
	instance->appendChar = &appendChar_string;
	instance->clear = &clear_string;
	instance->length = &length_string;
	instance->begin = &begin_string;
	instance->end = &end_string;
	instance->chrCopy = &chrCopy_string;
	instance->delete = &delete_string;

	instance->append(instance, content);

	return instance;
}