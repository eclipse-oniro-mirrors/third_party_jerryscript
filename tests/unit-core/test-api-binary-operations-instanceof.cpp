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

#define T(lhs, rhs, res) \
  { lhs, rhs, res }

typedef struct
{
  jerry_value_t lhs;
  jerry_value_t rhs;
  bool expected;
} test_entry_t;

static jerry_value_t
my_constructor (const jerry_value_t func_val, /**< function */
                const jerry_value_t this_val, /**< this */
                const jerry_value_t argv[], /**< arguments */
                const jerry_length_t argc) /**< number of arguments */
{
  (void) func_val;
  (void) this_val;
  (void) argv;
  (void) argc;
  return jerry_create_undefined ();
} /* my_constructor */

class ApiBinaryOperationsInstanceofTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ApiBinaryOperationsInstanceofTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ApiBinaryOperationsInstanceofTest TearDownTestCase";
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
HWTEST_F(ApiBinaryOperationsInstanceofTest, Test001, testing::ext::TestSize.Level1)
{
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  TEST_INIT ();

  jerry_init (JERRY_INIT_EMPTY);

  jerry_value_t base_obj = jerry_create_object ();
  jerry_value_t constructor = jerry_create_external_function (my_constructor);

  jerry_value_t no_proto_instance_val = jerry_construct_object (constructor, NULL, 0);

  jerry_value_t prototype_str = jerry_create_string ((const jerry_char_t *) "prototype");
  jerry_value_t res = jerry_set_property (constructor, prototype_str, base_obj);
  jerry_release_value (prototype_str);
  TEST_ASSERT (!jerry_value_is_error (res));
  jerry_release_value (res);

  jerry_value_t instance_val = jerry_construct_object (constructor, NULL, 0);

  jerry_value_t error = jerry_create_error_from_value (base_obj, false);

  test_entry_t bool_tests[] =
  {
    T (jerry_acquire_value (instance_val), jerry_acquire_value (constructor), true),
    T (jerry_acquire_value (no_proto_instance_val), jerry_acquire_value (constructor), false),
    T (jerry_acquire_value (base_obj), jerry_acquire_value (constructor), false)
  };

  for (uint32_t idx = 0; idx < sizeof (bool_tests) / sizeof (test_entry_t); idx++)
  {
    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF,
                                                   bool_tests[idx].lhs,
                                                   bool_tests[idx].rhs);
    TEST_ASSERT (!jerry_value_is_error (result));
    TEST_ASSERT (jerry_get_boolean_value (result) == bool_tests[idx].expected);
    jerry_release_value (bool_tests[idx].lhs);
    jerry_release_value (bool_tests[idx].rhs);
    jerry_release_value (result);
  }

  test_entry_t error_tests[] =
  {
    T (jerry_acquire_value (constructor), jerry_acquire_value (instance_val), true),
    T (jerry_create_undefined (), jerry_acquire_value (constructor), true),
    T (jerry_acquire_value (instance_val), jerry_create_undefined (), true),
    T (jerry_acquire_value (instance_val), jerry_acquire_value (base_obj), true),
    T (jerry_acquire_value (error), jerry_acquire_value (constructor), true),
    T (jerry_acquire_value (instance_val), jerry_acquire_value (error), true),
    T (jerry_create_string ((const jerry_char_t *) ""), jerry_create_string ((const jerry_char_t *) ""), true),
    T (jerry_create_string ((const jerry_char_t *) ""), jerry_create_number (5.0), true),
    T (jerry_create_number (5.0), jerry_create_string ((const jerry_char_t *) ""), true),
    T (jerry_create_array (1), jerry_create_array (1), true),
    T (jerry_create_array (1), jerry_create_object (), true),
    T (jerry_create_object (), jerry_create_array (1), true),
    T (jerry_create_null (), jerry_create_object (), true),
    T (jerry_create_object (), jerry_create_string ((const jerry_char_t *) ""), true)
  };

  for (uint32_t idx = 0; idx < sizeof (error_tests) / sizeof (test_entry_t); idx++)
  {
    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF,
                                                   error_tests[idx].lhs,
                                                   error_tests[idx].rhs);
    TEST_ASSERT (jerry_value_is_error (result) == error_tests[idx].expected);
    jerry_release_value (error_tests[idx].lhs);
    jerry_release_value (error_tests[idx].rhs);
    jerry_release_value (result);
  }

  jerry_release_value (base_obj);
  jerry_release_value (constructor);
  jerry_release_value (error);
  jerry_release_value (instance_val);
  jerry_release_value (no_proto_instance_val);

  jerry_cleanup ();
  free (ctx_p);
}
