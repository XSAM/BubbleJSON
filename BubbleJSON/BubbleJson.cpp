//
// Created by BangshengXie on 23/05/2017.
//

#include "BubbleJson.h"
#include "Struct.h"
#include "BubbleValue.h"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <iostream>

using namespace bubbleJson;
using namespace std;

BubbleJson::BubbleJson()
{
    this->context = new BubbleContext();
}

BubbleJson::~BubbleJson()
{
    MemoryFreeContextStack();
    delete this->context;
}

tuple<ParseResults, BubbleValue *> BubbleJson::Parse(const char *json)
{
    BubbleValue* bubbleValue = new BubbleValue();
    bubbleValue->isRoot = true;
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

std::tuple<ParseResults, BubbleValue *> BubbleJson::Parse(const std::string json)
{
    return Parse(json.c_str());
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

ParseResults BubbleJson::ParseStringRaw(char **refString, size_t *refLength)
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
                *refString = (char *) ContextPop(lenght);
                *refLength = lenght;
                //current p point '\"' character
                c->json = p;
                return ParseResult_Ok;
            case '\\':
                switch (*p++) {
                    case '\"':
                        ContextPushChar('\"'); break;
                    case '\\':
                        ContextPushChar('\\'); break;
                    case '/':
                        ContextPushChar('/'); break;
                    case 'b':
                        ContextPushChar('\b'); break;
                    case 'f':
                        ContextPushChar('\f'); break;
                    case 'n':
                        ContextPushChar('\n'); break;
                    case 'r':
                        ContextPushChar('\r'); break;
                    case 't':
                        ContextPushChar('\t'); break;
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
                ContextPushChar(ch);
        }
    }
}

ParseResults BubbleJson::ParseString(BubbleValue *bubbleValue)
{
    ParseResults result;
    char* string;
    size_t length;
    if ((result = ParseStringRaw(&string, &length)) == ParseResult_Ok)
        bubbleValue->SetString(string, length);
    return result;
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
        case '{':   return ParseObject(bubbleValue);
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

inline void BubbleJson::ContextPushChar(char ch)
{
    *(char*) ContextPush(sizeof(ch)) = ch;
}

inline void BubbleJson::ContextPushString(const char *string, size_t length)
{
    memcpy(ContextPush(length), string, length);
}

void* BubbleJson::ContextPush(size_t size)
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

void* BubbleJson::ContextPop(size_t size)
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
        ContextPushChar(number & 0xFF);
    else if (number <= 0x7FF)
    {
        ContextPushChar(0xC0 | ((number >> 6) & 0xFF));
        ContextPushChar(0x80 | ((number) & 0x3F));
    }
    else if (number <= 0xFFFF)
    {
        ContextPushChar(0xE0 | ((number >> 12) & 0xFF));
        ContextPushChar(0x80 | ((number >> 6) & 0x3F));
        ContextPushChar(0x80 | ((number) & 0x3F));
    }
    else if (number <= 0x10FFFF)
    {
        ContextPushChar(0xF0 | ((number >> 18) & 0xFF));
        ContextPushChar(0x80 | ((number >> 12) & 0x3F));
        ContextPushChar(0x80 | ((number >> 6) & 0x3F));
        ContextPushChar(0x80 | ((number) & 0x3F));
    }
}

ParseResults BubbleJson::ParseArray(BubbleValue *bubbleValue)
{
    BubbleContext* c = this->context;
    ParseResults result;

    Expect('[');
    ParseWhitespace();

    vector<BubbleValue>* elements = new vector<BubbleValue>;
    bubbleValue->u.array.elements = elements;

    if (*c->json == ']')
    {
        bubbleValue->type = ValueType_Array;
        c->json++;
        return ParseResult_Ok;
    }
    while (true)
    {
        BubbleValue valueTmp;
        if ((result = ParseValue(&valueTmp)) != ParseResult_Ok)
            break;

        elements->push_back(valueTmp);

        ParseWhitespace();
        if (*c->json == ',')
        {
            c->json++;
            ParseWhitespace();

        }
        else if (*c->json == ']')
        {
            c->json++;
            bubbleValue->type = ValueType_Array;
            return ParseResult_Ok;
        }
        else
        {
            result = ParseResult_MissCommaOrSquareBracket;
            break;
        }
            
    }
    BubbleValue* tmp;
    size_t count = elements->size();
    for (int i = 0; i < count; ++i)
    {
        tmp = &elements->at(i);
        tmp->MemoryFreeValue();
    }
    delete elements;
    bubbleValue->u.array.elements = nullptr;
    return result;
}

