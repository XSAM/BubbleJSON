//
// Created by BangshengXie on 23/05/2017.
//

#include "BubbleJson.h"
#include "Struct.h"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <iostream>

using namespace bubbleJson;

BubbleJson::BubbleJson()
{
    this->context = new BubbleContext();
    this->value = new BubbleValue();
}

BubbleJson::~BubbleJson()
{
    MemoryFreeContextStack();
    delete this->context;
    MemoryFreeBubbleValue(this->value);
    delete this->value;
}

tuple<ParseResults, BubbleValue *> BubbleJson::Parse(const char *json)
{
    BubbleValue* bubbleValue = this->value;
    MemoryFreeBubbleValue(bubbleValue);
    InitBubbleValue(bubbleValue);
    MemoryFreeContextStack();

    this->context->json = json;

    ParseWhitespace();
    ParseResults result = ParseValue(bubbleValue);
    if (result == ParseResult_Ok)
    {
        ParseWhitespace();
        if (*this->context->json != '\0')
        {
            bubbleValue->type = ValueType_Null;
            result = ParseResult_RootNotSingular;
        }
    }
    return make_pair(result, bubbleValue);
}

inline void BubbleJson::InitBubbleValue(BubbleValue *bubbleValue)
{
    bubbleValue->type = ValueType_Null;
}

inline void BubbleJson::Expect(const char expectChar)
{
    assert(*this->context->json == expectChar);
    this->context->json++;
}

void BubbleJson::ParseWhitespace()
{
    const char *p = this->context->json;
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
        p++;
    this->context->json = p;
}

ParseResults BubbleJson::ParseNumber(BubbleValue *bubbleValue)
{
    const char* p = this->context->json;

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
    bubbleValue->u.number = strtod(this->context->json, NULL);
    if (errno == ERANGE && (bubbleValue->u.number == HUGE_VAL || bubbleValue->u.number == -HUGE_VAL))
        return ParseResult_NumberTooBig;
    bubbleValue->type = ValueType_Number;
    this->context->json = p;

    return ParseResult_Ok;
}

ParseResults BubbleJson::ParseString(BubbleValue *bubbleValue)
{
    BubbleContext* c = this->context;
    size_t originTop = c->top;
    size_t lenght;
    unsigned highSurrogate, lowSurrogate;

    auto ReturnError = [=,&c](ParseResults parseResult){
        c->top = originTop;
        return parseResult;
    };

    Expect('\"');
    const char* p = c->json;
    while (true)
    {
        //traverse json
        char ch = *p;
        p++;
        switch (ch)
        {
            case '\"':
                lenght = c->top - originTop;
                bubbleValue->SetString((const char *) BubbleContextPop(lenght), lenght);
                //current p point '\"' character
                c->json = p;
                return ParseResult_Ok;
            case '\\':
                switch (*p++) {
                    case '\"': BubbleContextPushChar('\"'); break;
                    case '\\': BubbleContextPushChar('\\'); break;
                    case '/':  BubbleContextPushChar('/' ); break;
                    case 'b':  BubbleContextPushChar('\b'); break;
                    case 'f':  BubbleContextPushChar('\f'); break;
                    case 'n':  BubbleContextPushChar('\n'); break;
                    case 'r':  BubbleContextPushChar('\r'); break;
                    case 't':  BubbleContextPushChar('\t'); break;
                    case 'u':
                        if (!(p = ParseHexToInt(p, &highSurrogate)))
                            return ReturnError(ParseResult_InvalidUnicodeHex);
                        if (highSurrogate >= 0xD800 && highSurrogate <= 0xDBFF)//surrogate pair
                        {
                            if (*p++ != '\\')
                                return ReturnError(ParseResult_InvalidUnicodeSurrogate);
                            if (*p++ != 'u')
                                return ReturnError(ParseResult_InvalidUnicodeSurrogate);
                            if (!(p = ParseHexToInt(p, &lowSurrogate)))
                                return ReturnError(ParseResult_InvalidUnicodeHex);
                            if (lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF)
                                return ReturnError(ParseResult_InvalidUnicodeSurrogate);
                            highSurrogate = (((highSurrogate - 0xD800) << 10) | (lowSurrogate - 0xDC00)) + 0x10000;
                        }
                        EncodeUTF8(highSurrogate);
                        break;
                    default:
                        c->top = originTop;
                        return ParseResult_InvalidStringEscape;
                }
                break;
            case '\0':
                c->top = originTop;
                return ParseResult_MissQuotationMark;
            default:
                if (ch < 0x20)
                {
                    c->top = originTop;
                    return ParseResult_InvalidStringChar;
                }
                BubbleContextPushChar(ch);
        }
    }
}

ParseResults BubbleJson::ParseValue(BubbleValue *bubbleValue)
{
    switch (*this->context->json)
    {
        case 't':   return ParseLiteral(bubbleValue, "true", ValueType_True);
        case 'f':   return ParseLiteral(bubbleValue, "false", ValueType_False);
        case 'n':   return ParseLiteral(bubbleValue, "null", ValueType_Null);
        case '\"':  return ParseString(bubbleValue);
        case '[':   return ParseArray(bubbleValue);
        case '\0':  return ParseResult_ExpectValue;
        default:
            return ParseNumber(bubbleValue);
    }
}

