//
// Created by BangshengXie on 23/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEJSON_H
#define BUBBLEJSON_BUBBLEJSON_H

#include <utility>
#include <tuple>
#include <cassert>
#include "Struct.h"
#include "BubbleValue.h"

namespace bubbleJson {

class BubbleValue;

class BubbleJson
{
public:
    BubbleJson();
    std::tuple<ParseResults, BubbleValue *> Parse(const char *json);
    ~BubbleJson();
private:
    BubbleContext* context;
    const size_t stackInitSize = 256;

    void Expect(const char expectChar);
    void ParseWhitespace();
    ParseResults ParseValue(BubbleValue *bubbleValue);
    ParseResults ParseNumber(BubbleValue *bubbleValue);
    ParseResults ParseStringRaw(char **refString, size_t *refLength);//a pointer point a char* pointer
    ParseResults ParseString(BubbleValue *bubbleValue);
    ParseResults ParseArray(BubbleValue *bubbleValue);
    ParseResults ParseObject(BubbleValue *bubbleValue);
    ParseResults ParseLiteral(BubbleValue *bubbleValue, const char *expectJson, ValueTypes expectResult);
    const char * ParseHexToInt(const char *ch, unsigned *number);
    void EncodeUTF8(unsigned number);

    void MemoryFreeContextStack();

    void BubbleContextPushChar(char ch);
    void* BubbleContextPush(size_t size);
    void* BubbleContextPop(size_t size);
};

}


#endif //BUBBLEJSON_BUBBLEJSON_H
