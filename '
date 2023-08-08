#ifndef WJSON_H
#define WJSON_H

#include <stdint.h>

typedef enum WJSONType { // each JSON field has a type associated with it.
  WJ_TYPE_STRING,
  WJ_TYPE_NUMBER,
  WJ_TYPE_BOOLEAN,
  WJ_TYPE_ARRAY,
  WJ_TYPE_OBJECT,
  WJ_TYPE_NULL,
  WJ_TYPE_COUNT,
} WJSONType;

typedef struct WJSONValue WJSONValue;

// an array of values to be hashed over. most of the slots will be empty.
typedef WJSONValue *WJSONObject;

// the data associated with a WJSONValue.
typedef struct WJSONData {
  union {
    char *string;
    double number;     // in JSON, all "number" values are doubles.
    WJSONValue *array; // a simple list of values with specified length.
    int boolean;
    WJSONObject *object; // an object pointer, acts like a hashmap.
  } value;
  union {
    unsigned int str_len;
    unsigned int array_len;
    uint64_t padding_garbage; // some don't have this extra data field.
  } length;
} WJSONData;

typedef struct WJSONValue {
  WJSONType type;
  WJSONData data;
} WJSONValue;

typedef struct WJSONFile {
  WJSONValue *root; // the top level value may be any value, but there's only
                    // one top level value in a JSON file. therefore, we can
                    // fully describe the data in the file recursively with one
                    // root WJSONValue in a structure tree.
} WJSONFile;

// we really only need the getter to be public here.
WJSONValue *wjson_object_get(WJSONObject object, const char *key);

WJSONFile wjson_parse_file(const char *file_path);

#endif // !WJSON_H
