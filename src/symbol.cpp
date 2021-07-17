#include <iostream>
#include "symbol.h"
using namespace std;

int Symbol::ID = 0;

Symbol::Symbol(){
    
}

Symbol::Symbol(string name1, string section, string value, char visibility, int size, int type)
{
    this->name = name1;
    this->section = section;
    this->value = value;
    this->visibility = visibility;
    this->number = this->ID++;
    this->size = size;
    this->type = 1;
    this->relokacioniZapis = {};
}

Symbol::Symbol(string name1, char visibility)
{
    this->name = name1;
    this->visibility = visibility;
    this->number = this->ID++;
    this->section = "0";
    this->size = 0;
    this->type = 1;
    this->value = "0";
}

Symbol::Symbol(string name1, char visibility, string section)
{
    this->name = name1;
    this->visibility = visibility;
    this->number = this->ID++;
    this->section = section;
    this->value = "0";
    this->type = 0;
    this->size = 0;
}

string Symbol::getName()
{
    return name;
}
string Symbol::getSection()
{
    return section;
}
string Symbol::getValue()
{
    return value;
}
char Symbol::getVisibility()
{
    return visibility;
}
int Symbol::getNumber()
{
    return number;
}
int Symbol::getSize()
{
    return size;
}
vector<string> Symbol::getZapis(){
    return zapis;
}

void Symbol::setName(string name)
{
    this->name = name;
}
void Symbol::setSection(string section)
{
    this->section = section;
}
void Symbol::setValue(string value)
{
    this->value = value;
}
void Symbol::setVisibility(char type)
{
    this->visibility = type;
}
void Symbol::setNumber(int number)
{
    this->number = number;
}
void Symbol::setSize(int size)
{
    this->size = size;
}
void Symbol::setZapis(vector<string> zapis){
    this->zapis = zapis;
}

int Symbol::getType(){
    return this->type;
}

void Symbol::setType(int tip){
    this->type = tip;
}

vector<RelocationRecord>& Symbol::getRelokacioniZapis(){
    return this->relokacioniZapis;
}

void Symbol::setRelokacioniZapis(vector<RelocationRecord> relokacioniZapis){
    this->relokacioniZapis = relokacioniZapis;
}
