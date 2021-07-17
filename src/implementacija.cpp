#include "implementacija.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "ctype.h"
#include <cctype>
#include "algorithm"
#include "symbol.h"
#include <regex>
using namespace std;

int currentSection = 0;
Symbol curSection;
int Asembler::locationCounter = 0;
SymbolTable *Asembler::symbolTable = new SymbolTable();
RelocationTable *Asembler::relocationTable = new RelocationTable();
struct SectionValue
{
    int section;
    int value;
};
vector<SectionValue> pojavilaSeSekcija; // potrebno za drugi prolaz
vector<vector<string>> zapisi;
vector<string> stagod;

// P O M O C N E     F U N K C I J E
//==============================================

string intToHex(int i)
{
    stringstream sstream;
    sstream << hex << i;
    string res = sstream.str();

    return res;
}

int hexToInt(string broj)
{

    int result = stoi(broj, 0, 16);
    return result;
}

vector<string> insert2BytesWithoutLittleEndian(string number)
{
    if (number.size() > 4)
    {
        number = number.substr(number.size() - 4);
    }
    vector<string> pom{"0", "0", "0", "0"};
    int k = 0;
    for (int i = number.size() - 1; i >= 0; i--)
    {
        pom[pom.size() - 1 - k] = number[i];
        k++;
    }
    return pom;
}

vector<string> insert2BytesWithLittleEndian(string number)
{

    vector<string> pom{"0", "0", "0", "0"};
    int k = 0;
    for (int i = number.size() - 1; i >= 0; i--)
    {
        pom[pom.size() - 1 - k] = number[i];
        k++;
    }
    vector<string> tmp{"0", "0", "0", "0"};

    tmp[0] = pom[2];
    tmp[1] = pom[3];
    tmp[2] = pom[0];
    tmp[3] = pom[1];

    return tmp;
}

vector<string> insert4BytesWithLittleEndian(string number)
{
    cout << "DOSAO DA SE SREDI BROJ: " << number << endl;

    vector<string> pom{"0", "0", "0", "0", "0", "0", "0", "0"};
    int k = 0;
    for (int i = number.size() - 1; i >= 0; i--)
    {
        pom[pom.size() - 1 - k] = number[i];
        k++;
    }

    string tmp1;
    for (int i = 0; i < 4; i++)
        tmp1 += pom[i];
    vector<string> string1 = insert2BytesWithLittleEndian(tmp1);

    string tmp2;
    for (int i = 4; i < 8; i++)
        tmp1 += pom[i];
    vector<string> string2 = insert2BytesWithLittleEndian(tmp1);

    string2.insert(string2.end(), string1.begin(), string1.end());

    return string2;
}

bool bilaSekcija(int section)
{
    for (int i = 0; i < pojavilaSeSekcija.size(); i++)
    {
        if (pojavilaSeSekcija[i].section == section)
            return true;
    }
    return false;
}

void updateBilaSekcija(int section, int value)
{
    for (int i = 0; i < pojavilaSeSekcija.size(); i++)
    {
        if (pojavilaSeSekcija[i].section == section)
            pojavilaSeSekcija[i].value = value;
    }
}

int returnSectionValue(int section)
{
    for (int i = 0; i < pojavilaSeSekcija.size(); i++)
    {
        if (pojavilaSeSekcija[i].section == section)
            return pojavilaSeSekcija[i].value;
    }
    return 0;
}

/*bool is_integer(const std::string &s)
{
    return std::regex_match(s, std::regex("[(-|+)|][0-9]+"));
}*/
bool is_integer(string s)
{
    if (s.size() == 0)
        return false;
    for (int i = 0; i < s.size(); i++)
    {
        if ((s[i] >= '0' && s[i] <= '9') == false)
        {
            return false;
        }
    }
    return true;
}

