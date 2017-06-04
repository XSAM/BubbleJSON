//
// Created by BangshengXie on 23/05/2017.
//

#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "BubbleJson.h"
#include "Struct.h"
#include "test_file.h"


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
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
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
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->GetType());
    delete get<1>(result);\
}

static void TestParseTrue()
{
    auto result = gm_BubbleJson.Parse("true");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_True, get<1>(result)->GetType());
    delete get<1>(result);\
}

static void TestParseFalse()
{
    auto result = gm_BubbleJson.Parse("false");
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_False, get<1>(result)->GetType());
    delete get<1>(result);\
}

static void TestParseExpectValue()
{
    tuple<ParseResults, BubbleValue*> result = gm_BubbleJson.Parse("");
    EXPECT_EQ_INT(ParseResult_ExpectValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->GetType());
    delete get<1>(result);\

    result = gm_BubbleJson.Parse(" ");
    EXPECT_EQ_INT(ParseResult_ExpectValue, get<0>(result));
    EXPECT_EQ_INT(ValueType_Null, get<1>(result)->GetType());
    delete get<1>(result);\
}

#define TEST_ERROR(error, json)\
    do {\
        result = gm_BubbleJson.Parse(json);\
        EXPECT_EQ_INT(error, get<0>(result));\
        EXPECT_EQ_INT(ValueType_Null, get<1>(result)->GetType());\
        delete get<1>(result);\
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
        EXPECT_EQ_INT(ValueType_Number, get<1>(result)->GetType());\
        EXPECT_EQ_DOUBLE(expect, get<1>(result)->GetNumber());\
        delete get<1>(result);\
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
        EXPECT_EQ_INT(ValueType_String, get<1>(result)->GetType());\
        EXPECT_EQ_STRING(expect, get<1>(result)->GetString(), get<1>(result)->GetStringLength());\
        delete get<1>(result);\
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
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(0, value->GetArrayCount());
    delete value;

    result = gm_BubbleJson.Parse("[ null , false , true , 123 , \"abc\" ]");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(5, value->GetArrayCount());

    EXPECT_EQ_INT(ValueType_Null, (*value)[0].GetType());
    EXPECT_EQ_INT(ValueType_False, (*value)[1].GetType());
    EXPECT_EQ_INT(ValueType_True, (*value)[2].GetType());
    EXPECT_EQ_INT(ValueType_Number, (*value)[3].GetType());
    EXPECT_EQ_INT(ValueType_String, (*value)[4].GetType());

    EXPECT_EQ_DOUBLE(123.0, (*value)[3].GetNumber());
    EXPECT_EQ_STRING("abc", (*value)[4].GetString(), (*value)[4].GetStringLength());
    delete value;

    //a good way
    result = gm_BubbleJson.Parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(4, value->GetArrayCount());
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ_INT(ValueType_Array, (*value)[i].GetType());
        EXPECT_EQ_SIZE_T(i, (*value)[i].GetArrayCount());
        for (int j = 0; j < i; ++j)
        {
            EXPECT_EQ_INT(ValueType_Number, (*value)[i][j].GetType());
            EXPECT_EQ_DOUBLE((double)j, (*value)[i][j].GetNumber());
        }
    }
    delete value;

    //another way
    result = gm_BubbleJson.Parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(4, value->GetArrayCount());
    for (int i = 0; i < 4; ++i)
    {
        BubbleValue* valueLevel2 = value->GetArrayElement(i);
        EXPECT_EQ_INT(ValueType_Array, valueLevel2->GetType());
        EXPECT_EQ_SIZE_T(i, valueLevel2->GetArrayCount());
        for (int j = 0; j < i; ++j)
        {
            BubbleValue* valueLevel3 = valueLevel2->GetArrayElement(j);
            EXPECT_EQ_INT(ValueType_Number, valueLevel3->GetType());
            EXPECT_EQ_DOUBLE((double)j, valueLevel3->GetNumber());
        }
    }
    delete value;
}

static void TestParseArrayInsert()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;

    result = gm_BubbleJson.Parse(" [ true, false ] ");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());

    value->SetArray(1);
    (*value)[0].SetBoolean(false);
    value->InsertArrayElementWithIndex(1);
    (*value)[1].SetArray(2);
    (*value)[1][0].SetString("level2", 6);

    //expect [ false , [ "level2" , null ] ]
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(2, value->GetArrayCount());

    EXPECT_EQ_STRING("level2", (*value)[1][0].GetString(), (*value)[1][0].GetStringLength());
    EXPECT_EQ_INT(ValueType_Null, (*value)[1][1].GetType());
    EXPECT_EQ_SIZE_T(2, (*value)[1].GetArrayCount());
    delete value;
}

static void TestParseArrayDelete()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;

    result = gm_BubbleJson.Parse(" [ true, false ] ");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Array, value->GetType());

    value->DeleteArrayElementWithIndex(0);
    value->DeleteArrayElementWithIndex(0);

    //expect [ ]
    EXPECT_EQ_INT(ValueType_Array, value->GetType());
    EXPECT_EQ_SIZE_T(0, value->GetArrayCount());

    delete value;
}

