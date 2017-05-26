//
// Created by BangshengXie on 23/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEJSON_H
#define BUBBLEJSON_BUBBLEJSON_H

#include "Struct.h"
#include <utility>
#include <tuple>
#include <cassert>

using namespace std;

namespace bubbleJson {

class BubbleJson
{
public:
    BubbleJson();
    tuple<ParseResults, BubbleValue *> Parse(const char *json);
    ~BubbleJson();
private:
    BubbleContext* context;
    BubbleValue* value;
    const size_t stackInitSize = 256;
    void InitBubbleValue();

    void Expect(const char expectChar);
    void ParseWhitespace();
    ParseResults ParseNumber();
    ParseResults ParseString();
    ParseResults ParseValue();
    ParseResults ParseLiteral(const char *expectJson, ValueTypes expectResult);

    void MemoryFreeValueString();
    void MemoryFreeContextStack();
    void SetNull();
    bool GetBoolean();
    void SetBoolean(int boolean);
    double GetNumber();
    void SetNumber(double number);
    const char * GetString();
    void SetString(const char* string, size_t length);
    size_t GetStringLength();

    void BubbleContextPushChar(char ch);
    void* BubbleContextPush(size_t size);
    void* BubbleContextPop(size_t size);
};

}


#endif //BUBBLEJSON_BUBBLEJSON_H
