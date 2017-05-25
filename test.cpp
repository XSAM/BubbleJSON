//
// Created by BangshengXie on 23/05/2017.
//

#include <iostream>
#include <cassert>
#include "BubbleJson.h"


using namespace std;
using namespace bubbleJson;

static int g_Result;
static int g_TestCount;
static int g_TestPass;

static BubbleJson gm_BubbleJson;

#define EXPECT_EQ_BASE(expect, actual, format)\
    do {\
        g_TestCount++;\
        bool equality = (expect) == (actual);\
        if (equality)\
            g_TestPass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            g_Result = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE(expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(expect, actual, "%.17g")

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

    TEST_ERROR(ParseResult_NumberTooBig, "2e200");
    TEST_ERROR(ParseResult_NumberTooBig, "1E309");
    TEST_ERROR(ParseResult_NumberTooBig, "-1E309");
}

#define TEST_NUMBER(expect, json)\
    do {\
        result = gm_BubbleJson.Parse(json);\
        EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));\
        EXPECT_EQ_INT(ValueType_Number, get<1>(result)->type);\
        EXPECT_EQ_DOUBLE(expect, get<1>(result)->number);\
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
}

int main()
{
    TestParse();
    printf("%d/%d (%3.2f%%) passed\n", g_TestPass, g_TestCount, g_TestPass * 100.0 / g_TestCount);

    return g_Result;
}