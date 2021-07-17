#include "relocationrecord.h"
#include "iomanip"

string RelocationRecord::intToHex(int i)
{
    stringstream sstream;
    sstream << hex << i;
    string res = sstream.str();

    return res;
}

RelocationRecord::RelocationRecord(string offset, string type, int vr, int section)
{
    this->offset = offset;
    this->type = type;
    this->vr = vr;
    this->section = section;
}

void RelocationRecord::print(){
    cout<<left<<setw(10)<<this->offset<<left<<setw(13)<<this->type<<left<<setw(15)<<this->vr<<endl;
}

void RelocationRecord::print(ofstream& output){
    output<<left<<setw(10)<<this->offset<<left<<setw(13)<<this->type<<left<<setw(15)<<this->vr<<endl;
}

void RelocationRecord::printHex(ofstream& output){
    output<<left<<setw(10)<<intToHex(stoi(this->offset))<<left<<setw(13)<<this->type<<left<<setw(15)<<intToHex(this->vr)<<endl;
}
