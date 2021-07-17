#include <iostream>
#include "symboltable.h"
#include "iomanip"
using namespace std;

string SymbolTable::intToHex(int i)
{
    stringstream sstream;
    sstream << hex << i;
    string res = sstream.str();

    return res;
}

SymbolTable::SymbolTable()
{
    //Symbol *s = new Symbol("UND", "0", "0", 'l', 0, 0);
    //Symbol *s1 = new Symbol("APS", "1", "0", 'g', 0, 0);
    Symbol * s = new Symbol("UND", 'l', "0");
    Symbol *s1 = new Symbol("APS", 'l', "1");
    addElem(*s);
    addElem(*s1);
}

void SymbolTable::addElem(Symbol &symbol)
{
    Elem *pom = new Elem(symbol);
    if (this->head == nullptr)
    {
        this->head = pom;
        this->tail = pom;
    }
    else
    {
        if(this->head->next == nullptr){
            this->head->next = pom;
        }
        tail->next = pom;
        tail = pom;
    }
   
    size++;
}

bool SymbolTable::elemExsist(string name){
    Elem *tek = head;
    while(tek != nullptr){
        if(tek->symbol.getName() == name){
            return true;
        }
        tek = tek->next;
    }
    return false;
}

void SymbolTable::updateElem(Symbol &symbol){
    Elem *tek = head;
    while(tek != nullptr){
        if(tek->symbol.getName() == symbol.getName()){
            tek->symbol.setSection(symbol.getSection());
            tek->symbol.setValue(symbol.getValue());
            tek->symbol.setSize(symbol.getSize());
            tek->symbol.setZapis(symbol.getZapis());
            tek->symbol.setVisibility(symbol.getVisibility());
            tek->symbol.setRelokacioniZapis(symbol.getRelokacioniZapis());
        }
        tek = tek->next;
    }
}

Symbol SymbolTable::returnElem(string name){
    Elem *tek = head;
    while(tek != nullptr){
        if(tek->symbol.getName() == name){
            return tek->symbol;
        }
        tek = tek->next;
    }
    Symbol *s = new Symbol();
    return *s;
}

Symbol SymbolTable::returnElem(int value){
    Elem *tek = head;
    while(tek != nullptr){
        if(tek->symbol.getNumber() == value){
            return tek->symbol;
        }
        tek = tek->next;
    }
    Symbol *s = new Symbol();
    return *s;
}

void SymbolTable::print(){
    cout<<left<<setw(10)<<"Ime"<<left<<setw(10)<<"Sekcija"<<left<<setw(10)<<"Vrednost"<<left<<setw(13)<<"Vidljivost"<<left<<setw(13)<<"Redni broj"<<left<<setw(13)<<"Velicina sekcije"<<endl;
    Elem *tmp = this->head;
    while(tmp != nullptr){
        cout<<left<<setw(10)<<tmp->symbol.getName()<<left<<setw(10)<<tmp->symbol.getSection()<<left<<setw(10)<<tmp->symbol.getValue()
        <<left<<setw(13)<<tmp->symbol.getVisibility()<<left<<setw(13)<<tmp->symbol.getNumber()<<left<<setw(13)<<tmp->symbol.getSize()<<endl;
        tmp = tmp->next;
    }
}

void SymbolTable::print(ofstream& output){
    output<<left<<setw(10)<<"Ime"<<left<<setw(10)<<"Sekcija"<<left<<setw(10)<<"Vrednost"<<left<<setw(13)<<"Vidljivost"<<left<<setw(13)<<"Redni broj"<<left<<setw(13)<<"Velicina sekcije"<<endl;
    Elem *tmp = this->head;
    while(tmp != nullptr){
        output<<left<<setw(10)<<tmp->symbol.getName()<<left<<setw(10)<<tmp->symbol.getSection()<<left<<setw(10)<<tmp->symbol.getValue()
        <<left<<setw(13)<<tmp->symbol.getVisibility()<<left<<setw(13)<<tmp->symbol.getNumber()<<left<<setw(13)<<tmp->symbol.getSize()<<endl;
        tmp = tmp->next;
    }
}

void SymbolTable::printZapis(){
    Elem *tmp = this->head;
    while(tmp != nullptr){
        if((tmp->symbol.getSection() == to_string(tmp->symbol.getNumber()) && (tmp->symbol.getName() != "UND" && tmp->symbol.getName() != "APS"))){
            cout<<tmp->symbol.getName()<<endl;
            vector<string> pom = tmp->symbol.getZapis();
            for(int i=0; i < pom.size(); i++){
                cout<<pom.at(i);
                if(i % 2 == 1) cout << " ";
            }
            cout<<endl;
        }
       tmp = tmp->next;
    }
}

