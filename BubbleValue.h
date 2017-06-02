//
// Created by BangshengXie on 29/05/2017.
//

#ifndef BUBBLEJSON_BUBBLEVALUE_H
#define BUBBLEJSON_BUBBLEVALUE_H

#include <cstdlib>
#include <vector>
#include <map>
#include <string>
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
        struct { std::map<std::string, BubbleValue>* members; } object;
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
    void SetArray(size_t count);
    void InsertArrayElementWithIndex(size_t index);
    void DeleteArrayElementWithIndex(size_t index);

    size_t GetObjectCount();
    BubbleValue * GetObjectValueWithKey(const char *key);
    BubbleValue * GetObjectValueWithKey(const std::string key);
    std::map<std::string, BubbleValue> * GetObjects();

    BubbleValue& operator[](const size_t index);
    BubbleValue& operator[](const size_t index) const;
    BubbleValue& operator[](const int index);
    BubbleValue& operator[](const int index) const;
    BubbleValue& operator[](const char* key);
    BubbleValue& operator[](const char* key) const;
    BubbleValue& operator[](std::string key);
    BubbleValue& operator[](std::string key) const;

    void operator delete(void *bubbleValue);
};

}


#endif //BUBBLEJSON_BUBBLEVALUE_H
