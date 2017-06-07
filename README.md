# Bubble JSON

[![travis build status](https://api.travis-ci.org/XSAM/BubbleJSON.svg?branch=master_stl)](https://travis-ci.org/XSAM/BubbleJSON)

A lite JSON parser/generator for C++

# Features

- small but **complete**
- support **Unicode**
- provide friendly operator **[]** to access data
- using C++ standard library. It is **cross-platform**

# Installation

BubbleJSON is a header-only C++ library. copy the `BubbleJSON` folder into the project or system include path. Make sure check for C++11 support.

To run test follow these steps:
- Go to repository folder
- `cmake .`
- `make`
- run `bubbleJson_test`

# Example

This simple example show a way to **parse/modification/stringify** *json* with `BubbleJSON`

```c++
#include <iostream>
#include "BubbleJSON/BubbleJson.h"

using namespace bubbleJson;

int main()
{
    // parse char* or string type json into tuple<Parse>
    const char* json = "{\"project\":\"BubbleJSON\",\"Compatibility\":\"C++11\"}";
    BubbleJson bubbleJson;

    auto result = bubbleJson.Parse(json);
    // parse ok, no error
    if (std::get<0>(result) != ParseResult_Ok)
        printf("parse error");

    // get value
    BubbleValue* value = std::get<1>(result);
    std::cout << (*value)["project"].GetString();

    // modify value
    (*value)["Compatibility"].SetArray(2);
    (*value)["Compatibility"][0].SetString("cpp11");
    (*value)["Compatibility"][1].SetString("all platform");

    // output stringify result, default Stringify_Type is beauty
    // output {"Compatibility":["cpp11","all platform"],"project":"BubbleJSON"}
    auto stringify = bubbleJson.Stringify(value, StringifyType_Minimum);
    std::cout << std::get<0>(stringify);

    // don't forget delete value, and free stringify result
    delete value;
    free (std::get<0>(stringify));

    return 0;
}
```

# API

Parse or Stringify with **BubbleJson**
```c++
std::tuple<ParseResults, BubbleValue *> Parse(const char *json);
std::tuple<ParseResults, BubbleValue *> Parse(const std::string json);
// there are two stringify modes can choose, beauty and minimum
std::tuple<char*, size_t > Stringify(BubbleValue *bubbleValue, StringifyTypes stringifyType = StringifyType_Beauty);
```

Get data with **BubbleValue**

```c++
ValueTypes GetType();
void SetBoolean(bool boolean);
bool GetBoolean();
double GetNumber();
const char* GetString();
size_t GetStringLength();

BubbleValue* GetArrayElement(size_t index);
size_t GetArrayCount();

size_t GetObjectCount();
BubbleValue * GetObjectValueWithKey(const char *key);
BubbleValue * GetObjectValueWithKey(const std::string key);
std::map<std::string, BubbleValue> * GetObjects();
```

Set data with **BubbleValue**
```c++
void SetNull();
void SetNumber(double number);
void SetString(const char* string, size_t length);
void SetString(const std::string string);

void SetArray(size_t count);
void InsertArrayElementWithIndex(size_t index);
void DeleteArrayElementWithIndex(size_t index);

void SetObject();
void InsertObjectElementWithKey(const char *key, const size_t keyLength);
void InsertObjectElementWithKey(const std::string key);
void DeleteObjectElementWithKey(const char *key, const size_t keyLength);
void DeleteObjectElementWithKey(const std::string key);
// also you can invoke GetObjects with BubbleValue
// and set the return value whatever you want
```

operator [] with **BubbleValue**
```c++
BubbleValue& operator[](const size_t index);
BubbleValue& operator[](const size_t index) const;
BubbleValue& operator[](const int index);
BubbleValue& operator[](const int index) const;
BubbleValue& operator[](const char* key);
BubbleValue& operator[](const char* key) const;
BubbleValue& operator[](std::string key);
BubbleValue& operator[](std::string key) const;
```