void SymbolTable::printRelokacioniZapis(){
   
    Elem *tmp = this->head;
    while(tmp != nullptr){
        if((tmp->symbol.getSection() == to_string(tmp->symbol.getNumber()) && (tmp->symbol.getName() != "UND" && tmp->symbol.getName() != "APS"))){
            cout<<tmp->symbol.getName()<<endl;
            cout<<left<<setw(10)<<"Offset"<<left<<setw(13)<<"Tip"<<setw(15)<<"Vrednost"<<endl;
            for(int i = 0; i < tmp->symbol.getRelokacioniZapis().size(); i++){
                tmp->symbol.getRelokacioniZapis().at(i).print();
            }
            cout<<endl;
        }
       tmp = tmp->next;
    }
}

void SymbolTable::printZapis(ofstream& output){
    
    Elem *tmp = this->head;
    while(tmp != nullptr){
        if((tmp->symbol.getSection() == to_string(tmp->symbol.getNumber()) && (tmp->symbol.getName() != "UND" && tmp->symbol.getName() != "APS"))){
            output<<tmp->symbol.getName()<<endl;
            vector<string> pom = tmp->symbol.getZapis();
            for(int i=0; i < pom.size(); i++){
                output<<pom.at(i);
                if(i % 2 == 1) output << " ";
            }
            output<<endl;
        }
       tmp = tmp->next;
    }
}

void SymbolTable::printRelokacioniZapis(ofstream& output){
    
    Elem *tmp = this->head;
    while(tmp != nullptr){
        if((tmp->symbol.getSection() == to_string(tmp->symbol.getNumber()) && (tmp->symbol.getName() != "UND" && tmp->symbol.getName() != "APS"))){
            output<<tmp->symbol.getName()<<endl;
            output<<left<<setw(10)<<"Offset"<<left<<setw(13)<<"Tip"<<setw(15)<<"Vrednost"<<endl;
            for(int i = 0; i < tmp->symbol.getRelokacioniZapis().size(); i++){
                tmp->symbol.getRelokacioniZapis().at(i).print(output);
            }
            output<<endl;
        }
       tmp = tmp->next;
    }
}

void SymbolTable::reorganize()
{
    SymbolTable* nova = new SymbolTable();
    nova->head->symbol.setNumber(0);
    nova->head->next->symbol.setNumber(1);
    Elem *tek = this->head->next->next;
    Elem *head1 = nullptr, *tail1 = nullptr;
    while (tek != nullptr)
    {
        if (tek->symbol.getSection() == to_string(tek->symbol.getNumber()))
        {
            nova->addElem(tek->symbol);
        }
        tek = tek->next;
    }
    tek = this->head->next->next;
    while (tek != nullptr)
    {
        if (tek->symbol.getSection() != to_string(tek->symbol.getNumber()))
        {
            nova->addElem(tek->symbol);
        }
        tek = tek->next;
    }
    tek = nova->head;
    int i = 0;
    while (tek != nullptr)
    {
        if (tek->symbol.getSection() == to_string(tek->symbol.getNumber()) && tek->symbol.getType() == 0) //tip 0 je za sekcije
        {
            Elem *tmp = tek->next;
            while (tmp != nullptr)
            {
                if (tmp->symbol.getSection() == to_string(tek->symbol.getNumber()))
                {
                    tmp->symbol.setSection(to_string(i));
                    //cout<<"Simbol: "<<tmp->symbol.getName()<<" sekcija"<<tmp->symbol.getSection()<<endl;
                }
                tmp = tmp->next;
            }
            tek->symbol.setSection(to_string(i));
        }
        tek->symbol.setNumber(i);
        i++;
        tek = tek->next;
    }
    head = nova->head;
    tail = nova->tail;
    delete nova;
}

void SymbolTable::printHex(ofstream& output){

}
void SymbolTable::printRelokacioniZapisHex(ofstream& output){
    Elem *tmp = this->head;
    while(tmp != nullptr){
        if((tmp->symbol.getSection() == to_string(tmp->symbol.getNumber()) && (tmp->symbol.getName() != "UND" && tmp->symbol.getName() != "APS"))){
            output<<tmp->symbol.getName()<<endl;
            output<<left<<setw(10)<<"Offset"<<left<<setw(13)<<"Tip"<<setw(15)<<"Vrednost"<<endl;
            for(int i = 0; i < tmp->symbol.getRelokacioniZapis().size(); i++){
                tmp->symbol.getRelokacioniZapis().at(i).printHex(output);
            }
            output<<endl;
        }
       tmp = tmp->next;
    }
}