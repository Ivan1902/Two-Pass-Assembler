#ifndef _SYMBOL_H_
#define _SYMBOL_H_
#include <iostream>
#include <vector>
#include "relocationrecord.h"
using namespace std;



class Symbol{
    string name;
    string section;
    string value;
    char visibility; //local or global
    int number;
    int size; //velicina sekcije
    int type; // 0 za sekciju i 1 za simbol
    vector<string> zapis;
    vector<RelocationRecord> relokacioniZapis;
    

public:

    static int ID;

    Symbol(string name, string section, string value, char visibility, int size, int tip);
    Symbol(string name, char visibility);
    Symbol(string name, char visibility, string section); //za sekcije
    Symbol();


    string getName();
    string getSection();
    string getValue();
    char getVisibility();
    int getNumber();
    int getSize();
    vector<string> getZapis();
    int getType();
    vector<RelocationRecord>& getRelokacioniZapis();

    void setName(string name);
    void setSection(string section);
    void setValue(string value);
    void setVisibility(char visibility);
    void setNumber(int number);
    void setSize(int size);
    void setZapis(vector<string> zapis);
    void setType(int tip);
    void setRelokacioniZapis(vector<RelocationRecord> relokacioniZapis);


};

#endif