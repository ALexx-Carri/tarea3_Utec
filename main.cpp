#include <iostream>
#include <list>
#include <string>
#include <functional>
#include <map>
#include <fstream>
#include <algorithm>
using namespace std;
using Command = function<void(const std::list<std::string>&)>;
class Entity {
private:
    string nombre;
    int vida;
    int vida_maxima;
    float posx, posy;
public:
    Entity(string n = "Jugador", int v = 100) : nombre(n), vida(v), vida_maxima(v), posx(0), posy(0) {}
    void heal(int cantidad) {
        vida = min(vida + cantidad, vida_maxima);
    }
    void damage(int cantidad) {
        vida = max(vida - cantidad, 0);
    }
    void move(float x, float y) {
        posx += x;
        posy += y;
    }
    int getVida() const { return vida; }
    string getNombre() const { return nombre; }
    void printStatus() const {
        cout << "--- Status de " << nombre << " ---" << endl;
        cout << "Vida: " << vida << "/" << vida_maxima << endl;
        cout << "Posicion: (" << posx << ", " << posy << ")" << endl;
        cout << "------------------------" << endl;
    }
};
void healCommand(Entity& target, const list<string>& args) {
    if (args.empty()) return;
    try {
        int amount = stoi(args.front());
        target.heal(amount);
    } catch (...) { cerr << "Error en heal." << endl; }
}
class DamageFunctor {
private:
    Entity& entity;
    int contadorUso = 0;
public:
    DamageFunctor(Entity& e) : entity(e) {}
    void operator()(const list<string>& args) {
        if (args.empty()) return;
        try {
            int d = stoi(args.front());
            entity.damage(d);
            contadorUso++;
            cout << "[Functor Log] Daño aplicado. Usos totales: " << contadorUso << endl;
        } catch (...) { cerr << "Error en damage." << endl; }
    }
};
class CommandCenter {
private:
    Entity& entity;
    map<string, Command> commands;
    list<string> history;
    map<string, list<pair<string, list<string>>>> macros;
public:
    CommandCenter(Entity& e) : entity(e) {}
    void registerCommand(const string& name, Command cmd) {
        commands[name] = cmd;
    }
    void removeCommand(const string& name) {
        map<string, Command>::iterator it = commands.find(name);
        if (it != commands.end()) {
            commands.erase(it);
            cout << "Comando '" << name << "' eliminado." << endl;
        }
    }
    void execute(const string& name, const list<string>& args) {
        map<string, Command>::iterator it = commands.find(name);
        if (it != commands.end()) {
            int antes = entity.getVida();
            it->second(args);
            int despues = entity.getVida();
            string log = "Cmd: " + name + " | Vida: " + to_string(antes) + " -> " + to_string(despues);
            history.push_back(log);
        } else {
            cerr << "Error: Comando '" << name << "' no encontrado." << endl;
        }
    }
    void registerMacro(const string& name, const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }
    void executeMacro(const string& name) {
        auto itM = macros.find(name);
        if (itM == macros.end()) return;
        cout << ">>> Ejecutando Macro: " << name << endl;
        list<pair<string, list<string>>>::iterator itStep;
        for (itStep = itM->second.begin(); itStep != itM->second.end(); ++itStep) {
            this->execute(itStep->first, itStep->second);
        }
    }
    void imprimirHistorial() {
        cout << "\n--- HISTORIAL DE EJECUCION ---" << endl;
        list<string>::iterator itH;
        for (itH = history.begin(); itH != history.end(); ++itH) {
            cout << *itH << endl;
        }
    }
};
map<string, string> cargarEnv(const string& ruta) {
    map<string, string> env;
    ifstream archivo(ruta);
    string linea;
    while (getline(archivo, linea)) {
        size_t pos = linea.find('=');
        if (pos != string::npos) {
            env[linea.substr(0, pos)] = linea.substr(pos + 1);
        }
    }
    return env;
}
int main() {auto misDatos = cargarEnv(".env");
    string miNombre;
    map<string, string>::iterator it = misDatos.find("USER_NAME");
    if (it != misDatos.end() && !it->second.empty()) {
        miNombre = it->second;
        cout << "Configuracion cargada exitosamente." << endl;
    } else {
        miNombre = "Prota Generico";
        cerr << "ADVERTENCIA: .env no encontrado o USER_NAME ausente." << endl;
    }
    Entity jugador(miNombre, 100);
    CommandCenter center(jugador);
    center.registerCommand("heal", [&jugador](const list<string>& a) { healCommand(jugador, a); });
    center.registerCommand("move", [&jugador](const list<string>& args) {
        if (args.size() < 2) return;
        jugador.move(stof(args.front()), stof(*(++args.begin())));
    });
    center.registerCommand("damage", DamageFunctor(jugador));
    center.registerCommand("status", [&jugador](const list<string>&) { jugador.printStatus(); });
    center.registerMacro("recovery", {{"heal", {"20"}}, {"status", {}}});
    center.registerMacro("skirmish", {{"move", {"2", "2"}}, {"damage", {"10"}}});
    center.registerMacro("full_check", {{"status", {}}, {"move", {"0", "0"}}, {"status", {}}});
    center.execute("status", {});
    center.execute("move", {"5", "10"});
    center.execute("damage", {"25"});
    center.executeMacro("skirmish");
    center.executeMacro("recovery");
    center.execute("comando_inexistente", {});
    center.imprimirHistorial();

    return 0;
}