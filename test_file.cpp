//
// Created by BangshengXie on 04/06/2017.
//

#include "test_file.h"
#include "BubbleJson.h"
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

    result = bubbleJson.Parse(json.c_str());
    value = get<1>(result);

    auto end = chrono::high_resolution_clock::now();

    //check result
    if(get<0>(result) != ParseResult_Ok)
        fprintf(stderr, "%s:%d: expect: 0 actual: %d", __FILE__, __LINE__, get<0>(result));

    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout<<"TestParseFromFile() elapsed time: "<<elapsed.count()<<" ms"<<endl;

    delete value;
}
