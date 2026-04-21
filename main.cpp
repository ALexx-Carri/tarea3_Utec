#include <iostream>
#include <list>
#include <string>
#include <functional>
#include <map>
#include <fstream>

using namespace std;

map<string, string> cargarEnv(const string& ruta) {
    map<string, string> env;
    ifstream archivo(ruta);
    string linea;

    while (getline(archivo, linea)) {
        size_t pos = linea.find('=');
        if (pos != string::npos) {
            string clave = linea.substr(0, pos);
            string valor = linea.substr(pos + 1);
            env[clave] = valor;
        }
    }
    return env;
}

class Entity {
private:
    string nombre;
    int vida, vida_maxima;
    float posx, posy;

public:
    Entity(string n="Jugador", int v=100)
        : nombre(n), vida(v), vida_maxima(v), posx(0), posy(0) {}

    int getVida() const { return vida; }

    void heal(int cantidad) {
        vida += cantidad;
        if (vida > vida_maxima) vida = vida_maxima;
    }

    void damage(int cantidad) {
        vida -= cantidad;
        if (vida < 0) vida = 0;
    }

    void move(float x, float y) {
        posx += x;
        posy += y;
    }

    void reset() {
        vida = vida_maxima;
        posx = posy = 0;
    }

    string statusString() const {
        return "Vida=" + to_string(vida) +
               " Pos=(" + to_string(posx) + "," + to_string(posy) + ")";
    }

    void printStatus() const {
        cout << statusString() << endl;
    }
};

using Command = function<void(const list<string>&)>;

class DamageFunctor {
private:
    Entity& entity;
    int contador;
public:
    DamageFunctor(Entity& e) : entity(e), contador(0) {}

    void operator()(const list<string>& args) {
        if (args.empty()) {
            cerr << "Error: damage requiere argumento\n";
            return;
        }
        int val = stoi(args.front());
        entity.damage(val);
        contador++;
        cout << "Damage aplicado. Veces usado: " << contador << endl;
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

    void execute(const string& name, const list<string>& args) {
        map<string, Command>::iterator it = commands.find(name);

        if (it != commands.end()) {
            string before = entity.statusString();
            it->second(args);
            string after = entity.statusString();

            history.push_back(name + " | " + before + " -> " + after);
        } else {
            cerr << "Error: comando no encontrado: " << name << endl;
        }
    }

    void removeCommand(const string& name) {
        map<string, Command>::iterator it = commands.find(name);
        if (it != commands.end()) {
            commands.erase(it);
            cout << "Comando eliminado: " << name << endl;
        } else {
            cerr << "No existe el comando\n";
        }
    }

    void showHistory() {
        list<string>::iterator it;
        for (it = history.begin(); it != history.end(); ++it) {
            cout << *it << endl;
        }
    }

    void registerMacro(const string& name,
        const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it = macros.find(name);

        if (it == macros.end()) {
            cerr << "Macro no encontrada\n";
            return;
        }

        list<pair<string, list<string>>>::const_iterator step;

        for (step = it->second.begin(); step != it->second.end(); ++step) {
            if (commands.find(step->first) == commands.end()) {
                cerr << "Error en macro, comando inexistente: " << step->first << endl;
                return;
            }
            execute(step->first, step->second);
        }
    }
};

void healCommand(Entity& target, const list<string>& args) {
    if (args.empty()) {
        cerr << "heal requiere argumento\n";
        return;
    }
    int val = stoi(args.front());
    target.heal(val);
}

int main() {
    auto datos = cargarEnv(".env");
    string nombre = datos["USER_NAME"];

    Entity jugador(nombre, 100);
    CommandCenter center(jugador);

    center.registerCommand("heal", [&jugador](const list<string>& args) {
        healCommand(jugador, args);
    });


    center.registerCommand("move", [&jugador](const list<string>& args) {
        if (args.size() < 2) return;
        auto it = args.begin();
        float x = stof(*it++);
        float y = stof(*it);
        jugador.move(x, y);
    });


    DamageFunctor dmg(jugador);
    center.registerCommand("damage", dmg);


    center.registerCommand("status", [&jugador](const list<string>&) {
        jugador.printStatus();
    });

    center.registerCommand("reset", [&jugador](const list<string>&) {
        jugador.reset();
    });

    list<string> args;

    args = {"10"};
    center.execute("heal", args);

    args = {"5"};
    center.execute("damage", args);

    args = {"3", "4"};
    center.execute("move", args);

    args.clear();
    center.execute("status", args);

    center.registerMacro("combo1", {
        {"heal", {"10"}},
        {"move", {"1", "1"}},
        {"status", {}}
    });

    center.registerMacro("combo2", {
        {"damage", {"20"}},
        {"status", {}}
    });

    center.registerMacro("combo3", {
        {"reset", {}},
        {"status", {}}
    });
    cout << "\n--- Ejecutando Macro combo1 ---\n";
    center.executeMacro("combo1");

    cout << "\n--- Ejecutando Macro combo2 ---\n";
    center.executeMacro("combo2");

    cout << "\n--- Ejecutando Macro combo3 ---\n";
    center.executeMacro("combo3");
    center.removeCommand("heal");
    center.execute("heal", {"10"}); 
    cout << "\n--- HISTORIAL ---\n";
    center.showHistory();

    return 0;
}
