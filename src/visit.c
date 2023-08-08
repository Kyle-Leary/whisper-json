#include "visit.h"
#include "ast.h"
#include "wjson.h"
#include <stdlib.h>
#include <string.h>

// visit the root node, and generate a tree from that.
WJSONValue *value_from_root(NodeIndex root) {
  Node node = ast[root];

  WJSONValue *value = (WJSONValue *)calloc(sizeof(WJSONValue), 1);

  switch (node.type) {
  case NT_VALUE: {
    // COPY the data from the value.
    // TODO: recursively copy the data. make sure that references to internal
    // strings don't die here.
    memcpy(value, node.data.as_ptr, sizeof(WJSONValue));
  } break;
  default: {
  } break;
  }

  return value;
}
