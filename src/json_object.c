#include "json_object.h"
#include "wjson.h"
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

// api helpers
static WJSONValue *wjson_object_get(WJSONObject object, const char *key) {
  return object + (hash((char *)key) % WJSON_OBJECT_LEN);
}

// public api functionality
WJSONValue *wjson_get(WJSONValue *value, const char *key) {
  WJSONObject obj = value->data.value.object;
  return wjson_object_get(obj, key);
}

WJSONValue *wjson_index(WJSONValue *value, unsigned int array_index) {
  WJSONValue *array = value->data.value.array;
  return &array[array_index];
}

WJSONObject wjson_object_create() {
  WJSONObject object =
      (WJSONObject)calloc(sizeof(WJSONValue), WJSON_OBJECT_LEN);
  return object;
}

// private internal functionality
void wjson_object_insert(WJSONObject object, const char *key,
                         WJSONValue *value) {
  memcpy(object + (hash((char *)key) % WJSON_OBJECT_LEN), value,
         sizeof(WJSONValue) * 1);
}
