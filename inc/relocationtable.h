#ifndef _RELOCATION_TABLE_H_
#define _RELOCATION_TABLE_H_
#include<iostream>
#include<fstream>
#include "relocationrecord.h"
using namespace std;

class RelocationTable{
    public:
    struct Elem{
        RelocationRecord record;
        Elem* next;
        Elem(RelocationRecord &r): record(r), next(nullptr){}
    };
    Elem *head = nullptr;
    Elem *tail = nullptr;
    int size = 0;

    RelocationTable();
    void addElem(RelocationRecord &r);
    void print();
    void print(ofstream& output);
};



#endif