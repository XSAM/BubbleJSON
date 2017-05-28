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
    void MemoryFreeValueString(BubbleValue *bubbleValue);
    ~BubbleJson();
private:
    BubbleContext* context;
    BubbleValue* value;
    const size_t stackInitSize = 256;
    void InitBubbleValue(BubbleValue *bubbleValue);

    void Expect(const char expectChar);
    void ParseWhitespace();
    ParseResults ParseValue(BubbleValue *bubbleValue);
    ParseResults ParseNumber(BubbleValue *bubbleValue);
    ParseResults ParseString(BubbleValue *bubbleValue);
    ParseResults ParseLiteral(BubbleValue *bubbleValue, const char *expectJson, ValueTypes expectResult);
    const char * ParseHexToInt(const char *ch, unsigned *number);
    void EncodeUTF8(unsigned number);

    void MemoryFreeContextStack();
    void SetNull(BubbleValue *bubbleValue);
    bool GetBoolean(BubbleValue *bubbleValue);
    void SetBoolean(BubbleValue *bubbleValue, int boolean);
    double GetNumber(BubbleValue *bubbleValue);
    void SetNumber(BubbleValue *bubbleValue, double number);
    const char *GetString(BubbleValue *bubbleValue);
    void SetString(BubbleValue *bubbleValue, const char *string, size_t length);
    size_t GetStringLength(BubbleValue *bubbleValue);

    void BubbleContextPushChar(char ch);
    void* BubbleContextPush(size_t size);
    void* BubbleContextPop(size_t size);
};

}


#endif //BUBBLEJSON_BUBBLEJSON_H
