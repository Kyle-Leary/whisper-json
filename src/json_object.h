#pragma once

#include "wjson.h"

#define WJSON_OBJECT_LEN 2048

// expose this functionality only internally.
void wjson_object_insert(WJSONObject object, const char *key,
                         WJSONValue *value);
WJSONObject wjson_object_create();
