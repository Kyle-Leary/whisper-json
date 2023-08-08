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
// signify failure (a key miss) with a NULL value ptr.
static WJSONValue *wjson_object_get(WJSONObject object, const char *key) {
  WJSONValue *value = object + (hash((char *)key) % WJSON_OBJECT_LEN);
  if (value->type == WJ_TYPE_INVALID) {
    // don't print anything. the caller can work this out on their own and react
    // how they'd like.
    return NULL;
  }
  return value;
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
  // we'll suck it up and actually spend the time zero-allocing the object. this
  // allows us to know when we've "missed" in the object getter with reasonable
  // accuracy.
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
