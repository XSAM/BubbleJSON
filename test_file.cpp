//
// Created by BangshengXie on 04/06/2017.
//

#include "test_file.h"
#include "BubbleJSON/BubbleJson.h"
#include <iostream>
#include <fstream>
#include <tuple>
#include <chrono>

using namespace std;
using namespace bubbleJson;

void TestParseFromFile()
{
    tuple<ParseResults, BubbleValue*> result;
    BubbleValue *value;
    BubbleJson bubbleJson;

    //read file
    ifstream file;
    file.open("test.json");
    if (!file)
    {
        cout<<"file is not found!"<<endl;
        return;
    }
    string json;
    string tmp;
    while (getline(file,tmp))
    {
        json += tmp;
    };
    file.close();

    //parse
    auto start = chrono::high_resolution_clock::now();

    result = bubbleJson.Parse(json);
    value = get<1>(result);

    auto end = chrono::high_resolution_clock::now();
    auto parseElapsed = chrono::duration_cast<chrono::milliseconds>(end - start);

    //stringify
    start = chrono::high_resolution_clock::now();

    auto stringifyResult = bubbleJson.Stringify(value, StringifyType_Beauty);

    end = chrono::high_resolution_clock::now();
    auto stringifyElapsed = chrono::duration_cast<chrono::milliseconds>(end - start);

    //check result
    if(get<0>(result) != ParseResult_Ok)
        fprintf(stderr, "%s:%d: expect: 0 actual: %d", __FILE__, __LINE__, get<0>(result));

    cout<<"parse from file elapsed time: "<<parseElapsed.count()<<" ms"<<endl;
    cout<<"stringify from file elapsed time: "<<stringifyElapsed.count()<<" ms"<<endl;

    delete value;
    free (get<0>(stringifyResult));
}
