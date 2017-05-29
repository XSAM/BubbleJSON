//
// Created by BangshengXie on 23/05/2017.
//

#include <iostream>
#include <cassert>
#include <cstring>
#include "BubbleJson.h"
#include "Struct.h"


using namespace std;
using namespace bubbleJson;

static int g_Result;
static int g_TestCount;
static int g_TestPass;

static BubbleJson gm_BubbleJson;

#define EXPECT_EQ_BASE(equality, expect, actual, format)\
    do {\
        g_TestCount++;\
        if (equality)\
            g_TestPass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            g_Result = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

static void TestParseNull()
{
    auto result = gm_BubbleJson.Parse("null");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);
}

static void TestParseTrue()
{
    auto result = gm_BubbleJson.Parse("true");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_True, get<1>(result)->type);
}

static void TestParseFalse()
{
    auto result = gm_BubbleJson.Parse("false");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_False, get<1>(result)->type);
}

static void TestParseExpectValue()
{
    tuple<ParseResults, BubbleValue*> result = gm_BubbleJson.Parse("");
    EXPECT_EQ_INT(ParseResult_ExpectValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);

    result = gm_BubbleJson.Parse(" ");
    EXPECT_EQ_INT(ParseResult_ExpectValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);
}

#define TEST_ERROR(error, json)\
    do {\
        result = gm_BubbleJson.Parse(json);\
        EXPECT_EQ_INT(error, get<0>(result));\
        EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);\
    } while(0)

static void TestParseInvalidValue()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_InvalidValue, "ull");
    TEST_ERROR(ParseResult_InvalidValue, "qwe");

    //invalid number
    TEST_ERROR(ParseResult_InvalidValue, "+0");
    TEST_ERROR(ParseResult_InvalidValue, "+1");
    TEST_ERROR(ParseResult_InvalidValue, ".123");
    TEST_ERROR(ParseResult_InvalidValue, "0.");
    TEST_ERROR(ParseResult_InvalidValue, "INF");

    //invalid value in array
    TEST_ERROR(ParseResult_InvalidValue, "[1,]");
    TEST_ERROR(ParseResult_InvalidValue, "[\"a\", nul]");
}

static void TestParseRootNotSingular()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_RootNotSingular, "true a");

    //invalid number
    TEST_ERROR(ParseResult_RootNotSingular, "12a");
    TEST_ERROR(ParseResult_RootNotSingular, "0123");
}

static void TestParseNumberTooBig()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_NumberTooBig, "1E309");
    TEST_ERROR(ParseResult_NumberTooBig, "-1E309");
}

#define TEST_NUMBER(expect, json)\
    do {\
        result = gm_BubbleJson.Parse(json);\
        EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));\
        EXPECT_EQ_INT(ValueType_Number, get<1>(result)->type);\
        EXPECT_EQ_DOUBLE(expect, get<1>(result)->u.number);\
    } while(0)

static void TestParseNumber() {
    tuple<ParseResults, BubbleValue*> result;

    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1e12, "1E012");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");

    TEST_NUMBER(2e200, "2e200");
}

#define TEST_STRING(expect, json)\
    do {\
        result = gm_BubbleJson.Parse(json);\
        EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));\
        EXPECT_EQ_INT(ValueType_String, get<1>(result)->type);\
        EXPECT_EQ_STRING(expect, get<1>(result)->u.string.literal, get<1>(result)->u.string.length);\
    } while(0)

static void TestParseString()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
    TEST_STRING("测试", "\"\\u6D4B\\u8BD5\"");
}

static void TestParseInvalidUnicodeHex()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u0\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u01\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u012\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u/000\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\uG000\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u0/00\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u0G00\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u0/00\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u00G0\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u000/\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u000G\"");
    TEST_ERROR(ParseResult_InvalidUnicodeHex, "\"\\u 123\"");
}

