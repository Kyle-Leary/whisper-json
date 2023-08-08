#pragma once

#include "defines.h"
#include <signal.h>
#include <stdint.h>

typedef enum NodeType {
  NT_NULL = 0, // always the zero variant, a zero-bytes node is invalid and
               // skipped in insertion functions.

  NT_VALUE, // a JSON value with an associated type.

  NT_EMPTY, // epsilon empty statement

  NT_DECL, // representation of "key": value, with the key in the data field and
           // value as the left node.

  NT_LIST, // use a generic list node type, instead of re-implementing the same
           // StatementList, ArgList, ... they're really not different, even in
           // different contexts.

  NT_BLOCK, // this is more than just another statement list. we need to keep
            // information about blocks around in the ast for scoping reasons.

  NT_COUNT
} NodeType;

typedef u16 NodeIndex; // let the indexing into the node array be 16 bits wide.
typedef Value64 NodeData;

// 128 bit large structure, 64-bit align it.
typedef struct Node {
  NodeType type;
  NodeIndex left; // store indices into the ast as child nodes.
  NodeIndex right;
  NodeData data; // an arbitrary 64-bit piece of data, this can also be used as
                 // a raw integer, depending on the node type.
} Node;

// blank data field, explicitly stated with a named macro rather than implicitly
// punning a {0}. less secretive!
#define BLANK_VALUE64                                                          \
  (Value64) { .as_uint = 0 }

#define NO_NODE_DATA BLANK_VALUE64
#define NO_TOKEN_DATA BLANK_VALUE64

// the ast array itself and ast management functionality, along with node type
// defines.
extern Node ast[AST_LEN]; // store the literal values in a row.

// then, helpers for managing the ast itself.
Node make_node(NodeType type, NodeIndex left, NodeIndex right, NodeData data);
NodeIndex add_node(Node n);
void clean_ast();

void test_ast();
