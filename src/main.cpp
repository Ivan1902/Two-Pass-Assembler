#include <iostream>
#include <fstream>
#include "implementacija.h"
using namespace std;

int main(int argc, char* argv[]){
    // pokretanje:
    // g++ -o asembler src/*.cpp -Iinc 
    // ./asembler -o tests/odbrana.txt tests/ulaz1.txt
    Asembler* asembler = new Asembler();

    string izlaz = argv[2];
    string ulaz = argv[3];

    ofstream output(izlaz);

    //asembler->parsedInput = asembler->parseInput("ulaz.txt");
    asembler->parsedInput = asembler->parseInput(ulaz);

    for (int i = 0; i < asembler->parsedInput.size(); i++)
    {
        for(int j = 0; j < asembler->parsedInput.at(i).size(); j++){
            cout << "\t"<< asembler->parsedInput.at(i).at(j) << "\t";
        }
        cout<<endl;
    }
    cout << "=========================================="<<endl;

    if(asembler->greska) {
        return 0;
    }

    asembler->firstPass(asembler->parsedInput, output);

    if(asembler->greska) {
        return 0;
    }

    cout<<"############################################"<<endl;

  
    asembler->secondPass(asembler->parsedInput, output); 
   

    return 0;

}