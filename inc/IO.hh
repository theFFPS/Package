#pragma once
#ifndef __IO__HH__
#define __IO__HH__
#include <iostream>
#include <sstream>
using namespace std;
namespace TNB {
    string ReadFile(string fileName);
    void WriteToFile(string fileName, string serializedData);
    string ToBytes(string text);
    string FromBytes(string text);
    string IntAsString(unsigned long long intt);
}
#endif