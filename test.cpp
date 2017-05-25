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

static void TestParseInvalidValue()
{
    auto result = gm_BubbleJson.Parse("ull");
    EXPECT_EQ_INT(ParseResult_InvalidValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);

    result = gm_BubbleJson.Parse(".,");
    EXPECT_EQ_INT(ParseResult_InvalidValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->type);
}

static void TestParseRootNotSingular()
{
    auto result = gm_BubbleJson.Parse("true a");
    EXPECT_EQ_INT(ParseResult_RootNotSingular, get<0>(result));
    EXPECT_EQ_INT(ValueType_True, get<1>(result)->type);
}

static void TestParse()
{
    TestParseNull();
    TestParseTrue();
    TestParseFalse();
    TestParseExpectValue();
    TestParseInvalidValue();
    TestParseRootNotSingular();
}

int main()
{
    TestParse();
    printf("%d/%d (%3.2f%%) passed\n", g_TestPass, g_TestCount, g_TestPass * 100.0 / g_TestCount);

    return g_Result;
}