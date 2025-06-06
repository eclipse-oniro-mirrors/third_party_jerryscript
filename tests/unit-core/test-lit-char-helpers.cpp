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
extern "C"
{
  #include "ecma-init-finalize.h"
  #include "jmem.h"
  #include "lit-char-helpers.h"
}

#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include "ecma-helpers.h"
#include "lit-strings.h"
#include "js-parser-internal.h"
#include "test-common.h"
#include <gtest/gtest.h>

static lit_code_point_t
lexer_hex_to_character (const uint8_t *source_p) /**< current source position */
{
  lit_code_point_t result = 0;

  do
  {
    uint32_t byte = *source_p++;

    result <<= 4;

    if (byte >= LIT_CHAR_0 && byte <= LIT_CHAR_9)
    {
      result += byte - LIT_CHAR_0;
    }
    else
    {
      byte = LEXER_TO_ASCII_LOWERCASE (byte);
      if (byte >= LIT_CHAR_LOWERCASE_A && byte <= LIT_CHAR_LOWERCASE_F)
      {
        result += byte - (LIT_CHAR_LOWERCASE_A - 10);
      }
      else
      {
        return UINT32_MAX;
      }
    }
  }
  while (*source_p);

  return result;
} /* lexer_hex_to_character */

class LitCharHelpersTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "LitCharHelpersTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "LitCharHelpersTest TearDownTestCase";
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
HWTEST_F(LitCharHelpersTest, Test001, testing::ext::TestSize.Level1)
{
  TEST_INIT ();
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  jmem_init ();
  ecma_init ();

  const uint8_t _1_byte_long1[] = "007F";
  const uint8_t _1_byte_long2[] = "0000";
  const uint8_t _1_byte_long3[] = "0065";

  const uint8_t _2_byte_long1[] = "008F";
  const uint8_t _2_byte_long2[] = "00FF";
  const uint8_t _2_byte_long3[] = "07FF";

  const uint8_t _3_byte_long1[] = "08FF";
  const uint8_t _3_byte_long2[] = "0FFF";
  const uint8_t _3_byte_long3[] = "FFFF";

  const uint8_t _6_byte_long1[] = "10000";
  const uint8_t _6_byte_long2[] = "10FFFF";

  size_t length;

  /* Test 1-byte-long unicode sequences. */
  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_1_byte_long1));
  TEST_ASSERT (length == 1);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_1_byte_long2));
  TEST_ASSERT (length == 1);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_1_byte_long3));
  TEST_ASSERT (length == 1);

  /* Test 2-byte-long unicode sequences. */
  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_2_byte_long1));
  TEST_ASSERT (length == 2);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_2_byte_long2));
  TEST_ASSERT (length == 2);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_2_byte_long3));
  TEST_ASSERT (length == 2);

  /* Test 3-byte-long unicode sequences. */
  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_3_byte_long1));
  TEST_ASSERT (length == 3);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_3_byte_long2));
  TEST_ASSERT (length == 3);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_3_byte_long3));
  TEST_ASSERT (length == 3);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_6_byte_long1));
  TEST_ASSERT (length == 6);

  length = lit_code_point_get_cesu8_length (lexer_hex_to_character (_6_byte_long2));
  TEST_ASSERT (length == 6);

  ecma_finalize ();
  jmem_finalize ();
  free (ctx_p);
  return;
}