static void TestParseInvalidUnicodeSurrogate()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_InvalidUnicodeSurrogate, "\"\\uD800\"");
    TEST_ERROR(ParseResult_InvalidUnicodeSurrogate, "\"\\uDBFF\"");
    TEST_ERROR(ParseResult_InvalidUnicodeSurrogate, "\"\\uD800\\\\\"");
    TEST_ERROR(ParseResult_InvalidUnicodeSurrogate, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(ParseResult_InvalidUnicodeSurrogate, "\"\\uD800\\uE000\"");
}

static void TestParseMissingQuotationMark()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_MissQuotationMark, "\"");
    TEST_ERROR(ParseResult_MissQuotationMark, "\"abc");
}

static void TestParseInvalidStringEscape()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_InvalidStringEscape, "\"\\v\"");
    TEST_ERROR(ParseResult_InvalidStringEscape, "\"\\'\"");
    TEST_ERROR(ParseResult_InvalidStringEscape, "\"\\0\"");
    TEST_ERROR(ParseResult_InvalidStringEscape, "\"\\x12\"");
}

static void TestParseInvalidStringChar()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_InvalidStringChar, "\"\x19");
    TEST_ERROR(ParseResult_InvalidStringChar, "\"\x1");
}

static void TestParseArray()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;
    result = gm_BubbleJson.Parse("[ ]");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, get<1>(result)->type);
    EXPECT_EQ_SIZE_T(0, get<1>(result)->u.array.count);

    result = gm_BubbleJson.Parse("[ null , false , true , 123 , \"abc\" ]");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, get<1>(result)->type);
    EXPECT_EQ_SIZE_T(5, get<1>(result)->u.array.count);

    EXPECT_EQ_INT(ValueType_Null, gm_BubbleJson.GetArrayElement(value, 0)->type);
    EXPECT_EQ_INT(ValueType_False, gm_BubbleJson.GetArrayElement(value, 1)->type);
    EXPECT_EQ_INT(ValueType_True, gm_BubbleJson.GetArrayElement(value, 2)->type);
    EXPECT_EQ_INT(ValueType_Number, gm_BubbleJson.GetArrayElement(value, 3)->type);
    EXPECT_EQ_INT(ValueType_String, gm_BubbleJson.GetArrayElement(value, 4)->type);

    EXPECT_EQ_DOUBLE(123.0, gm_BubbleJson.GetArrayElement(value, 3)->u.number);
    EXPECT_EQ_STRING("abc", gm_BubbleJson.GetArrayElement(value, 4)->u.string.literal, gm_BubbleJson.GetArrayElement(value, 4)->u.string.length);


    result = gm_BubbleJson.Parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, get<1>(result)->type);
    EXPECT_EQ_SIZE_T(4, get<1>(result)->u.array.count);
    for (int i = 0; i < 4; ++i)
    {
        BubbleValue* valueLevel2 = gm_BubbleJson.GetArrayElement(value, i);
        EXPECT_EQ_INT(ValueType_Array, valueLevel2->type);
        EXPECT_EQ_SIZE_T(i, valueLevel2->u.array.count);
        for (int j = 0; j < i; ++j)
        {
            BubbleValue* valueLevel3 = gm_BubbleJson.GetArrayElement(valueLevel2, j);
            EXPECT_EQ_INT(ValueType_Number, valueLevel3->type);
            EXPECT_EQ_DOUBLE((double)j, valueLevel3->u.number);
        }
    }
}

static void TestParseMissCommaOrSquareBracket()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1}");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1 2}");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[[1]");
}

static void TestParse()
{
    TestParseNull();
    TestParseTrue();
    TestParseFalse();
    TestParseExpectValue();
    TestParseInvalidValue();
    TestParseRootNotSingular();
    TestParseNumber();
    TestParseNumberTooBig();
    TestParseString();
    TestParseMissingQuotationMark();
    TestParseInvalidStringEscape();
    TestParseInvalidStringChar();
    TestParseInvalidUnicodeHex();
    TestParseInvalidUnicodeSurrogate();
    TestParseArray();
    TestParseMissCommaOrSquareBracket();
}

int main()
{
    TestParse();
    printf("%d/%d (%3.2f%%) passed\n", g_TestPass, g_TestCount, g_TestPass * 100.0 / g_TestCount);

    return g_Result;
}