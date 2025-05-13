#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <utility>
#include <thread>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

string StringID(UINT id) {
    char buffer[256];
    if (LoadString(GetModuleHandle(NULL), id, buffer, sizeof(buffer))) {
        return string(buffer);
    }
    return "[String not found]";
}

// not \xCE\xB2
const string defaultName = "User";
const string version = StringID(101)+StringID(1);
int level;
int seed;
string player_data;
string enemy_data;

int randomize (int max = 10) {
    srand(seed);
    int value = rand() % max;
    seed += rand();
    return value;
}


string removeFrom(string& str, const string& toRemove) {
    size_t pos;
    while ((pos = str.find(toRemove)) != string::npos) {
        str.erase(pos, toRemove.length());
    }
    return str;
}

void readArg (string arg) {
        if ((arg.rfind("C:/", 0) == 0) or (arg.rfind("C:\\", 0) == 0)) {
            std::ifstream file(arg);
            std::stringstream buffer;
            buffer << file.rdbuf();
            player_data = buffer.str();
        }
        else if (arg.rfind("player=", 0) == 0) {
            arg = StringID(201) + removeFrom(arg, "player=") + ".zapolemos3";
            std::ifstream file(arg);
            std::stringstream buffer;
            buffer << file.rdbuf();
            player_data = buffer.str();
        }
        else if (arg.rfind("enemy=", 0) == 0) {
            arg = StringID(201) + removeFrom(arg, "enemy=") + ".zapolemos3";
            std::ifstream file(arg);
            std::stringstream buffer;
            buffer << file.rdbuf();
            enemy_data = buffer.str();
        }
}

class player {
    public:
    string name;
    bool is_AI;
    int health, damage, maxHealth, skill, defense, recovery, healing;

    player(const string& jsonData) {
        nlohmann::json json = nlohmann::json::parse(jsonData);
        name = json.value("name", defaultName);
        is_AI = json.value("is_AI", false);
        health = maxHealth = json.value("health", 10);
        damage = json.value("damage", 2);
        skill = json.value("skill", 5);
        defense = json.value("defense", 0);
        recovery = json.value("recovery", 3);
        healing = json.value("healing", 4);
    }
    void display () {
        cout << name << endl << health << "/" << maxHealth << endl << damage << endl;
    }
    pair<string, bool> ai (player & target) {
        bool success;
        string action;

        float targetHealth = target.health;
        float myHealth = health;
        float targetHP = targetHealth/target.maxHealth;
        float ownHP = myHealth / maxHealth;
        float chance = 5 + round (ownHP - targetHP * level * 0.5);
        if (randomize()<=chance) {
            success = heal();
            action = "heal";
        }
        else {
            success = attack(target);
            action = "attack";
        }
        return make_pair(action, success);
    }
    bool attack (player & target) {
        int chance = skill - target.defense;
        if (randomize() <= chance) {
            target.health -= damage;
            return true;
        }
        return false;
    }
    bool heal () {
        if (randomize() <= recovery) {
            health = min (health + healing, maxHealth);
            return true;
        }
        return false;
    }
    void act(player & target, int input = 1) {
        if (is_AI) {
            ai(target);
        }
        else {
            if (input == 2) {
                heal();
            }
            else {
                attack(target);
            }
        }
    }
};

void start(string &name, int &level) {
    if (player_data.empty()) {
        cout << StringID(102);
        getline ( cin, name );
        name = (name != "") ? name : defaultName;
        cout << "Hello, " << name << "!" << endl;
        player_data = "{\"name\":\""+name+"\"}";
    }
    cout << "Level (0-10): ";
    cin >> level;
    level = max(0,min(level,10));
}

int main (int argc, char* argv[]) {
    seed = atoi(ctime(0));
    for (int argno = 1; argno<argc; ++argno) {
        readArg(argv[argno]);
    }
    if (enemy_data.empty()) {
        enemy_data = "{\"is_AI\":true,\"name\":\"Enemy\"}";
    }
    cout << StringID(2) << endl;
    cout << version << endl;
    string name;
    start (name, level);
    player User(player_data);
    player Enemy(enemy_data);
    int input;
    while ((User.health>0) and (Enemy.health>0)) {
        User.display();
        cout << "1: Attack\n2: Heal\nInput: ";
        cin >> input;
        User.act(Enemy, input);
        Enemy.act(User);
        Enemy.display();
    }
    bool victory = (Enemy.health<User.health);
    if(victory) {
        cout <<"\nYou win!";
    }
    else {
        cout << "\nYou lose!";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
