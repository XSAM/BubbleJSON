//
// Created by BangshengXie on 29/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEVALUE_H
#define BUBBLEJSON_BUBBLEVALUE_H

#include <cstdlib>
#include "Struct.h"

namespace bubbleJson {

class BubbleValue
{
private:
    union
    {
        double number;
        struct
        {
            char *literal;
            size_t length;
        } string;
        struct
        {
            BubbleValue *elements;
            size_t count;
        } array;
    } u;
    ValueTypes type;
public:
    friend class BubbleJson;
    BubbleValue();
    ~BubbleValue();
    void MemoryFreeBubbleValue();

    ValueTypes GetType();

    void SetNull();

    void SetBoolean(bool boolean);
    bool GetBoolean();

    void SetNumber(double number);
    double GetNumber();

    void SetString(const char* string, size_t length);
    const char* GetString();
    size_t GetStringLength();

    BubbleValue* GetArrayElement(size_t index);
    size_t GetArrayCount();
};

}


#endif //BUBBLEJSON_BUBBLEVALUE_H
