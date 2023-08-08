#pragma once

#include "defines.h"
#include "wjson.h"

WJSONValue *parse(char text_input[INPUT_LEN]);
void test_parse();
