/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include "test-common.h"
#include <gtest/gtest.h>

typedef struct
{
  jerry_type_t type_info;
  jerry_value_t value;
} test_entry_t;

#define ENTRY(TYPE, VALUE) { TYPE, VALUE }

static jerry_value_t
test_ext_function (const jerry_value_t function_obj, /**< function object */
                   const jerry_value_t this_val, /**< function this value */
                   const jerry_value_t args_p[], /**< array of arguments */
                   const jerry_length_t args_cnt) /**< number of arguments */
{
  (void) function_obj;
  (void) this_val;
  (void) args_p;
  (void) args_cnt;
  return jerry_create_boolean (true);
} /* test_ext_function */

class ApiValueTypeTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ApiValueTypeTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ApiValueTypeTest TearDownTestCase";
    }

    void SetUp() override {}
    void TearDown() override {}

};

static constexpr size_t JERRY_SCRIPT_MEM_SIZE = 50 * 1024 * 1024;
static void* context_alloc_fn(size_t size, void* cb_data)
{
    (void)cb_data;
    size_t newSize = size > JERRY_SCRIPT_MEM_SIZE ? JERRY_SCRIPT_MEM_SIZE : size;
    return malloc(newSize);
}

HWTEST_F(ApiValueTypeTest, Test001, testing::ext::TestSize.Level1)
{
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  TEST_INIT ();

  jerry_init (JERRY_INIT_EMPTY);

  const jerry_char_t test_eval_function[] = "function demo(a) { return a + 1; }; demo";

  test_entry_t entries[] =
  {
    ENTRY (JERRY_TYPE_NUMBER, jerry_create_number (-33.0)),
    ENTRY (JERRY_TYPE_NUMBER, jerry_create_number (3)),
    ENTRY (JERRY_TYPE_NUMBER, jerry_create_number_nan ()),
    ENTRY (JERRY_TYPE_NUMBER, jerry_create_number_infinity (false)),
    ENTRY (JERRY_TYPE_NUMBER, jerry_create_number_infinity (true)),

    ENTRY (JERRY_TYPE_BOOLEAN, jerry_create_boolean (true)),
    ENTRY (JERRY_TYPE_BOOLEAN, jerry_create_boolean (false)),

    ENTRY (JERRY_TYPE_UNDEFINED, jerry_create_undefined ()),

    ENTRY (JERRY_TYPE_OBJECT, jerry_create_object ()),
    ENTRY (JERRY_TYPE_OBJECT, jerry_create_array (10)),
    ENTRY (JERRY_TYPE_ERROR, jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t *) "error")),

    ENTRY (JERRY_TYPE_NULL, jerry_create_null ()),

    ENTRY (JERRY_TYPE_FUNCTION, jerry_eval (test_eval_function,
                                            sizeof (test_eval_function) - 1,
                                            JERRY_PARSE_NO_OPTS)),
    ENTRY (JERRY_TYPE_FUNCTION, jerry_create_external_function (test_ext_function)),

    ENTRY (JERRY_TYPE_STRING, jerry_create_string (test_eval_function)),
    ENTRY (JERRY_TYPE_STRING, jerry_create_string ((jerry_char_t *) "")),
  };

  for (size_t idx = 0; idx < sizeof (entries) / sizeof (entries[0]); idx++)
  {
    jerry_type_t type_info = jerry_value_get_type (entries[idx].value);

    TEST_ASSERT (type_info != JERRY_TYPE_NONE);
    TEST_ASSERT (type_info == entries[idx].type_info);

    jerry_release_value (entries[idx].value);
  }

  if (jerry_is_feature_enabled (JERRY_FEATURE_SYMBOL))
  {
    jerry_value_t symbol_desc_value = jerry_create_string ((jerry_char_t *) "foo");
    jerry_value_t symbol_value = jerry_create_symbol (symbol_desc_value);
    jerry_type_t type_info = jerry_value_get_type (symbol_value);

    TEST_ASSERT (type_info != JERRY_TYPE_NONE);
    TEST_ASSERT (type_info == JERRY_TYPE_SYMBOL);

    jerry_release_value (symbol_value);
    jerry_release_value (symbol_desc_value);
  }

  jerry_cleanup ();
  free (ctx_p);
}
