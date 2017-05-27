//
// Created by BangshengXie on 23/05/2017.
//

#ifndef BUBBLEJSON_STRUCT_H
#define BUBBLEJSON_STRUCT_H


#include <cstdio>

namespace bubbleJson {

enum ParseResults
{
    ParseResult_Ok,
    ParseResult_ExpectValue,
    ParseResult_InvalidValue,
    ParseResult_RootNotSingular,
    ParseResult_NumberTooBig,
    ParseResult_InvalidStringChar,
    ParseResult_InvalidStringEscape,
    ParseResult_MissQuotationMark,
    ParseResult_InvalidUnicodeHex,
    ParseResult_InvalidUnicodeSurrogate
};

enum ValueTypes
{
    ValueType_Null,
    ValueType_False,
    ValueType_True,
    ValueType_Number,
    ValueType_String,
    ValueType_Array,
    ValueType_Object
};

struct BubbleContext
{
    const char* json;
    char* stack;
    size_t size, top;
};

struct BubbleValue
{
    union
    {
        double number;
        struct { char* literal; size_t length;} string;
    }u;
    ValueTypes type;
};

}


#endif //BUBBLEJSON_STRUCT_H
