#ifndef WJSON_H
#define WJSON_H

#include <stdint.h>

typedef enum WJSONType { // each JSON field has a type associated with it.
  WJ_TYPE_INVALID =
      0, // objects are zero-allocated, so if an object field grabbed has an
         // invalid type, we know it hasn't been initialized yet and we missed.
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
    WJSONObject object; // an object, acts like a hashmap. this isn't a pointer
                        // because it already points to an array!
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

/* all of the helper interface methods with the returned WJSONValue structure
 * operate generically and don't force the caller to actually check the type.
 * they'll just throw an error if the wrong type is passed. */

/* we really only need the getter to be public here. */
WJSONValue *wjson_get(WJSONValue *value, const char *key);

/* index into a JSON value object, assuming that the object is of the proper
 * array type. */
WJSONValue *wjson_index(WJSONValue *value, unsigned int array_index);

/* the only actual parsing function, takes in a path to a file and returns
 * either the top level JSON object or just errors. TODO: is it a good use of
 * time to actually handle errors better? it might get annoying for a library to
 * forcefully exit the whole application. */
WJSONValue *wjson_parse_file(const char *file_path);
/* another version that just takes in an input buffer directly. */
WJSONValue *wjson_parse_string(char *input);

/* doesn't prettyprint objects since it doesn't keep around the string names, so
 * this might have limited usefulness. also, control whether it should show the
 * hashed string value when printing out all the subobjects of a WJSONValue
 * object type. */
void wjson_pretty_print(WJSONValue *value, int show_hash);

#endif // !WJSON_H
