#ifndef _RELOCATION_RECORD_H_
#define _RELOCATION_RECORD_H_
#include <iostream>
#include <fstream>
using namespace std;

class RelocationRecord{
    public:
    string offset;
    string type;
    int vr;
    int section;

    RelocationRecord(string offset, string type, int vr, int section);
    void print();
    void print(ofstream& output);
    void printHex(ofstream& output);
    string intToHex(int i);
};

#endif