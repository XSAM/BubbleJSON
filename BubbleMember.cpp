//
// Created by BangshengXie on 30/05/2017.
//

#include "BubbleMember.h"
#include <cassert>
#include <memory>
#include <iostream>
#include <exception>

using namespace bubbleJson;

BubbleMember::BubbleMember()
{
    this->key = nullptr;
    this->value = new BubbleValue();
}

BubbleMember::~BubbleMember()
{
    //root value will delete this recursively
    //delete this->value;
}

void BubbleMember::SetKey(const char *string, size_t length)
{
    //allow empty string
    assert(string != nullptr || length == 0);
    MemoryFreeKey();

    this->key = (char*)malloc(length + 1);
    memcpy(this->key, string, length);
    this->key[length] = '\0';
    this->keyLength = length;
}

void BubbleMember::MemoryFreeKey()
{
    if (this->key != nullptr)
    {
        free(this->key);
        this->key = nullptr;
        this->keyLength = 0;
    }
}

void BubbleMember::MemoryFreeAll()
{
    MemoryFreeKey();
    this->value->MemoryFreeValue();
    delete this->value;
}

const char* BubbleMember::GetKey()
{
    return this->key;
}

size_t BubbleMember::GetKeyLength()
{
    return this->keyLength;
}

BubbleValue* BubbleMember::GetValue()
{
    return this->value;
}