static void TestParseMissCommaOrSquareBracket()
{
    tuple<ParseResults, BubbleValue*> result;

    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1}");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[1 2}");
    TEST_ERROR(ParseResult_MissCommaOrSquareBracket, "[[1]");
}

static void TestParseObject()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;
    result = gm_BubbleJson.Parse("{ }");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Object, value->GetType());
    EXPECT_EQ_SIZE_T(0, value->GetObjectCount());
    delete value;

    //a good way
    result = gm_BubbleJson.Parse(
            " { "
                    "\"n\" : null , "
                    "\"f\" : false , "
                    "\"t\" : true , "
                    "\"i\" : 123 , "
                    "\"s\" : \"abc\", "
                    "\"a\" : [ 1, 2, 3 ],"
                    "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                    " } "
    );
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Object, value->GetType());
    EXPECT_EQ_SIZE_T(7, value->GetObjectCount());

    EXPECT_TRUE(&(*value)["n"] != nullptr);//kind of weird
    EXPECT_EQ_INT(ValueType_Null, (*value)["n"].GetType());
    EXPECT_TRUE(&(*value)["f"] != nullptr);
    EXPECT_EQ_INT(ValueType_False, (*value)["f"].GetType());
    EXPECT_TRUE(&(*value)["t"] != nullptr);
    EXPECT_EQ_INT(ValueType_True, (*value)["t"].GetType());

    EXPECT_TRUE(&(*value)["i"] != nullptr);
    EXPECT_EQ_INT(ValueType_Number, (*value)["i"].GetType());
    EXPECT_EQ_DOUBLE(123.0, (*value)["i"].GetNumber());

    EXPECT_TRUE(&(*value)["s"] != nullptr);
    EXPECT_EQ_INT(ValueType_String, (*value)["s"].GetType());
    EXPECT_EQ_STRING("abc", (*value)["s"].GetString(), (*value)["s"].GetStringLength());

    EXPECT_TRUE(&(*value)["a"] != nullptr);
    EXPECT_EQ_INT(ValueType_Array, (*value)["a"].GetType());
    EXPECT_EQ_SIZE_T(3, (*value)["a"].GetArrayCount());
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ_INT(ValueType_Number, (*value)["a"][i].GetType());
        EXPECT_EQ_DOUBLE(i + 1.0, (*value)["a"][i].GetNumber());
    }

    EXPECT_TRUE(&(*value)["o"] != nullptr);
    EXPECT_EQ_INT(ValueType_Object, (*value)["o"].GetType());
    EXPECT_EQ_SIZE_T(3, (*value)["o"].GetObjectCount());
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ_INT(ValueType_Number, (*value)["o"][to_string(i+1)].GetType());
        EXPECT_EQ_DOUBLE(i + 1.0, (*value)["o"][to_string(i+1)].GetNumber());
    }
    delete value;


    //another way
    result = gm_BubbleJson.Parse(
            " { "
            "\"n\" : null , "
            "\"f\" : false , "
            "\"t\" : true , "
            "\"i\" : 123 , "
            "\"s\" : \"abc\", "
            "\"a\" : [ 1, 2, 3 ],"
            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
            " } "
    );
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Object, value->GetType());
    EXPECT_EQ_SIZE_T(7, value->GetObjectCount());

    EXPECT_TRUE(value->GetObjectValueWithKey("n") != nullptr);
    EXPECT_EQ_INT(ValueType_Null, value->GetObjectValueWithKey("n")->GetType());
    EXPECT_TRUE(value->GetObjectValueWithKey("f") != nullptr);
    EXPECT_EQ_INT(ValueType_False, value->GetObjectValueWithKey("f")->GetType());
    EXPECT_TRUE(value->GetObjectValueWithKey("t") != nullptr);
    EXPECT_EQ_INT(ValueType_True, value->GetObjectValueWithKey("t")->GetType());

    EXPECT_TRUE(value->GetObjectValueWithKey("i") != nullptr);
    EXPECT_EQ_INT(ValueType_Number, value->GetObjectValueWithKey("i")->GetType());
    EXPECT_EQ_DOUBLE(123.0, value->GetObjectValueWithKey("i")->GetNumber());

    EXPECT_TRUE(value->GetObjectValueWithKey("s") != nullptr);
    EXPECT_EQ_INT(ValueType_String, value->GetObjectValueWithKey("s")->GetType());
    EXPECT_EQ_STRING("abc", value->GetObjectValueWithKey("s")->GetString(), value->GetObjectValueWithKey("s")->GetStringLength());

    EXPECT_TRUE(value->GetObjectValueWithKey("a") != nullptr);
    EXPECT_EQ_INT(ValueType_Array, value->GetObjectValueWithKey("a")->GetType());
    EXPECT_EQ_SIZE_T(3, value->GetObjectValueWithKey("a")->GetArrayCount());
    for (int i = 0; i < 3; ++i)
    {
        BubbleValue* valueLevel2 = value->GetObjectValueWithKey("a")->GetArrayElement(i);
        EXPECT_EQ_INT(ValueType_Number, valueLevel2->GetType());
        EXPECT_EQ_DOUBLE(i + 1.0, valueLevel2->GetNumber());
    }

    EXPECT_TRUE(value->GetObjectValueWithKey("o") != nullptr);
    EXPECT_EQ_INT(ValueType_Object, value->GetObjectValueWithKey("o")->GetType());
    EXPECT_EQ_SIZE_T(3, value->GetObjectValueWithKey("o")->GetObjectCount());
    auto objects = value->GetObjectValueWithKey("o")->GetObjects();
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_TRUE(objects->find(to_string(i+1)) != objects->end());
        EXPECT_EQ_DOUBLE(i + 1.0, objects->find(to_string(i+1))->second.GetNumber());
    }
    delete value;
}

