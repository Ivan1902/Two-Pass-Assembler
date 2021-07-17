#ifndef _IMPLEMENTACIJA_H_
#define _IMPLEMENTACIJA_H_
#include <iostream>
#include <vector>
#include <fstream>
#include "symboltable.h"
#include "relocationtable.h"
using namespace std;

class Asembler{
    public:
    static int locationCounter;
    int line = 0;
    bool greska = false;
    static SymbolTable *symbolTable;
    static RelocationTable *relocationTable;

    vector<vector<string>> parsedInput;
    vector<vector<string>> parseInput(string filename);
    vector<string> isparsirajLiniju(string line);
    void firstPass(vector<vector<string>> &parsedInput, ofstream& output);
    void firstPass(vector<string> &line);
    void secondPass(vector<vector<string>> &parsedInput, ofstream& output);
    void secondPass(vector<string> &line);
};

#endif