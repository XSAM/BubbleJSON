//
// Created by BangshengXie on 29/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEVALUE_H
#define BUBBLEJSON_BUBBLEVALUE_H

#include <cstdlib>
#include <vector>
#include "Struct.h"
#include "BubbleMember.h"

namespace bubbleJson {

class BubbleMember;

class BubbleValue
{
private:
    bool isRoot;//only root can invoke MemoryFreeValue while destruct
    union
    {
        double number;
        struct { char *literal; size_t length; } string;
        struct { std::vector<BubbleValue>* elements; } array;
        struct { BubbleMember* member; size_t count; } object;
    } u;
    ValueTypes type;
public:
    friend class BubbleJson;
    BubbleValue();
    ~BubbleValue();
    void MemoryFreeValue();

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

    size_t GetObjectCount();
    const char* GetObjectKey(size_t index);
    size_t GetObjectKeyLength(size_t index);
    BubbleValue* GetObjectValue(size_t index);
    BubbleMember* GetObjects();

    BubbleValue& operator[](const size_t index);
    BubbleValue& operator[](const size_t index) const;
    void operator delete(void *bubbleValue);
};

}


#endif //BUBBLEJSON_BUBBLEVALUE_H
