#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <iostream>
#include <fstream>
#include "symbol.h"
using namespace std;


class SymbolTable{
public:
    struct Elem{
        Symbol symbol;
        Elem* next;
        Elem(Symbol &s) : symbol(s), next(nullptr) {}
    };
    Elem *head = nullptr, *tail = nullptr;
    int size = 0;


    SymbolTable();
    void addElem(Symbol &symbol);
    void print();
    void print(ofstream& output);
    void printZapis();
    void printZapis(ofstream& output);
    bool elemExsist(string name);
    void updateElem(Symbol &symbol);
    Symbol returnElem(string name);
    Symbol returnElem(int value);
    void reorganize();
    void printRelokacioniZapis();
    void printRelokacioniZapis(ofstream& output);
    void printHex(ofstream& output);
    void printRelokacioniZapisHex(ofstream& output);
    string intToHex(int i);

};

#endif