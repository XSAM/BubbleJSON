//
// Created by BangshengXie on 23/05/2017.
//

#include "BubbleJson.h"
#include <cerrno>
#include <cmath>
#include <iostream>

using namespace bubbleJson;

BubbleJson::BubbleJson()
{
    this->content = new BubbleContent();
    this->value = new BubbleValue();
}

BubbleJson::~BubbleJson()
{
    delete this->content;
    delete this->value;
}

tuple<ParseResults, BubbleValue *> BubbleJson::Parse(const char *json)
{
    this->content->json = json;
    this->value->type = ValueType_Null;

    ParseWhitespace();
    ParseResults result = ParseValue();
    if (result == ParseResult_Ok)
    {
        ParseWhitespace();
        if (*this->content->json != '\0')
        {
            this->value->type = ValueType_Null;
            result = ParseResult_RootNotSingular;
        }
    }
    return make_pair(result, this->value);
}

inline void BubbleJson::Expect(const char expectChar)
{
    assert(*this->content->json == expectChar);
    this->content->json++;
}

void BubbleJson::ParseWhitespace()
{
    const char *p = this->content->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
        p++;
    this->content->json = p;
}

ParseResults BubbleJson::ParseNumber()
{
    const char *p = this->content->json;

    auto IsDigit1To9 = [&p]()->bool {
        return (*p >= '1' && *p <= '9');
    };
    auto IsDigit = [&p]()->bool {
        return (*p >= '0' && *p <= '9');
    };

    if (*p == '-') p++;
    if (*p == '0') p++;
    else
    {
        if (!IsDigit1To9()) return ParseResult_InvalidValue;
        p++;
        while (IsDigit()) { p++; }
    }
    if (*p == '.')
    {
        p++;
        if (!IsDigit()) return ParseResult_InvalidValue;
        p++;
        while (IsDigit()) { p++; }
    }
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-') p++;
        while (IsDigit()) { p++; }
    }

    errno = 0;
    this->value->number = strtod(this->content->json, NULL);
    if (errno == ERANGE && (this->value->number == HUGE_VAL || this->value->number == -HUGE_VAL))
        return ParseResult_NumberTooBig;
    this->value->type = ValueType_Number;
    this->content->json = p;

    return ParseResult_Ok;
}

ParseResults BubbleJson::ParseValue()
{
    switch (*this->content->json)
    {
        case 't':
            return ParseLiteral("true", ValueType_True);
        case 'f':
            return ParseLiteral("false", ValueType_False);
        case 'n':
            return ParseLiteral("null", ValueType_Null);
        case '\0':
            return ParseResult_ExpectValue;
        default:
            return ParseNumber();
    }
}

ParseResults BubbleJson::ParseLiteral(const char *expectJson, ValueTypes expectResult)
{
    Expect(expectJson[0]);

    const char *json = this->content->json;
    int i;
    //因为Expect里面json会自加1，所以json比expectJson快进一个字符
    for (i = 0; expectJson[i + 1]; i++)
    {
        if (json[i] != expectJson[i + 1])
            return ParseResult_InvalidValue;
    }
    this->content->json += i;
    this->value->type = expectResult;
    return ParseResult_Ok;
}