ParseResults BubbleJson::ParseObject(BubbleValue *bubbleValue)
{
    BubbleContext* c = this->context;
    ParseResults result;

    Expect('{');
    ParseWhitespace();

    map<string, BubbleValue>* members = new map<string, BubbleValue>;
    bubbleValue->u.object.members = members;

    if (*c->json == '}')
    {
        bubbleValue->type = ValueType_Object;
        c->json++;
        return ParseResult_Ok;
    }
    while (true)
    {
        char* c_str;
        size_t length;

        if (*c->json != '"')
        {
            result = ParseResult_MissKey;
            break;
        }
        if ((result = ParseStringRaw(&c_str, &length)) != ParseResult_Ok)
            break;
        string string(c_str,length);

        ParseWhitespace();
        if (*c->json == ':')
        {
            c->json++;
            ParseWhitespace();
        }
        else
        {
            result = ParseResult_MissColon;
            break;
        }

        BubbleValue valueTmp;
        if ((result = ParseValue(&valueTmp)) != ParseResult_Ok)
            break;
        members->emplace(string, valueTmp);

        ParseWhitespace();
        if (*c->json == ',')
        {
            c->json++;
            ParseWhitespace();
        }
        else if (*c->json == '}')
        {
            c->json++;
            bubbleValue->type = ValueType_Object;
            return ParseResult_Ok;
        }
        else
        {
            result = ParseResult_MissCommaOrCurlyBracket;
            break;
        }
    }

    for (auto it = members->begin(); it != members->end(); ++it)
    {
        //delete (it->first);
        it->second.MemoryFreeValue();
    }
    delete members;
    bubbleValue->u.object.members = nullptr;
    return result;
}

std::tuple<char*, size_t > BubbleJson::Stringify(BubbleValue *bubbleValue, StringifyTypes stringifyType)
{
    MemoryFreeContextStack();
    StringifyValue(bubbleValue, stringifyType, 0);
    char* json = this->context->stack;
    size_t length = this->context->top;

    //clear memory mark,but not memory space
    //ownership is transferred to return value
    this->context->top = 0;
    this->context->size = 0;
    this->context->stack = nullptr;
    return make_tuple(json, length);
}

void BubbleJson::StringifyValue(BubbleValue *value, StringifyTypes stringifyType, int tabCount)
{
    switch (value->GetType())
    {
        case ValueType_Null:    ContextPushString("null", 4); break;
        case ValueType_True:    ContextPushString("true", 4); break;
        case ValueType_False:   ContextPushString("false", 5); break;
        case ValueType_Number:
            //assume it took 32byte, then reclaim unused memory
            this->context->top -= 32 - sprintf((char*)ContextPush(32), "%.17g", value->GetNumber());
            break;
        case ValueType_String:
            ContextPushChar('\"');
            ContextPushString(value->GetString(), value->GetStringLength());
            ContextPushChar('\"');
            break;
        case ValueType_Object:
            StringifyObject(value, stringifyType, tabCount);
            break;
        case ValueType_Array:
            StringifyArray(value, stringifyType, tabCount);
            break;
        default:
            assert(false);
    }
}

void BubbleJson::StringifyObject(BubbleValue *value, StringifyTypes stringifyType, int tabCount)
{
    ContextPushChar('{');

    tabCount++;
    auto members = value->u.object.members;
    auto it = members->begin();
    while (true)
    {
        if (stringifyType == StringifyType_Beauty)
        {
            ContextPushChar('\n');
            for (int i = 0; i < tabCount; ++i) { ContextPushChar('\t'); }
        }
        ContextPushChar('\"');
        ContextPushString(it->first.c_str(), it->first.length());
        if (stringifyType == StringifyType_Beauty)
            ContextPushString("\": ", 3);
        else
            ContextPushString("\":", 2);
        StringifyValue(&it->second, stringifyType, tabCount);
        it++;

        //reach the last one
        if(it == members->end())
            break;
        else
            ContextPushChar(',');
    }

    tabCount--;
    if (stringifyType == StringifyType_Beauty)
    {
        ContextPushChar('\n');
        for (int i = 0; i < tabCount; ++i) { ContextPushChar('\t'); }
    }
    ContextPushChar('}');
}

void BubbleJson::StringifyArray(BubbleValue *value, StringifyTypes stringifyType, int tabCount)
{
    ContextPushChar('[');

    tabCount++;
    auto elements = value->u.array.elements;
    auto it = elements->begin();
    while (true)
    {
        if(stringifyType == StringifyType_Beauty)
        {
            ContextPushChar('\n');
            for (int i = 0; i < tabCount; ++i) { ContextPushChar('\t'); }
        }
        StringifyValue(&*it, stringifyType, tabCount);//weird
        it++;

        //reach the last one
        if(it == elements->end())
            break;
        else
            ContextPushChar(',');
    }

    tabCount--;
    if (stringifyType == StringifyType_Beauty)
    {
        ContextPushChar('\n');
        for (int i = 0; i < tabCount; ++i) { ContextPushChar('\t'); }
    }
    ContextPushChar(']');
}

