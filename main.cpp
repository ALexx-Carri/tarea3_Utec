#include <iostream>
#include <iostream>
#include <list>
#include <string>
#include <functional>
#include<map>
#include <fstream>
using namespace std;

map<string, string> cargarEnv(const string& ruta) {
    map<string, string> env;
    ifstream archivo(ruta);
    string linea;

    while (std::getline(archivo, linea)) {
        size_t pos = linea.find('=');
        if (pos != std::string::npos) {
            std::string clave = linea.substr(0, pos);
            std::string valor = linea.substr(pos + 1);
            env[clave] = valor;
        }
    }
    return env;
}

class Entity {
private:string nombre;
    int vida;int vida_maxima;int energia /* no se si usar energia(tal vez relleno) */;
    float posx,posy;
public:
    Entity(string n="Jugador",int v=100):nombre(n),vida(v),vida_maxima(v),posx(0),posy(0){};
    void heal(int cantidad) {
        int anterior =vida;
        vida+=cantidad;
        if (vida>vida_maxima)vida=vida_maxima;
        cout<<"["<<nombre<<"]"<<" Vida antes: "<<anterior<<"-> Despues: "<<vida<<endl;
    }
    void damage(int cantidad) {
        int anterior=vida;
        vida-=cantidad;
        if (vida<0)vida=0;
        cout<<"["<<nombre<<"]"<<" Vida antes: "<<anterior<<"-> Despues: "<<vida<<endl;
    }
    void move(float x,float y) {
        posx+=x;
        posy+=y;
        cout<<"["<<nombre<<"] "<<"Nueva posición: ("<<posx<<", "<<posy<<")"<<endl;
    }
    void printStatus()const {
        cout << "--- Status de " << nombre << " ---" << std::endl;
        cout << "Vida: " << vida << "/" << vida_maxima << std::endl;
        cout << "Posición: (" << posx << ", " << posy << ")" << std::endl;
        cout << "------------------------" << std::endl;
    }
};
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
using Command=function<void(const std::list<std::string>&)>;
class CommandCenter {
private: Entity& entity;
    map<string,Command>commands;
    list<string> history;

public:
    CommandCenter(Entity& e):entity(e){}
    void registerCommand(const string&name,Command cmd)
    {commands[name]=cmd;};
    void execute(const string&name,const list<string>&args) {
        auto it=commands.find(name);
        if (it!=commands.end()) {
            it->second(args); history.push_back("Ejucutado: "
                +name);
        }else{cerr<<"Error: Commando "<<name<<"no encontrado."<<endl;}
    };
    void removeCommand(const string& name);

    void registerMacro(const string&name,const list<pair<string,list<string>>>& steps);
    void executeMacro(const string&name);
};
int main() {
    auto misDatos = cargarEnv(".env");
    string nombreFinal;
    map<string, string>::iterator it = misDatos.find("USER_NAME");
    if (it == misDatos.end()) {
        cerr << "CRITICO: No se pudo cargar el nombre del usuario desde .env" << endl;
        nombreFinal = "Prota Generico";
    } else {
        cout << "Configuracion cargada para: " << it->second << endl;
        nombreFinal = it->second;
    }
    Entity jugador(nombreFinal, 100);
    CommandCenter center(jugador);
    center.registerCommand("heal", [&jugador](const list<string>& args) {
        healCommand(jugador, args);
    });
    return 0;
}