static void TestParseObjectInsert()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;

    result = gm_BubbleJson.Parse(" { \"test\": \"string\" , \"test2\": 123 } ");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Object, value->GetType());

    value->InsertObjectElementWithKey("test");
    EXPECT_EQ_SIZE_T(2, value->GetObjectCount());

    value->InsertObjectElementWithKey("level2");
    (*value)["level2"].SetArray(3);
    (*value)["level2"][0].SetString("test", 4);
    (*value)["level2"][1].SetBoolean(true);
    (*value)["level2"][2].SetObject();
    (*value)["level2"][2].InsertObjectElementWithKey("level3");
    (*value)["level2"][2]["level3"].SetNumber(456);

    //expect { "test": "string", "test2": 123, "level2": [ "test", true , "level3": 456 ] }
    EXPECT_EQ_STRING("test", (*value)["level2"][0].GetString(), (*value)["level2"][0].GetStringLength());
    EXPECT_EQ_INT(true, (*value)["level2"][1].GetBoolean());
    EXPECT_EQ_DOUBLE(456.0, (*value)["level2"][2]["level3"].GetNumber());
    EXPECT_EQ_SIZE_T(3, (*value)["level2"].GetArrayCount());
    delete value;
}

static void TestParseObjectDelete()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;

    result = gm_BubbleJson.Parse(" { \"test\": \"string\" , \"test2\": 123 } ");
    value = get<1>(result);
    EXPECT_EQ_INT(ParseResult_Ok, get<0>(result));
    EXPECT_EQ_INT(ValueType_Object, value->GetType());

    value->DeleteObjectElementWithKey("test");
    value->DeleteObjectElementWithKey("test3");//not exist

    //expect { "test2": 123 }
    EXPECT_EQ_DOUBLE(123.0, (*value)["test2"].GetNumber());
    EXPECT_EQ_SIZE_T(1, value->GetObjectCount());
    delete value;
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
    TestParseArrayInsert();
    TestParseArrayDelete();

    TestParseMissCommaOrSquareBracket();
    TestParseObject();
    TestParseObjectInsert();
    TestParseObjectDelete();
}

static void TestStringifyObject()
{
    cout<<"TestStringifyObject() result:"<<endl;
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;

    result = gm_BubbleJson.Parse(" {\"forecast\":{\"code\":\"28\",\"date\":\"04 Jun 2017\",\"day\":\"Sun\",\"high\":\"63\",\"low\":\"51\",\"text\":\"Mostly Cloudy\"  }} ");
    value = get<1>(result);
    auto stringifyResult = gm_BubbleJson.Stringify(value, StringifyType_Beauty);
    string json = string(get<0>(stringifyResult), get<1>(stringifyResult));
    cout<<"beauty:\n"<<json<<endl<<endl;
    free (get<0>(stringifyResult));

    stringifyResult = gm_BubbleJson.Stringify(value, StringifyType_Minimum);
    json = string(get<0>(stringifyResult), get<1>(stringifyResult));
    cout<<"minimum:\n"<<json<<endl<<endl;
    //don't forget free
    free (get<0>(stringifyResult));
    delete value;
}

static void TestStringify()
{
    TestStringifyObject();
}

int main()
{
	auto start = chrono::high_resolution_clock::now();
    TestParse();
    TestStringify();
    auto end = chrono::high_resolution_clock::now();
    auto testParseElapsed = chrono::duration_cast<chrono::milliseconds>(end - start);

    TestParseFromFile();
    end = chrono::high_resolution_clock::now();
    auto totalElapsed = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout<<"unit test elapsed time:"<<testParseElapsed.count()<<" ms"<<endl;
    cout<<"total elapsed time: "<<totalElapsed.count()<<" ms"<<endl;
    printf("%d/%d (%3.2f%%) passed\n", g_TestPass, g_TestCount, g_TestPass * 100.0 / g_TestCount);

    return g_Result;
}
