#include "relocationtable.h"
#include "iomanip"

RelocationTable::RelocationTable(){
    this->head = nullptr;
    this->tail = nullptr;
    this->size = 0;
}
void RelocationTable::addElem(RelocationRecord &r){
    Elem *pom = new Elem(r);
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

void RelocationTable::print(){
    
    cout<<left<<setw(10)<<"Ofset"<<left<<setw(13)<<"Tip"<<left<<setw(15)<<"Vrednost"<<left<<setw(13)<<"Sekcija"<<endl;
    Elem *tmp = this->head;
    while(tmp != nullptr){
        cout<<left<<setw(10)<<tmp->record.offset<<left<<setw(13)<<tmp->record.type<<left<<setw(15)<<tmp->record.vr
        <<left<<setw(13)<<tmp->record.section<<endl;
        tmp = tmp->next;
    }
}

void RelocationTable::print(ofstream& output){
    
    output<<left<<setw(10)<<"Ofset"<<left<<setw(13)<<"Tip"<<left<<setw(15)<<"Vrednost"<<left<<setw(13)<<"Sekcija"<<endl;
    Elem *tmp = this->head;
    while(tmp != nullptr){
        output<<left<<setw(10)<<tmp->record.offset<<left<<setw(13)<<tmp->record.type<<left<<setw(15)<<tmp->record.vr
        <<left<<setw(13)<<tmp->record.section<<endl;
        tmp = tmp->next;
    }
}
