//
// Created by BangshengXie on 29/05/2017.
//

#include "BubbleValue.h"
#include <cassert>

using namespace bubbleJson;

BubbleValue::BubbleValue()
{
    this->type = ValueType_Null;
    this->isRoot = false;
}

BubbleValue::~BubbleValue()
{
    //only root can invoke MemoryFreeValue while destruct
    if (this->isRoot)
        MemoryFreeValue();
}

void BubbleValue::MemoryFreeValue()
{
    switch (this->type)
    {
        case ValueType_String:
            free(this->u.string.literal);
            this->u.string.literal = nullptr;
            break;
        case ValueType_Array:
            for (int i = 0; i < this->u.array.count; ++i)
            {
                this->u.array.elements[i].MemoryFreeValue();
            }
            free(this->u.array.elements);
            this->u.array.elements = nullptr;
            break;
        case ValueType_Object:
            for (int i = 0; i < this->u.object.count; ++i)
            {
                this->u.object.member[i].MemoryFreeAll();
            }
            free(this->u.object.member);
            this->u.object.member = nullptr;
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
    assert(this->type == ValueType_Array && this->u.array.count > index);
    return &this->u.array.elements[index];
}

size_t BubbleValue::GetArrayCount()
{
    assert(this->type == ValueType_Array);
    return this->u.array.count;
}

size_t BubbleValue::GetObjectCount()
{
    assert(this->type == ValueType_Object);
    return this->u.object.count;
}

const char *BubbleValue::GetObjectKey(size_t index)
{
    assert(this->type == ValueType_Object
           && index < this->u.object.count);
    return this->u.object.member[index].key;
}

size_t BubbleValue::GetObjectKeyLength(size_t index)
{
    assert(this->type == ValueType_Object
           && index < this->u.object.count);
    return this->u.object.member[index].keyLength;
}

BubbleValue *BubbleValue::GetObjectValue(size_t index)
{
    assert(this->type == ValueType_Object
           && index < this->u.object.count);
    return this->u.object.member[index].value;
}

BubbleMember* BubbleValue::GetObjects()
{
    assert(this->type == ValueType_Object);
    return this->u.object.member;
}