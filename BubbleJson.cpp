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
    delete this->context;
    delete this->value;
}

tuple<ParseResults, BubbleValue *> BubbleJson::Parse(const char *json)
{
    InitBubbleValue();
    MemoryFreeValueString();
    MemoryFreeContextStack();

    this->context->json = json;
    this->value->type = ValueType_Null;

    ParseWhitespace();
    ParseResults result = ParseValue();
    if (result == ParseResult_Ok)
    {
        ParseWhitespace();
        if (*this->context->json != '\0')
        {
            this->value->type = ValueType_Null;
            result = ParseResult_RootNotSingular;
        }
    }
    return make_pair(result, this->value);
}

inline void BubbleJson::InitBubbleValue()
{
    this->value->type = ValueType_Null;
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

ParseResults BubbleJson::ParseNumber()
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
    this->value->u.number = strtod(this->context->json, NULL);
    if (errno == ERANGE && (this->value->u.number == HUGE_VAL || this->value->u.number == -HUGE_VAL))
        return ParseResult_NumberTooBig;
    this->value->type = ValueType_Number;
    this->context->json = p;

    return ParseResult_Ok;
}

ParseResults BubbleJson::ParseString()
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
                SetString((const char*)BubbleContextPop(lenght),lenght);
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
                        if (highSurrogate >= 0xD800 && highSurrogate <= 0xD8FF)//surrogate pair
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

ParseResults BubbleJson::ParseValue()
{
    switch (*this->context->json)
    {
        case 't':   return ParseLiteral("true", ValueType_True);
        case 'f':   return ParseLiteral("false", ValueType_False);
        case 'n':   return ParseLiteral("null", ValueType_Null);
        case '\0':  return ParseResult_ExpectValue;
        case '\"':  return ParseString();
        default:
            return ParseNumber();
    }
}

ParseResults BubbleJson::ParseLiteral(const char *expectJson, ValueTypes expectResult)
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
    this->value->type = expectResult;
    return ParseResult_Ok;
}

void BubbleJson::MemoryFreeValueString()
{
    if(this->value->type == ValueType_String)
    {
        free(this->value->u.string.literal);
        this->value->u.string.literal = nullptr;
    }

    this->value->type = ValueType_Null;
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

void BubbleJson::SetNull()
{
    MemoryFreeValueString();
}

bool BubbleJson::GetBoolean()
{
    assert(this->value->type == ValueType_True || this->value->type == ValueType_False);
    return this->value->type == ValueType_True;
}

void BubbleJson::SetBoolean(int boolean)
{
    MemoryFreeValueString();
    this->value->type = boolean ? ValueType_True : ValueType_False;
}

double BubbleJson::GetNumber()
{
    assert(this->value->type == ValueType_Number);
    return this->value->u.number;
}

void BubbleJson::SetNumber(double number)
{
    MemoryFreeValueString();
    this->value->u.number = number;
    this->value->type = ValueType_Number;
}

const char * BubbleJson::GetString()
{
    assert(this->value->type == ValueType_String
           && this->value->u.string.literal != nullptr);
    return this->value->u.string.literal;
}

void BubbleJson::SetString(const char *string, size_t length)
{
    //allow empty string
    assert(string != nullptr || length == 0);
    MemoryFreeValueString();
    BubbleValue* v = this->value;

    v->u.string.literal = (char*)malloc(length + 1);//include \0
    memcpy(v->u.string.literal, string, length);
    v->u.string.literal[length] = '\0';
    v->u.string.length = length;

    v->type = ValueType_String;
}

size_t BubbleJson::GetStringLength()
{
    assert(this->value->type == ValueType_String
           && this->value->u.string.literal != nullptr);
    return this->value->u.string.length;
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

    //over stack size if need
    if (c->top + size >= c->size)
    {
        //need first malloc
        if (c->size == 0)
            c->size = stackInitSize;

        while (c->top + size >= c->size)
            c->size += c->size >> 1;//c->size * 1.5
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



