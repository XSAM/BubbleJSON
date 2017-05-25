//
// Created by BangshengXie on 23/05/2017.
//

#ifndef BUBBLEJSON_STRUCT_H
#define BUBBLEJSON_STRUCT_H


namespace bubbleJson {

enum ParseResults
{
    ParseResult_Ok,
    ParseResult_ExpectValue,
    ParseResult_InvalidValue,
    ParseResult_RootNotSingular,
    ParseResult_NumberTooBig
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

struct BubbleContent
{
    const char* json;
};

struct BubbleValue
{
    double number;
    ValueTypes type;
};

}


#endif //BUBBLEJSON_STRUCT_H
