//
// Created by BangshengXie on 30/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEMEMBER_H
#define BUBBLEJSON_BUBBLEMEMBER_H

#include "BubbleValue.h"

namespace bubbleJson {

class BubbleValue;

class BubbleMember
{
private:
    char* key;
    size_t keyLength;
    BubbleValue* value;

    void MemoryFreeKey();
    void MemoryFreeAll();
public:
    friend class BubbleJson;
    friend class BubbleValue;
    BubbleMember();
    ~BubbleMember();

    void SetKey(const char* string, size_t length);
    const char* GetKey();
    size_t GetKeyLength();
    BubbleValue* GetValue();
};

}

#endif //BUBBLEJSON_BUBBLEMEMBER_H
