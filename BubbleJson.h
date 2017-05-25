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
    BubbleContent* content;
    BubbleValue* value;

    void Expect(const char expectChar);
    void ParseWhitespace();
    ParseResults ParseNumber();

    ParseResults ParseValue();
    ParseResults ParseLiteral(const char *expectJson, ValueTypes expectResult);
};

}


#endif //BUBBLEJSON_BUBBLEJSON_H
