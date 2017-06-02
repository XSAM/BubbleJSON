//
// Created by BangshengXie on 29/05/2017.
//

#include "BubbleValue.h"
#include <cassert>

using namespace bubbleJson;
using namespace std;

BubbleValue::BubbleValue()
{
    this->type = ValueType_Null;
    this->isRoot = false;
}

BubbleValue::~BubbleValue()
{
    //only delete can invoke MemoryFreeValue while destruct
    //prevent memory being freed was not allocated

    //if (this->isRoot)
    //    MemoryFreeValue();
}

void BubbleValue::MemoryFreeValue()
{
    auto members = this->u.object.members;
    switch (this->type)
    {
        case ValueType_String:
            free(this->u.string.literal);
            this->u.string.literal = nullptr;
            break;
        case ValueType_Array:
            for (int i = 0; i < this->u.array.elements->size(); ++i)
            {
                this->u.array.elements->at(i).MemoryFreeValue();
            }
            delete this->u.array.elements;
            this->u.array.elements = nullptr;
            break;
        case ValueType_Object:
            for (auto it = members->begin(); it != members->end(); ++it)
            {
                it->second.MemoryFreeValue();
            }
            delete this->u.object.members;
            this->u.object.members = nullptr;
            break;
        default:
            break;
    }

    this->type = ValueType_Null;
}

ValueTypes BubbleValue::GetType()
{
    return this->type;
}

void BubbleValue::SetNull()
{
    MemoryFreeValue();
}

void BubbleValue::SetBoolean(bool boolean)
{
    MemoryFreeValue();
    this->type = boolean ? ValueType_True : ValueType_False;
}

bool BubbleValue::GetBoolean()
{
    assert(this->type == ValueType_True || this->type == ValueType_False);
    return this->type == ValueType_True;
}

void BubbleValue::SetNumber(double number)
{
    MemoryFreeValue();
    this->u.number = number;
    this->type = ValueType_Number;
}

double BubbleValue::GetNumber()
{
    assert(this->type == ValueType_Number);
    return this->u.number;
}

void BubbleValue::SetString(const char *string, size_t length)
{
    //allow empty string
    assert(string != nullptr || length == 0);
    MemoryFreeValue();

    this->u.string.literal = (char*)malloc(length + 1);
    memcpy(this->u.string.literal, string, length);
    this->u.string.literal[length] = '\0';
    this->u.string.length = length;

    this->type = ValueType_String;
}

const char *BubbleValue::GetString()
{
    assert(this->type == ValueType_String
           && this->u.string.literal != nullptr);
    return this->u.string.literal;
}

size_t BubbleValue::GetStringLength()
{
    assert(this->type == ValueType_String
           && this->u.string.literal != nullptr);
    return this->u.string.length;
}

BubbleValue *BubbleValue::GetArrayElement(size_t index)
{
    assert(this->type == ValueType_Array);
    assert(this->u.array.elements->size() > index);
    return &(this->u.array.elements->at(index));
}

size_t BubbleValue::GetArrayCount()
{
    assert(this->type == ValueType_Array);
    return this->u.array.elements->size();
}

void BubbleValue::SetArray(size_t count)
{
    assert(count > 0);
    MemoryFreeValue();

    vector<BubbleValue>* elements = new vector<BubbleValue>(count);
    this->u.array.elements = elements;
    this->type = ValueType_Array;
}

void BubbleValue::InsertArrayElementWithIndex(size_t index)
{
    assert(this->type == ValueType_Array);
    assert(index <= this->u.array.elements->size());//allow same index, insert value to the end

    BubbleValue valueTmp;
    auto iterator = this->u.array.elements->begin() + index;
    this->u.array.elements->insert(iterator, valueTmp);
}

void BubbleValue::DeleteArrayElementWithIndex(size_t index)
{
    assert(this->type == ValueType_Array);
    assert(index < this->u.array.elements->size());

    auto iterator = this->u.array.elements->begin() + index;
    this->u.array.elements->erase(iterator);
}

size_t BubbleValue::GetObjectCount()
{
    assert(this->type == ValueType_Object);
    return this->u.object.members->size();
}

BubbleValue * BubbleValue::GetObjectValueWithKey(const char *key)
{
    assert(this->type == ValueType_Object);
    auto it = this->u.object.members->find(key);
    return it != this->u.object.members->end() ? &it->second : nullptr;
}

BubbleValue * BubbleValue::GetObjectValueWithKey(const string key)
{
    assert(this->type == ValueType_Object);
    auto it = this->u.object.members->find(key);
    return it != this->u.object.members->end() ? &it->second : nullptr;
}

map<string, BubbleValue>* BubbleValue::GetObjects()
{
    assert(this->type == ValueType_Object);
    return this->u.object.members;
}

BubbleValue &BubbleValue::operator[](const size_t index)
{
    assert(this->type == ValueType_Array
           && index < this->u.array.elements->size());
    return this->u.array.elements->at(index);
}

BubbleValue &BubbleValue::operator[](size_t index) const
{
    return (const_cast<BubbleValue*>(this))->operator[](index);
}

BubbleValue &BubbleValue::operator[](const int index)
{
    assert(this->type == ValueType_Array
           && index < this->u.array.elements->size());
    return this->u.array.elements->at(index);
}

BubbleValue &BubbleValue::operator[](int index) const
{
    return (const_cast<BubbleValue*>(this))->operator[](index);
}

BubbleValue &BubbleValue::operator[](const char *key)
{
    return *this->GetObjectValueWithKey(key);
}

BubbleValue &BubbleValue::operator[](const char *key) const
{
    return (const_cast<BubbleValue*>(this))->operator[](key);
}

BubbleValue &BubbleValue::operator[](std::string key)
{
    return *this->GetObjectValueWithKey(key);
}

BubbleValue &BubbleValue::operator[](std::string key) const
{
    return (const_cast<BubbleValue*>(this))->operator[](key);
}

void BubbleValue::operator delete(void *bubbleValue)
{
    BubbleValue* value = (BubbleValue*)bubbleValue;

    //only root can free memory
    if (value->isRoot)
        value->MemoryFreeValue();
    //delete value (not void* bubbleValue) will invoke delete operator infinity
    ::operator delete(bubbleValue);
}