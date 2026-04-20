#include <iostream>
#include <iostream>
#include <list>
#include <string>
#include <functional>
using namespace std;
void healCommand(Entity& target,const list<string>& args) {
    if (args.empty()) {
        cerr<<"Error: El comando 'heal' requiere un argumento (cantidad)"<<endl;return;
    }
    try {
        int amount=stoi(args.front());
        if (amount<0) {
            cerr << "Error: La cantidad de curación no puede ser negativa." <<endl;
            return;
        }
        target.heal(amount);
        cout << "Comando ejecutado: heal " << amount << endl;
    }catch (const invalid_argument&e){cerr<<"Error: El argumento debe ser un número entero válido."<<endl;}
}

int main(){return 0;}