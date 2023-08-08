#include "json_object.h"
#include <stdlib.h>
#include <string.h>

static unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

// public api functionality
WJSONValue *wjson_object_get(WJSONObject object, const char *key) {
  return object + (hash((char *)key) % WJSON_OBJECT_LEN);
}

// private internal functionality
void wjson_object_insert(WJSONObject object, const char *key,
                         WJSONValue *value) {
  memcpy(object + (hash((char *)key) % WJSON_OBJECT_LEN), value,
         sizeof(WJSONValue) * 1);
}

WJSONObject *wjson_object_create() {
  WJSONObject *object = (WJSONObject *)calloc(sizeof(WJSONObject), 1);
  return object;
}
