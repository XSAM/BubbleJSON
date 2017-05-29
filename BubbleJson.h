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

using namespace std;

namespace bubbleJson {

    class BubbleValue;

class BubbleJson
{
public:
    BubbleJson();
    tuple<ParseResults, BubbleValue *> Parse(const char *json);
    void MemoryFreeBubbleValue(BubbleValue *bubbleValue);
    ~BubbleJson();
private:
    BubbleContext* context;
    const size_t stackInitSize = 256;
    void InitBubbleValue(BubbleValue *bubbleValue);

    void Expect(const char expectChar);
    void ParseWhitespace();
    ParseResults ParseValue(BubbleValue *bubbleValue);
    ParseResults ParseNumber(BubbleValue *bubbleValue);
    ParseResults ParseString(BubbleValue *bubbleValue);
    ParseResults ParseArray(BubbleValue *bubbleValue);
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