void obradaPodatka(string registar, string operand, vector<string> &curZapis, int counter)
{
    if (operand[0] == '$' && is_integer(operand.substr(1)))
    {
        //cout << operand << ":)";
        string s;
        s.push_back(registar[1]);
        curZapis.push_back(s);            // destinacioni registar je regD
        curZapis.push_back("f");          // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0");          // nema nikakvog azuriranja
        curZapis.push_back("0");          // neposredno adresiranje
        vector<string> pom = insert2BytesWithoutLittleEndian(intToHex(stoi(operand.substr(1)))); // payload instrukcije
        curZapis.insert(curZapis.end(), pom.begin(), pom.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (operand[0] == '$' && Asembler::symbolTable->elemExsist(operand.substr(1)))
    {
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        curZapis.push_back("f"); // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("0"); // neposredno adresiranje
        Symbol s = Asembler::symbolTable->returnElem(operand.substr(1));
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(to_string(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (is_integer(operand))
    {
        //cout << operand << ":)";
        string s;
        s.push_back(registar[1]);
        curZapis.push_back(s);                                         // destinacioni registar je regD
        curZapis.push_back("f");                                       // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0");                                       // nema nikakvog azuriranja
        curZapis.push_back("4");                                       // memorijsko adresiranje
        vector<string> pom = insert2BytesWithoutLittleEndian(intToHex(stoi(operand))); // payload instrukcije
        curZapis.insert(curZapis.end(), pom.begin(), pom.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (Asembler::symbolTable->elemExsist(operand))
    {
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        curZapis.push_back("f"); // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("4"); // memorijsko adresiranje
        Symbol s = Asembler::symbolTable->returnElem(operand);
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(to_string(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (operand[0] == '%' && Asembler::symbolTable->elemExsist(operand.substr(1, operand.size() - 1)))
    {
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        curZapis.push_back("7"); // ovde ide 7 jer je PC adresiranje
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("3"); // registarsko indirektno sa 16-bitnim pomerajem
        Symbol s = Asembler::symbolTable->returnElem(operand.substr(1));
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue()) - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue()) - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_PC16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(0 - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_PC16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (operand == "r0" || operand == "r1" || operand == "r2" || operand == "r3" || operand == "r4" || operand == "r5" ||
             operand == "r6" || operand == "r7" || operand == "sp" || operand == "pc" || operand == "psw")
    {
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        if (operand == "*sp")
            curZapis.push_back("6");
        else if (operand == "*pc")
            curZapis.push_back("7");
        else if (operand == "*psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(operand[1]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("1"); // registarsko direktno adresiranje
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (operand == "[r0]" || operand == "[r1]" || operand == "[r2]" || operand == "[r3]" || operand == "[r4]" || operand == "[r5]" ||
             operand == "[r6]" || operand == "[r7]" || operand == "[sp]" || operand == "[pc]" || operand == "[psw]")
    {
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        if (operand == "[sp]")
            curZapis.push_back("6");
        else if (operand == "[pc]")
            curZapis.push_back("7");
        else if (operand == "[psw]")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr, src registar
            string s;
            s.push_back(operand[2]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("2"); // registarsko indirektno adresiranje
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if ((operand.substr(1, operand.find('+') - 1) == "r0" || operand.substr(1, operand.find('+') - 1) == "r1" || operand.substr(1, operand.find('+') - 1) == "r2" ||
              operand.substr(1, operand.find('+') - 1) == "r3" || operand.substr(1, operand.find('+') - 1) == "r4" || operand.substr(1, operand.find('+') - 1) == "r5" ||
              operand.substr(1, operand.find('+') - 1) == "r6" || operand.substr(1, operand.find('+') - 1) == "r7" || operand.substr(1, operand.find('+') - 1) == "sp" ||
              operand.substr(1, operand.find('+') - 1) == "pc" || operand.substr(1, operand.find('+') - 1) == "psw") &&
             is_integer(operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2)))
    {
        string registarOperanda = operand.substr(1, operand.find('+') - 1);
        string pomeraj = operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2);
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        if (registarOperanda == "sp")
            curZapis.push_back("6");
        else if (registarOperanda == "pc")
            curZapis.push_back("7");
        else if (registarOperanda == "psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(registarOperanda[1]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("3"); // registarsko indirektno adresiranje sa pomerajem
        vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(pomeraj)));
        curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if ((operand.substr(1, operand.find('+') - 1) == "r0" || operand.substr(1, operand.find('+') - 1) == "r1" || operand.substr(1, operand.find('+') - 1) == "r2" ||
              operand.substr(1, operand.find('+') - 1) == "r3" || operand.substr(1, operand.find('+') - 1) == "r4" || operand.substr(1, operand.find('+') - 1) == "r5" ||
              operand.substr(1, operand.find('+') - 1) == "r6" || operand.substr(1, operand.find('+') - 1) == "r7" || operand.substr(1, operand.find('+') - 1) == "sp" ||
              operand.substr(1, operand.find('+') - 1) == "pc" || operand.substr(1, operand.find('+') - 1) == "psw") &&
             Asembler::symbolTable->elemExsist(operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2)))
    {
        string registarOperanda = operand.substr(1, operand.find('+') - 1);
        string simbol = operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2);
        Symbol s = Asembler::symbolTable->returnElem(simbol);
        string str;
        str.push_back(registar[1]);
        curZapis.push_back(str); // destinacioni registar je regD
        if (registar == "*sp")
            curZapis.push_back("6");
        else if (registar == "*pc")
            curZapis.push_back("7");
        else if (registar == "*psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(registarOperanda[1]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("3"); // registarsko indirektno adresiranje sa pomerajem
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    
}

void obradaSkoka(string operand, vector<string> &curZapis, int counter)
{
    //cout << operand;
    if (is_integer(operand))
    {
        //cout << operand << ":)";
        curZapis.push_back("f");                                       // fiksno
        curZapis.push_back("f");                                       // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0");                                       // nema nikakvog azuriranja
        curZapis.push_back("0");                                       // neposredno adresiranje
        vector<string> pom = insert2BytesWithoutLittleEndian(intToHex(stoi(operand))); // payload instrukcije
        curZapis.insert(curZapis.end(), pom.begin(), pom.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (Asembler::symbolTable->elemExsist(operand))
    {
        curZapis.push_back("f"); // fiksno
        curZapis.push_back("f"); // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("0"); // neposredno adresiranje
        Symbol s = Asembler::symbolTable->returnElem(operand);
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(to_string(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (operand[0] == '%' && Asembler::symbolTable->elemExsist(operand.substr(1, operand.size() - 1)))
    {
        curZapis.push_back("f"); // fiksno
        curZapis.push_back("7"); // PC registar
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("5"); // registarsko direktno sa 16-bitnim označenim sabirkom
        Symbol s = Asembler::symbolTable->returnElem(operand.substr(1));
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue()) - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue()) - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_PC16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(0 - 2));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_PC16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (operand[0] == '*' && is_integer(operand.substr(1, operand.size() - 1)))
    {
        curZapis.push_back("f");                                                 // fiksno
        curZapis.push_back("f");                                                 // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0");                                                 // nema nikakvog azuriranja
        curZapis.push_back("4");                                                 // memorijsko adresiranje
        vector<string> pom = insert2BytesWithoutLittleEndian(intToHex(stoi(operand.substr(1)))); // payload instrukcije
        curZapis.insert(curZapis.end(), pom.begin(), pom.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (operand[0] == '*' && Asembler::symbolTable->elemExsist(operand.substr(1, operand.size() - 1)))
    {
        curZapis.push_back("f"); // fiksno
        curZapis.push_back("f"); // ovako cu da oznacavam niza 4b u RegsDescr kada mi nije potreban registar
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("4"); // memorijsko adresiranje
        Symbol s = Asembler::symbolTable->returnElem(operand.substr(1));
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
    else if (operand == "*r0" || operand == "*r1" || operand == "*r2" || operand == "*r3" || operand == "*r4" || operand == "*r5" ||
             operand == "*r6" || operand == "*r7" || operand == "*sp" || operand == "*pc" || operand == "*psw")
    {
        curZapis.push_back("f"); // fiksno
        if (operand == "*sp")
            curZapis.push_back("6");
        else if (operand == "*pc")
            curZapis.push_back("7");
        else if (operand == "*psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(operand[2]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("1"); // registarsko direktno adresiranje
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if (operand == "*[r0]" || operand == "*[r1]" || operand == "*[r2]" || operand == "*[r3]" || operand == "*[r4]" || operand == "*[r5]" ||
             operand == "*[r6]" || operand == "*[r7]" || operand == "*[sp]" || operand == "*[pc]" || operand == "*[psw]")
    {
        curZapis.push_back("f"); // fiksno
        if (operand == "*sp")
            curZapis.push_back("6");
        else if (operand == "*pc")
            curZapis.push_back("7");
        else if (operand == "*psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(operand[3]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("2"); // registarsko indirektno adresiranje
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if ((operand.substr(2, operand.find('+') - 2) == "r0" || operand.substr(2, operand.find('+') - 2) == "r1" || operand.substr(2, operand.find('+') - 2) == "r2" ||
              operand.substr(2, operand.find('+') - 2) == "r3" || operand.substr(2, operand.find('+') - 2) == "r4" || operand.substr(2, operand.find('+') - 2) == "r5" ||
              operand.substr(2, operand.find('+') - 2) == "r6" || operand.substr(2, operand.find('+') - 2) == "r7" || operand.substr(2, operand.find('+') - 2) == "sp" ||
              operand.substr(2, operand.find('+') - 2) == "pc" || operand.substr(2, operand.find('+') - 2) == "psw") &&
             is_integer(operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2)))
    {
        string registar = operand.substr(2, operand.find('+') - 2);
        string pomeraj = operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2);
        curZapis.push_back("f"); // fiksno
        if (registar == "sp")
            curZapis.push_back("6");
        else if (registar == "pc")
            curZapis.push_back("7");
        else if (registar == "psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(registar[1]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("3"); // registarsko indirektno adresiranje sa pomerajem
        vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(pomeraj)));
        curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
        curSection.setZapis(curZapis);
        Asembler::symbolTable->updateElem(curSection);
    }
    else if ((operand.substr(2, operand.find('+') - 2) == "r0" || operand.substr(2, operand.find('+') - 2) == "r1" || operand.substr(2, operand.find('+') - 2) == "r2" ||
              operand.substr(2, operand.find('+') - 2) == "r3" || operand.substr(2, operand.find('+') - 2) == "r4" || operand.substr(2, operand.find('+') - 2) == "r5" ||
              operand.substr(2, operand.find('+') - 2) == "r6" || operand.substr(2, operand.find('+') - 2) == "r7" || operand.substr(2, operand.find('+') - 2) == "sp" ||
              operand.substr(2, operand.find('+') - 2) == "pc" || operand.substr(2, operand.find('+') - 2) == "psw") &&
             Asembler::symbolTable->elemExsist(operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2)))
    {
        string registar = operand.substr(2, operand.find('+') - 2);
        string simbol = operand.substr(operand.find('+') + 1, operand.size() - operand.find('+') - 2);
        Symbol s = Asembler::symbolTable->returnElem(simbol);
        curZapis.push_back("f"); // fiksno
        if (registar == "sp")
            curZapis.push_back("6");
        else if (registar == "pc")
            curZapis.push_back("7");
        else if (registar == "psw")
            curZapis.push_back("8");
        else
        { // niza 4b u RegsDescr
            string s;
            s.push_back(registar[1]);
            curZapis.push_back(s);
        }
        curZapis.push_back("0"); // nema nikakvog azuriranja
        curZapis.push_back("3"); // registarsko indirektno adresiranje sa pomerajem
        if (s.getSection() == "1")
        { // apsolutni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'l')
        { // lokalni simbol
            //cout<<"AAAAAAA";
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(stoi(s.getValue())));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
        else if (s.getVisibility() == 'g')
        { // globalni simbol
            vector<string> tmp = insert2BytesWithoutLittleEndian(intToHex(0));
            curZapis.insert(curZapis.end(), tmp.begin(), tmp.end());
            curSection.setZapis(curZapis);
            Asembler::symbolTable->updateElem(curSection);
            RelocationRecord r(to_string(counter + 3), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
            Asembler::relocationTable->addElem(r);
            vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
            relZapisi.push_back(r);
            curSection.setRelokacioniZapis(relZapisi);
            Asembler::symbolTable->updateElem(curSection);
        }
    }
}

//===============================================

void Asembler::firstPass(vector<vector<string>> &parsedInput, ofstream& output)
{
    if(this->greska) {
        cout<<"Greska u parsiranju!!!"<<endl;
        return;
    }
    for (int i = 0; i < parsedInput.size(); i++)
    {
        if(parsedInput.at(i).size() > 0) firstPass(parsedInput.at(i));
        if(this->greska == true){
            cout<<"Pronadjena greska na liniji: "<<i<<endl;
            return;
        }
    }
    if (this->greska == true)
    {
        cout << "GRESKA PRONADJENA U PRVOM PROLAZU ASEMBLERA!!!" << endl;
        return;
    }
    //symbolTable->print();
    cout << "*************************************" << endl;
    symbolTable->reorganize();
    symbolTable->print();
    output<<endl<<"TABELA SIMBOLA" <<endl;
    symbolTable->print(output);
}

void Asembler::firstPass(vector<string> &line)
{
    if (line.at(0) == ".global")
    {
        for (int i = 1; i < line.size(); i++)
        {
            if(symbolTable->elemExsist(line.at(1))){
                //cout<<line.at(1) << "/////////////"<<endl;
                Symbol s = symbolTable->returnElem(line.at(1));
                s.setVisibility('g');
                symbolTable->updateElem(s);
            }
            else {
                Symbol *s = new Symbol(line.at(i), 'g');
                symbolTable->addElem(*s);
            }
            
        }
    }
    else if (line.at(0) == ".extern")
    {
        for (int i = 1; i < line.size(); i++)
        {
            if (symbolTable->elemExsist(line.at(i)))
            {
                this->greska = true;
                cout << "SIMBOL " << line.at(i) << "JE VEC DEFINISAN" << endl;
            }
            else
            {
                Symbol *s = new Symbol(line.at(i), 'g', "0");
                symbolTable->addElem(*s);
            }
        }
    }
    else if (line.at(0) == ".section")
    {
        if (currentSection != 0)
        {
            Symbol s = symbolTable->returnElem(currentSection);
            s.setSize(locationCounter);
            symbolTable->updateElem(s);
        }
        if (symbolTable->elemExsist(line.at(1)))
        {
            Symbol s = symbolTable->returnElem(line.at(1));
            locationCounter = s.getSize();
            currentSection = s.getNumber();
        }
        else
        {
            Symbol *s = new Symbol(line.at(1), 'l', to_string(s->ID));
            currentSection = s->getNumber();
            symbolTable->addElem(*s);
            locationCounter = 0;
        }
    }
    else if (line.at(0) == ".text")
    {
        if (currentSection != 0)
        {
            Symbol s = symbolTable->returnElem(currentSection);
            s.setSize(locationCounter);
            symbolTable->updateElem(s);
        }
        if (symbolTable->elemExsist(line.at(0)))
        {
            Symbol s = symbolTable->returnElem(line.at(0));
            locationCounter = s.getSize();
            currentSection = s.getNumber();
        }
        else
        {
            Symbol *s = new Symbol(line.at(0), 'l', to_string(s->ID));
            currentSection = s->getNumber();
            symbolTable->addElem(*s);
            locationCounter = 0;
        }
    }
    else if (line.at(0) == ".data")
    {
        if (currentSection != 0)
        {
            Symbol s = symbolTable->returnElem(currentSection);
            s.setSize(locationCounter);
            symbolTable->updateElem(s);
        }
        if (symbolTable->elemExsist(line.at(0)))
        {
            Symbol s = symbolTable->returnElem(line.at(0));
            locationCounter = s.getSize();
            currentSection = s.getNumber();
        }
        else
        {
            Symbol *s = new Symbol(line.at(0), 'l', to_string(s->ID));
            currentSection = s->getNumber();
            symbolTable->addElem(*s);
            locationCounter = 0;
        }
    }
    else if (line.at(0) == ".bss")
    {
        if (currentSection != 0)
        {
            Symbol s = symbolTable->returnElem(currentSection);
            s.setSize(locationCounter);
            symbolTable->updateElem(s);
        }
        if (symbolTable->elemExsist(line.at(0)))
        {
            Symbol s = symbolTable->returnElem(line.at(0));
            locationCounter = s.getSize();
            currentSection = s.getNumber();
        }
        else
        {
            Symbol *s = new Symbol(line.at(0), 'l', to_string(s->ID));
            currentSection = s->getNumber();
            symbolTable->addElem(*s);
            locationCounter = 0;
        }
    }
    else if (line.at(0) == ".end")
    {
        if (currentSection != 0)
        {
            Symbol s = symbolTable->returnElem(currentSection);
            s.setSize(locationCounter);
            symbolTable->updateElem(s);
        }
    }
    else if ((line.at(0).find(':') != std::string::npos) || (line.size() > 1) && line.at(1) == ":")
    {
        string labela;
        if (line.at(0).find(':') != std::string::npos)
        {
            labela = string(line.at(0).begin(), line.at(0).end() - 1);
        }
        else
        {
            labela = line.at(0);
        }

        if (symbolTable->elemExsist(labela))
        {
            Symbol s = symbolTable->returnElem(labela);
            if (s.getVisibility() == 'g'  && s.getSection() != "UND")
            {
                s.setSection(to_string(currentSection));
                s.setValue(to_string(locationCounter));
                symbolTable->updateElem(s);
            }
            else
            {
                this->greska = true;
                cout << "SIMBOL " << labela << " JE VEC DEFINISAN ILI JE EKSTERNI SIMBOL" << endl;
            }
        }
        else
        {
            Symbol *s;
            s = new Symbol(labela, to_string(currentSection), to_string(locationCounter), 'l', 0, 1);
            symbolTable->addElem(*s);
        }
        vector<string> linijaNakonLabele;
        if(line.size() > 2 && line.at(1) == ":")
        {
            for (int k = 2; k < line.size(); k++)
            {
                linijaNakonLabele.push_back(line.at(k));
                //cout << "------" << line.at(k) << "-------" << endl;
            }
            firstPass(linijaNakonLabele);
            for(int i = 0; i < linijaNakonLabele.size(); i++){
                line.at(2 + i) = linijaNakonLabele.at(i);
            }
            for(int i = line.size(); i > 2 + linijaNakonLabele.size(); i--){
                line.at(i).clear();
            }
        }
        else if(line.size() > 1 && line.at(1) != ":")
        {
            for (int k  = 1; k < line.size(); k++)
            {
                linijaNakonLabele.push_back(line.at(k));
                //cout << "------" << line.at(k) << "-------" << endl;
            }
            
            firstPass(linijaNakonLabele);
            for(int i = 0; i < linijaNakonLabele.size(); i++){
                line.at(1 + i) = linijaNakonLabele.at(i);
            }
            for(int i = line.size(); i > 1 + linijaNakonLabele.size(); i--){
                line.at(i).clear();
            }
        }
       

        
    }
    else if (line.at(0) == ".word")
    {
        for (int i = 1; i < line.size(); i++)
        {
            if (line.at(i)[0] == '0' && line.at(i)[1] == 'x')
            {
                string s;
                bool vodecaNula = true;
                for (int j = 2; j < line.at(i).size(); j++)
                {
                    if (line.at(i).at(j) != '0' || (line.at(i).at(j) == '0' && !vodecaNula))
                    {
                        s += line.at(i).at(j);
                        vodecaNula = false;
                    }
                }
                line.at(i) = to_string(hexToInt(s));
            }
        }
        locationCounter += (line.size() - 1) * 2;
    }
    else if (line.at(0) == ".skip")
    {
        if (line.at(1).size() > 2 && line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
        }
        locationCounter += stoi(line.at(1));
    }
    else if (line.at(0) == ".equ")
    {
        if (line.at(2)[0] == '0' && line.at(2)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(2).size(); j++)
            {
                if (line.at(2).at(j) != '0' || (line.at(2).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(2).at(j);
                    vodecaNula = false;
                }
            }
            //cout << s;
            line.at(2) = to_string(hexToInt(s));
        }
        /*if (symbolTable->elemExsist(line.at(1)) && symbolTable->returnElem(line.at(1)).getVisibility() != 'g')
        {
            this->greska = true;
            cout<<"Nema dodele equ direktivom elementu koji vec postoji"<<endl;
        }*/
       
            if(symbolTable->elemExsist(line.at(1)) && symbolTable->returnElem(line.at(1)).getVisibility() == 'g'){
                Symbol s = symbolTable->returnElem(line.at(1));
                s.setSection("1");
                s.setValue(line.at(2));
                symbolTable->updateElem(s);
            }
            else{
                Symbol *s = new Symbol(line.at(1), "1", to_string(stoi(line.at(2))), 'l', 0, 1);
                symbolTable->addElem(*s);
            }
            
        
    }
    else if (line.at(0) == "halt")
    {
        locationCounter += 1;
    }
    else if (line.at(0) == "int")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "iret")
    {
        locationCounter += 1;
    }
    else if (line.at(0) == "call")
    {
        if (line.size() > 2)
        {
            for (int i = 2; i < line.size(); i++)
                line.at(1).append(line.at(i));
            for (int i = 2; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(1)[0] == '*' && line.at(1)[1] == '0' && line.at(1)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = "*";
            line.at(1).append(to_string(hexToInt(s)));
        }
        else if (line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
            //cout<<line.at(1)<<endl;
        }
        else if (line.at(1).find('+') != std::string::npos && (line.at(1).substr(2, line.at(1).find('+') - 2) == "r0" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r1" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r2" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r3" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r4" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r5" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r6" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r7" || line.at(1).substr(2, line.at(1).find('+') - 2) == "sp" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "pc" || line.at(1).substr(2, line.at(1).find('+') - 2) == "psw") &&
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[0] == '0' && 
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[1] == 'x'){
                 string literal = line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(1).substr(0, line.at(1).find('+') + 1) + literal + ']';

            line.at(1) = linija;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "ret")
    {
        locationCounter += 1;
    }
    else if (line.at(0) == "jmp")
    {
        if (line.size() > 2)
        {
            for (int i = 2; i < line.size(); i++)
                line.at(1).append(line.at(i));
            for (int i = 2; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(1)[0] == '*' && line.at(1)[1] == '0' && line.at(1)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = "*";
            line.at(1).append(to_string(hexToInt(s)));
        }
        else if (line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
        }
        else if (line.at(1).find('+') != std::string::npos &&(line.at(1).substr(2, line.at(1).find('+') - 2) == "r0" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r1" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r2" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r3" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r4" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r5" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r6" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r7" || line.at(1).substr(2, line.at(1).find('+') - 2) == "sp" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "pc" || line.at(1).substr(2, line.at(1).find('+') - 2) == "psw") &&
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[0] == '0' && 
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[1] == 'x'){
                 string literal = line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(1).substr(0, line.at(1).find('+') + 1) + literal + ']';

            line.at(1) = linija;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jeq")
    {
        if (line.size() > 2)
        {
            for (int i = 2; i < line.size(); i++)
                line.at(1).append(line.at(i));
            for (int i = 2; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(1)[0] == '*' && line.at(1)[1] == '0' && line.at(1)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = "*";
            line.at(1).append(to_string(hexToInt(s)));
        }
        else if (line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
        }
        else if (line.at(1).find('+') != std::string::npos &&(line.at(1).substr(2, line.at(1).find('+') - 2) == "r0" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r1" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r2" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r3" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r4" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r5" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r6" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r7" || line.at(1).substr(2, line.at(1).find('+') - 2) == "sp" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "pc" || line.at(1).substr(2, line.at(1).find('+') - 2) == "psw") &&
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[0] == '0' && 
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[1] == 'x'){
                 string literal = line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(1).substr(0, line.at(1).find('+') + 1) + literal + ']';

            line.at(1) = linija;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jne")
    {
        if (line.size() > 2)
        {
            for (int i = 2; i < line.size(); i++)
                line.at(1).append(line.at(i));
            for (int i = 2; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(1)[0] == '*' && line.at(1)[1] == '0' && line.at(1)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = "*";
            line.at(1).append(to_string(hexToInt(s)));
        }
        else if (line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
        }
        else if (line.at(1).find('+') != std::string::npos &&(line.at(1).substr(2, line.at(1).find('+') - 2) == "r0" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r1" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r2" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r3" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r4" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r5" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r6" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r7" || line.at(1).substr(2, line.at(1).find('+') - 2) == "sp" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "pc" || line.at(1).substr(2, line.at(1).find('+') - 2) == "psw") &&
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[0] == '0' && 
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[1] == 'x'){
                 string literal = line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(1).substr(0, line.at(1).find('+') + 1) + literal + ']';

            line.at(1) = linija;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jgt")
    {
        if (line.size() > 2)
        {
            for (int i = 2; i < line.size(); i++)
                line.at(1).append(line.at(i));
            for (int i = 2; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(1)[0] == '*' && line.at(1)[1] == '0' && line.at(1)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = "*";
            line.at(1).append(to_string(hexToInt(s)));
        }
        else if (line.at(1)[0] == '0' && line.at(1)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(1).size(); j++)
            {
                if (line.at(1).at(j) != '0' || (line.at(1).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(1).at(j);
                    vodecaNula = false;
                }
            }
            line.at(1) = to_string(hexToInt(s));
        }
        else if (line.at(1).find('+') != std::string::npos &&(line.at(1).substr(2, line.at(1).find('+') - 2) == "r0" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r1" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r2" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r3" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r4" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r5" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "r6" || line.at(1).substr(2, line.at(1).find('+') - 2) == "r7" || line.at(1).substr(2, line.at(1).find('+') - 2) == "sp" ||
              line.at(1).substr(2, line.at(1).find('+') - 2) == "pc" || line.at(1).substr(2, line.at(1).find('+') - 2) == "psw") &&
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[0] == '0' && 
             line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2)[1] == 'x'){
                 string literal = line.at(1).substr(line.at(1).find('+') + 1, line.at(1).size() - line.at(1).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(1).substr(0, line.at(1).find('+') + 1) + literal + ']';

            line.at(1) = linija;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "push")
    {
        locationCounter += 3;
    }
    else if (line.at(0) == "pop")
    {
        locationCounter += 3;
    }
    else if (line.at(0) == "xchg")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "add")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "sub")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "mul")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "div")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "cmp")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "not")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "and")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "or")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "xor")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "test")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "shl")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "shr")
    {
        locationCounter += 2;
    }
    else if (line.at(0) == "ldr")
    {
        if (line.size() > 3)
        {
            for (int i = 3; i < line.size(); i++)
                line.at(2).append(line.at(i));
            for (int i = 3; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(2)[0] == '$' && line.at(2)[1] == '0' && line.at(2)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(2).size(); j++)
            {
                if (line.at(2).at(j) != '0' || (line.at(2).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(2).at(j);
                    vodecaNula = false;
                }
            }
            line.at(2) = "$";
            line.at(2).append(to_string(hexToInt(s)));
        }
        if (line.at(2)[0] == '0' && line.at(2)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(2).size(); j++)
            {
                if (line.at(2).at(j) != '0' || (line.at(2).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(2).at(j);
                    vodecaNula = false;
                }
            }
            line.at(2) = to_string(hexToInt(s));
        }

        if (line.at(2).find('+') != std::string::npos && (line.at(2).substr(1, line.at(2).find('+') - 1) == "r0" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r1" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r2" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "r3" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r4" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r5" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "r6" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r7" || line.at(2).substr(1, line.at(2).find('+') - 1) == "sp" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "pc" || line.at(2).substr(1, line.at(2).find('+') - 1) == "psw") &&
            line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2)[0] == '0' &&
            line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2)[1] == 'x')
        {
            //cout<<"AAAAAAAAA";
            string literal = line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(2).substr(0, line.at(2).find('+') + 1) + literal + ']';

            line.at(2) = linija;
        }

        int uvecaj = 5;
        //kad mi je operand r0 itd, i kad je [r0] itd onda je samo 3, dakle dodaj uslov za [r0]
        if (line.at(2) == "r0" || line.at(2) == "r1" || line.at(2) == "r2" || line.at(2) == "r3" || line.at(2) == "r4" || line.at(2) == "r5" || line.at(2) == "r6" || line.at(2) == "r7" || line.at(2) == "sp" || line.at(2) == "pc" || line.at(2) == "psw" ||
            line.at(2) == "[r0]" || line.at(2) == "[r1]" || line.at(2) == "[r2]" || line.at(2) == "[r3]" || line.at(2) == "[r4]" || line.at(2) == "[r5]" || line.at(2) == "[r6]" || line.at(2) == "[r7]" || line.at(2) == "[sp]" || line.at(2) == "[pc]" || line.at(2) == "[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "str")
    {
        if (line.size() > 3)
        {
            for (int i = 3; i < line.size(); i++)
                line.at(2).append(line.at(i));
            for (int i = 3; i < line.size(); i++)
                line.at(i).clear();
        }
        if (line.at(2)[0] == '$' && line.at(2)[1] == '0' && line.at(2)[2] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 3; j < line.at(2).size(); j++)
            {
                if (line.at(2).at(j) != '0' || (line.at(2).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(2).at(j);
                    vodecaNula = false;
                }
            }
            line.at(2) = '$';
            line.at(2).append(to_string(hexToInt(s)));
        }
        if (line.at(2)[0] == '0' && line.at(2)[1] == 'x')
        {
            string s;
            bool vodecaNula = true;
            for (int j = 2; j < line.at(2).size(); j++)
            {
                if (line.at(2).at(j) != '0' || (line.at(2).at(j) == '0' && !vodecaNula))
                {
                    s += line.at(2).at(j);
                    vodecaNula = false;
                }
            }
            line.at(2) = to_string(hexToInt(s));
        }

        if (line.at(2).find('+') != std::string::npos && (line.at(2).substr(1, line.at(2).find('+') - 1) == "r0" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r1" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r2" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "r3" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r4" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r5" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "r6" || line.at(2).substr(1, line.at(2).find('+') - 1) == "r7" || line.at(2).substr(1, line.at(2).find('+') - 1) == "sp" ||
             line.at(2).substr(1, line.at(2).find('+') - 1) == "pc" || line.at(2).substr(1, line.at(2).find('+') - 1) == "psw") &&
            line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2)[0] == '0' &&
            line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2)[1] == 'x')
        {
            //cout<<"AAAAAAAAA";
            string literal = line.at(2).substr(line.at(2).find('+') + 1, line.at(2).size() - line.at(2).find('+') - 2);

            string s;
            bool vodecaNula = true;
            for (int j = 2; j < literal.size(); j++)
            {
                if (literal.at(j) != '0' || (literal.at(j) == '0' && !vodecaNula))
                {
                    s += literal.at(j);
                    vodecaNula = false;
                }
            }

            literal = to_string(hexToInt(s));
            string linija = line.at(2).substr(0, line.at(2).find('+') + 1) + literal + ']';

            line.at(2) = linija;
        }
        int uvecaj = 5;
        if (line.at(2) == "r0" || line.at(2) == "r1" || line.at(2) == "r2" || line.at(2) == "r3" || line.at(2) == "r4" || line.at(2) == "r5" || line.at(2) == "r6" || line.at(2) == "r7" || line.at(2) == "sp" || line.at(2) == "pc" || line.at(2) == "psw" ||
            line.at(2) == "[r0]" || line.at(2) == "[r1]" || line.at(2) == "[r2]" || line.at(2) == "[r3]" || line.at(2) == "[r4]" || line.at(2) == "[r5]" || line.at(2) == "[r6]" || line.at(2) == "[r7]" || line.at(2) == "[sp]" || line.at(2) == "[pc]" || line.at(2) == "[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else
    {
        cout << "Greska sa prvim parametrom:  " << line.at(0) << endl;
        this->greska = true;
    }
}

// D R U G I     P R O L A Z
//==============================================================

void Asembler::secondPass(vector<vector<string>> &parsedInput, ofstream& output)
{
    for (int i = 0; i < parsedInput.size(); i++)
    {

        /*if(parsedInput.at(i).size() > 0)*/ secondPass(parsedInput.at(i));
        if (this->greska == true){
        cout<<"Greska na liniji "<<i + 1<< " u drugom prolazu asemblera"<<endl;
        break;
        }
    }
    if (this->greska == true)
    {
        cout << "GRESKA PRONADJENA U DRUGOM PROLAZU ASEMBLERA!!!" << endl;
        return;
    }
    cout << "ZAPISI :" << endl;
    output << endl <<"ZAPISI :" << endl;
    symbolTable->printZapis();
    symbolTable->printZapis(output);
    cout << "RELOKACIONA TABELA: " << endl;
    output<<endl;
    output << "RELOKACIONA TABELA: " << endl << endl;
    //output << "RELOKACIONA TABELA: " << endl;
    //relocationTable->print();
    // relocationTable->print(output);
    symbolTable->printRelokacioniZapis();
    symbolTable->printRelokacioniZapis(output);
}

void Asembler::secondPass(vector<string> &line)
{

    if (line.at(0) == ".global")
    {
    }
    else if (line.at(0) == ".extern")
    {
    }
    else if (line.at(0) == ".section")
    {
        if (curSection.getNumber() != 0)
        {
            updateBilaSekcija(curSection.getNumber(), locationCounter);
        }
        Symbol s = symbolTable->returnElem(line.at(1));
        curSection = s;
        if (bilaSekcija(curSection.getNumber()))
        {
            locationCounter = returnSectionValue(curSection.getNumber());
            currentSection = s.getNumber();
        }
        else
        {
            locationCounter = 0;
            currentSection = s.getNumber();
            pojavilaSeSekcija.push_back({s.getNumber(), 0});
        }
    }
    else if (line.at(0) == ".text")
    {
        if (curSection.getNumber() != 0)
        {
            updateBilaSekcija(curSection.getNumber(), locationCounter);
        }
        Symbol s = symbolTable->returnElem(line.at(0));
        curSection = s;
        if (bilaSekcija(curSection.getNumber()))
        {
            locationCounter = returnSectionValue(curSection.getNumber());
            currentSection = s.getNumber();
        }
        else
        {
            locationCounter = 0;
            currentSection = s.getNumber();
            pojavilaSeSekcija.push_back({s.getNumber(), 0});
        }
    }
    else if (line.at(0) == ".data")
    {
        if (curSection.getNumber() != 0)
        {
            updateBilaSekcija(curSection.getNumber(), locationCounter);
        }
        Symbol s = symbolTable->returnElem(line.at(0));
        curSection = s;
        if (bilaSekcija(curSection.getNumber()))
        {
            locationCounter = returnSectionValue(curSection.getNumber());
            currentSection = s.getNumber();
        }
        else
        {
            locationCounter = 0;
            currentSection = s.getNumber();
            pojavilaSeSekcija.push_back({s.getNumber(), 0});
        }
    }
    else if (line.at(0) == ".bss")
    {
        if (curSection.getNumber() != 0)
        {
            updateBilaSekcija(curSection.getNumber(), locationCounter);
        }
        Symbol s = symbolTable->returnElem(line.at(0));
        curSection = s;
        if (bilaSekcija(curSection.getNumber()))
        {
            locationCounter = returnSectionValue(curSection.getNumber());
            currentSection = s.getNumber();
        }
        else
        {
            locationCounter = 0;
            currentSection = s.getNumber();
            pojavilaSeSekcija.push_back({s.getNumber(), 0});
        }
    }
    else if (line.at(0) == ".end")
    {
        return;
    }
    else if ((line.at(0).find(':') != std::string::npos) || (line.size() > 1 && line.at(1) == ":"))
    {
        
        vector<string> linijaNakonLabele;
        if(line.size() > 2 && line.at(1) == ":")
        {
            for (int k = 2; k < line.size(); k++)
            {
                linijaNakonLabele.push_back(line.at(k));
                //cout << "------" << line.at(k) << "-------" << endl;
            }
            secondPass(linijaNakonLabele);
        }
        else if(line.size() > 1 && line.at(1) != ":")
        {
            for (int k  = 1; k < line.size(); k++)
            {
                linijaNakonLabele.push_back(line.at(k));
                //cout << "------" << line.at(k) << "-------" << endl;
            }
            
            
            secondPass(linijaNakonLabele);
        } 
    }
    else if (line.at(0) == ".word")
    {
        for (int i = 1; i < line.size(); i++)
        {
            
            if (symbolTable->elemExsist(line.at(i)))
            {
                Symbol s = symbolTable->returnElem(line.at(i));
                if (s.getSection() == "1")
                { // apsolutni simbol
                    vector<string> tmp = insert2BytesWithLittleEndian(intToHex(stoi(s.getValue())));
                    vector<string> pom = curSection.getZapis();
                    pom.insert(pom.end(), tmp.begin(), tmp.end());
                    curSection.setZapis(pom);
                    symbolTable->updateElem(curSection);
                }
                else if (s.getVisibility() == 'l')
                { // lokalni simbol
                    //cout<<"AAAAAAA";
                    vector<string> tmp = insert2BytesWithLittleEndian(intToHex(stoi(s.getValue())));
                    vector<string> trenutniZapis = curSection.getZapis();
                    trenutniZapis.insert(trenutniZapis.end(), tmp.begin(), tmp.end());
                    curSection.setZapis(trenutniZapis);
                    symbolTable->updateElem(curSection);
                    RelocationRecord r(to_string(locationCounter), "R_386_16", stoi(s.getSection()), stoi(curSection.getSection()));
                    relocationTable->addElem(r);
                    vector<RelocationRecord>& relZapisi = curSection.getRelokacioniZapis();
                    relZapisi.push_back(r);
                    curSection.setRelokacioniZapis(relZapisi);
                    Asembler::symbolTable->updateElem(curSection);
                }
                else if (s.getVisibility() == 'g')
                { // globalni simbol
                    vector<string> tmp = insert2BytesWithLittleEndian(to_string(0));
                    vector<string> trenutniZapis = curSection.getZapis();
                    trenutniZapis.insert(trenutniZapis.end(), tmp.begin(), tmp.end());
                    curSection.setZapis(trenutniZapis);
                    symbolTable->updateElem(curSection);
                    RelocationRecord r(to_string(locationCounter), "R_386_16", s.getNumber(), s.getSection() == "UND" ? 0 : stoi(curSection.getSection()));
                    relocationTable->addElem(r);
                    vector<RelocationRecord> relZapisi = curSection.getRelokacioniZapis();
                    relZapisi.push_back(r);
                    curSection.setRelokacioniZapis(relZapisi);
                    Asembler::symbolTable->updateElem(curSection);
                }
            }
            else
            { //literal
                vector<string> tmp = insert2BytesWithLittleEndian(intToHex(stoi(line.at(i))));
                vector<string> pom = curSection.getZapis();
                pom.insert(pom.end(), tmp.begin(), tmp.end());
                curSection.setZapis(pom);
                symbolTable->updateElem(curSection);
            }
            locationCounter += 2;
        }
    }
    else if (line.at(0) == ".skip")
    {

        vector<string> tmp;
        for (int i = 0; i < 2 * stoi(line.at(1)); i++)
        {
            tmp.push_back("0");
        }
        vector<string> currentZapis = curSection.getZapis();
        currentZapis.insert(currentZapis.end(), tmp.begin(), tmp.end());
        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += stoi(line.at(1));
    }
    else if (line.at(0) == ".equ")
    {
    }
    else if (line.at(0) == ".end")
    {
        return;
    }
    else if (line.at(0) == "halt")
    {
        vector<string> currentZapis = curSection.getZapis();
        currentZapis.push_back("0");
        currentZapis.push_back("0");
        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 1;
    }
    else if (line.at(0) == "int")
    {

        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("1");
        currentZapis.push_back("0");

        // registar
        if (line.at(1) == "sp")
            currentZapis.push_back("6");
        else if (line.at(1) == "pc")
            currentZapis.push_back("7");
        else if (line.at(1) == "psw")
            currentZapis.push_back("8");
        else
        {
            string str = "";
            str += line.at(1)[1];
            currentZapis.push_back(str);
        }
        currentZapis.push_back("f");

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line[0] == "iret")
    {

        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("2");
        currentZapis.push_back("0");

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 1;
    }
    else if (line.at(0) == "call")
    {
        vector<string> curZapis = curSection.getZapis();
        // kod instrukcije
        curZapis.push_back("3");
        curZapis.push_back("0");
        obradaSkoka(line.at(1), curZapis, locationCounter);
        if(curZapis.at(curZapis.size() - 2) == "3" && curZapis.at(curZapis.size() - 1) == "0"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "ret")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("4");
        currentZapis.push_back("0");

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 1;
    }
    else if (line.at(0) == "jmp")
    {
        vector<string> curZapis = curSection.getZapis();
        // kod instrukcije
        curZapis.push_back("5");
        curZapis.push_back("0");
        obradaSkoka(line.at(1), curZapis, locationCounter);
        if(curZapis.at(curZapis.size() - 2) == "5" && curZapis.at(curZapis.size() - 1) == "0"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jeq")
    {
        vector<string> curZapis = curSection.getZapis();
        // kod instrukcije
        curZapis.push_back("5");
        curZapis.push_back("1");
        obradaSkoka(line.at(1), curZapis, locationCounter);
        if(curZapis.at(curZapis.size() - 2) == "5" && curZapis.at(curZapis.size() - 1) == "1"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jne")
    {
        vector<string> curZapis = curSection.getZapis();
        // kod instrukcije
        curZapis.push_back("5");
        curZapis.push_back("2");
        obradaSkoka(line.at(1), curZapis, locationCounter);
        if(curZapis.at(curZapis.size() - 2) == "5" && curZapis.at(curZapis.size() - 1) == "2"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "jgt")
    {
        vector<string> curZapis = curSection.getZapis();
        // kod instrukcije
        curZapis.push_back("5");
        curZapis.push_back("3");
        obradaSkoka(line.at(1), curZapis, locationCounter);
        if(curZapis.at(curZapis.size() - 2) == "5" && curZapis.at(curZapis.size() - 1) == "3"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(1) == "*r0" || line.at(1) == "*r1" || line.at(1) == "*r2" || line.at(1) == "*r3" || line.at(1) == "*r4" || line.at(1) == "*r5" ||
            line.at(1) == "*r6" || line.at(1) == "*r7" || line.at(1) == "*sp" || line.at(1) == "*pc" || line.at(1) == "*psw" ||
            line.at(1) == "*[r0]" || line.at(1) == "*[r1]" || line.at(1) == "*[r2]" || line.at(1) == "*[r3]" || line.at(1) == "*[r4]" || line.at(1) == "*[r5]" ||
            line.at(1) == "*[r6]" || line.at(1) == "*[r7]" || line.at(1) == "*[sp]" || line.at(1) == "*[pc]" || line.at(1) == "*[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "push")
    {
        vector<string> curZapis = curSection.getZapis();
        curZapis.push_back("B");        // instrukcija smestanja podataka
        curZapis.push_back("0");        //fiksno
        string s;
        s.push_back(line.at(1)[1]);
        curZapis.push_back(s); // destinacioni registar
        curZapis.push_back("6");        // source registar je sp
        curZapis.push_back("1");        // umanjuje se za 2 pre formiranja adrese operanda
        curZapis.push_back("2");        // registarsko indirektno adresiranje

        curSection.setZapis(curZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 3;
    }
    else if (line.at(0) == "pop")
    {
        vector<string> curZapis = curSection.getZapis();
        curZapis.push_back("A");        // instrukcija ucitavanja podataka
        curZapis.push_back("0");        //fiksno
        string s;
        s.push_back(line.at(1)[1]);
        curZapis.push_back(s); // destinacioni registar
        curZapis.push_back("6");        // source registar je sp
        curZapis.push_back("4");        // uvecava se za 2 nakon formiranja adrese operanda
        curZapis.push_back("2");        // registarsko indirektno adresiranje

        curSection.setZapis(curZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 3;
    }
    else if (line.at(0) == "xchg")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("6");
        currentZapis.push_back("0");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "add")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("7");
        currentZapis.push_back("0");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "sub")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("7");
        currentZapis.push_back("1");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "mul")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("7");
        currentZapis.push_back("2");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "div")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("7");
        currentZapis.push_back("3");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "cmp")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("7");
        currentZapis.push_back("4");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "not")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("8");
        currentZapis.push_back("0");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        currentZapis.push_back("0"); // nemam drugi registar

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "and")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("8");
        currentZapis.push_back("1");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "or")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("8");
        currentZapis.push_back("2");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "xor")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("8");
        currentZapis.push_back("3");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "test")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("8");
        currentZapis.push_back("4");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "shl")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("9");
        currentZapis.push_back("0");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "shr")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("9");
        currentZapis.push_back("1");

        // indeks prvog registra
        string prvi;
        if (line.at(1) == "sp")
            prvi.push_back('6');
        else if (line.at(1) == "pc")
            prvi.push_back('7');
        else
            prvi.push_back(line.at(1)[1]);
        currentZapis.push_back(prvi);

        // indeks drugog registra
        string drugi;
        if (line.at(2) == "sp")
            drugi.push_back('6');
        else if (line.at(2) == "pc")
            drugi.push_back('7');
        else
            drugi.push_back(line.at(2)[1]);
        currentZapis.push_back(drugi);

        curSection.setZapis(currentZapis);
        symbolTable->updateElem(curSection);
        locationCounter += 2;
    }
    else if (line.at(0) == "ldr")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("a");
        currentZapis.push_back("0");
        obradaPodatka(line.at(1), line.at(2), currentZapis, locationCounter);
        if(currentZapis.at(currentZapis.size() - 2) == "a" && currentZapis.at(currentZapis.size() - 1) == "0"){
            this->greska = true;
        }

        int uvecaj = 5;
        //kad mi je operand r0 itd, i kad je [r0] itd onda je samo 3, dakle dodaj uslov za [r0]
        if (line.at(2) == "r0" || line.at(2) == "r1" || line.at(2) == "r2" || line.at(2) == "r3" || line.at(2) == "r4" || line.at(2) == "r5" || line.at(2) == "r6" || line.at(2) == "r7" || line.at(2) == "sp" || line.at(2) == "pc" || line.at(2) == "psw" ||
            line.at(2) == "[r0]" || line.at(2) == "[r1]" || line.at(2) == "[r2]" || line.at(2) == "[r3]" || line.at(2) == "[r4]" || line.at(2) == "[r5]" || line.at(2) == "[r6]" || line.at(2) == "[r7]" || line.at(2) == "[sp]" || line.at(2) == "[pc]" || line.at(2) == "[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
    else if (line.at(0) == "str")
    {
        vector<string> currentZapis = curSection.getZapis();
        // kod instrukcije
        currentZapis.push_back("b");
        currentZapis.push_back("0");
        obradaPodatka(line.at(1), line.at(2), currentZapis, locationCounter);
        if(currentZapis.at(currentZapis.size() - 2) == "b" && currentZapis.at(currentZapis.size() - 1) == "0"){
            this->greska = true;
        }
        int uvecaj = 5;
        if (line.at(2) == "r0" || line.at(2) == "r1" || line.at(2) == "r2" || line.at(2) == "r3" || line.at(2) == "r4" || line.at(2) == "r5" || line.at(2) == "r6" || line.at(2) == "r7" || line.at(2) == "sp" || line.at(2) == "pc" || line.at(2) == "psw" ||
            line.at(2) == "[r0]" || line.at(2) == "[r1]" || line.at(2) == "[r2]" || line.at(2) == "[r3]" || line.at(2) == "[r4]" || line.at(2) == "[r5]" || line.at(2) == "[r6]" || line.at(2) == "[r7]" || line.at(2) == "[sp]" || line.at(2) == "[pc]" || line.at(2) == "[psw]")
        {
            uvecaj = 3;
        }
        locationCounter += uvecaj;
    }
}

// D E O    Z A    P A R S I R A N J E
//=========================================================

vector<vector<string>> Asembler::parseInput(string filename)
{
    ifstream input;
    vector<vector<string>> parsedInput;
    parsedInput.clear();

    input.open(filename);

    if (!input.is_open())
    {
        cout << "Greska!";
        return parsedInput;
    }
    string line;
    int i = 0;

    while (getline(input, line))
    {
        vector<string> isparsiranaLinija = isparsirajLiniju(line);
        if (!isparsiranaLinija.empty())
        {
            parsedInput.push_back(isparsiranaLinija);
        }
    }

    if (this->greska)
    {
        cout << "GRESKA PRI PARSIRANJU!!!!" << endl;
    }

    input.close();

    return parsedInput;
}

vector<string> Asembler::isparsirajLiniju(string line)
{
    vector<string> parsiranaLinija;
    vector<char> parsiranaRec;
    bool komentar = false;
    char znak;
    this->line++;
    parsiranaLinija.clear();
    if (line.empty())
        return parsiranaLinija;
    for (int i = 0; i < line.length() + 1; i++)
    {
        if (!isblank(line[i]) && line[i] != '\0' && line[i] != '\n' && line[i] != ',' && komentar == false && line[i] != '\r')
        {
            if (line[i] == '#')
            {
                komentar = true;
            }
            else
            {
                if ((parsiranaRec.size() == 0 && (isalpha(line[i]) || isdigit(line[i]) || line[i] == '_' || line[i] == '.' ||
                                                  line[i] == '[' || line[i] == '$' || line[i] == '%' || line[i] == '*' || line[i] == '+' || line[i] == '-' || line[i] ==':')) ||
                    parsiranaRec.size() > 0)
                {

                    parsiranaRec.push_back(line.at(i));
                }
                else
                {
                    cout << "Greska na liniji " << this->line << endl;
                    this->greska = true;
                    parsiranaLinija.clear();
                    return parsiranaLinija;
                }
            }
        }
        else
        {
            if (znak == line[i] && line[i] == ',')
            {
                cout << "Greska na liniju " << this->line << endl;
                this->greska = true;
                parsiranaLinija.clear();
                return parsiranaLinija;
            }
            else if (parsiranaRec.size() > 0)
            {
                parsiranaLinija.push_back(string(parsiranaRec.begin(), parsiranaRec.end()));
                parsiranaRec.clear();
            }
        }
        if (!isblank(line[i]))
            znak = line[i];
    }

    return parsiranaLinija;
}