ParseResults BubbleJson::ParseLiteral(BubbleValue *bubbleValue, const char *expectJson, ValueTypes expectResult)
{
    Expect(expectJson[0]);

    const char *json = this->context->json;
    int i;
    //因为Expect里面json会自加1，所以json比expectJson快进一个字符
    for (i = 0; expectJson[i + 1]; i++)
    {
        if (json[i] != expectJson[i + 1])
            return ParseResult_InvalidValue;
    }
    this->context->json += i;
    bubbleValue->type = expectResult;
    return ParseResult_Ok;
}

void BubbleJson::MemoryFreeBubbleValue(BubbleValue *bubbleValue)
{
    assert(bubbleValue != nullptr);
    switch (bubbleValue->type)
    {
        case ValueType_String:
            free(bubbleValue->u.string.literal);
            bubbleValue->u.string.literal = nullptr;
            break;
        case ValueType_Array:
            for (int i = 0; i < bubbleValue->u.array.count; ++i)
            {
                MemoryFreeBubbleValue(&bubbleValue->u.array.elements[i]);
            }
            free(bubbleValue->u.array.elements);
            bubbleValue->u.array.elements = nullptr;
            break;
        default:
            break;
    }

    bubbleValue->type = ValueType_Null;
}

void BubbleJson::MemoryFreeContextStack()
{
    if (this->context->stack != nullptr)
    {
        this->context->top = 0;
        this->context->size = 0;
        free(this->context->stack);
        this->context->stack = nullptr;
    }
}

inline void BubbleJson::BubbleContextPushChar(char ch)
{
    *(char*)BubbleContextPush(sizeof(ch)) = ch;
}

void* BubbleJson::BubbleContextPush(size_t size)
{
    void* result;
    assert(size > 0);
    BubbleContext* c = this->context;

    //over stack count if need
    if (c->top + size >= c->size)
    {
        //need first malloc
        if (c->size == 0)
            c->size = stackInitSize;

        while (c->top + size >= c->size)
            c->size += c->size >> 1;//c->count * 1.5
        c->stack = (char*)realloc(c->stack, c->size);
    }

    //return the memory address should be push
    result = c->stack + c->top;
    //move the top pointer to the stack top
    c->top += size;
    return result;
}

void* BubbleJson::BubbleContextPop(size_t size)
{
    assert(this->context->top >= size);
    this->context->top -= size;
    return this->context->stack + this->context->top;
}

const char * BubbleJson::ParseHexToInt(const char *ch, unsigned *number)
{
    *number = 0;

    for (int i = 0; i < 4; i++)
    {
        *number <<= 4;
        if      (*ch >= '0' && *ch <= '9') *number |= (unsigned)*ch - '0';
        else if (*ch >= 'A' && *ch <= 'F') *number |= (unsigned)*ch - 'A' + 10;//'A' meaning 10 in hex
        else if (*ch >= 'a' && *ch <= 'f') *number |= (unsigned)*ch - 'a' + 10;
        else return nullptr;
        ch++;
    }
    return ch;
}

void BubbleJson::EncodeUTF8(unsigned number)
{
    if (number <= 0x7F)
        BubbleContextPushChar(number & 0xFF);
    else if (number <= 0x7FF)
    {
        BubbleContextPushChar(0xC0 | ((number >> 6) & 0xFF));
        BubbleContextPushChar(0x80 | ((number     ) & 0x3F));
    }
    else if (number <= 0xFFFF)
    {
        BubbleContextPushChar(0xE0 | ((number >> 12) & 0xFF));
        BubbleContextPushChar(0x80 | ((number >> 6) & 0x3F));
        BubbleContextPushChar(0x80 | ((number     ) & 0x3F));
    }
    else if (number <= 0x10FFFF)
    {
        BubbleContextPushChar(0xF0 | ((number >> 18) & 0xFF));
        BubbleContextPushChar(0x80 | ((number >> 12) & 0x3F));
        BubbleContextPushChar(0x80 | ((number >> 6) & 0x3F));
        BubbleContextPushChar(0x80 | ((number     ) & 0x3F));
    }
}

ParseResults BubbleJson::ParseArray(BubbleValue *bubbleValue)
{
    BubbleContext* c = this->context;
    ParseResults result;
    size_t count = 0;

    Expect('[');
    ParseWhitespace();
    if (*c->json == ']')
    {
        bubbleValue->type = ValueType_Array;
        bubbleValue->u.array.count = 0;
        bubbleValue->u.array.elements = nullptr;
        c->json++;
        return ParseResult_Ok;
    }
    while (true)
    {
        BubbleValue valueTmp;
        InitBubbleValue(&valueTmp);
        if ((result = ParseValue(&valueTmp)) != ParseResult_Ok)
            return result;
        count++;
        memcpy(BubbleContextPush(sizeof(valueTmp)), &valueTmp, sizeof(valueTmp));
        ParseWhitespace();
        if (*c->json == ']')
        {
            c->json++;
            bubbleValue->type = ValueType_Array;
            bubbleValue->u.array.count = count;
            size_t size = count * sizeof(BubbleValue);//same size
            memcpy(bubbleValue->u.array.elements = (BubbleValue*)malloc(size), BubbleContextPop(size), size);
            return ParseResult_Ok;
        }
        else if (*c->json == ',')
        {
            c->json++;
            ParseWhitespace();
        }
        else
            return ParseResult_MissCommaOrSquareBracket;
    }
}



