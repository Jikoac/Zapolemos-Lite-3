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
#include <filesystem>

using namespace std;

string StringID(UINT id) {
    char buffer[256];
    if (LoadString(GetModuleHandle(NULL), id, buffer, sizeof(buffer))) {
        return string(buffer);
    }
    return "[String not found]";
}

// not \xCE\xB2
const string defaultName = StringID(106);
const string version = StringID(101)+StringID(1);
int level;
int seed;
bool counter_enabled = true;
string player_data;
string enemy_data;
string enemyName = StringID(107);
bool choose_characters = false;

int randomize (int max = 10) {
    srand(seed);
    int value = rand() % max;
    seed += rand();
    return value;
}

void changeSettings(string settings) {
    nlohmann::json json = nlohmann::json::parse(settings);
    counter_enabled = json.value("counter_attack",true);
    choose_characters = json.value("select_characters", false);
}

string removeFrom(string& str, const string& toRemove) {
    size_t pos;
    while ((pos = str.find(toRemove)) != string::npos) {
        str.erase(pos, toRemove.length());
    }
    return str;
}

bool loadHero(string fileName, string & output) {
    filesystem::path filePathSave(StringID(201)+ fileName + ".zapolemos3");
    filesystem::path filePathSource(StringID(202)+ fileName + ".zapolemos3");
    filesystem::path filePathDirect(fileName);
    if (filesystem::exists(filePathSave)) {
        fileName = StringID(201) + fileName + ".zapolemos3";
    }
    else if (filesystem::exists(filePathSource)) {
        fileName = StringID(202) + fileName + ".zapolemos3";
    }
    else if (!filesystem::exists(filePathDirect)) {
        return false;
    }
    std::ifstream file(fileName);
    std::stringstream buffer;
    buffer << file.rdbuf();
    output = buffer.str();
    return true;
}

bool readArg (string arg) {
        if ((arg.rfind("C:/", 0) == 0) or (arg.rfind("C:\\", 0) == 0)) {
            std::ifstream file(arg);
            std::stringstream buffer;
            buffer << file.rdbuf();
            player_data = buffer.str();
        }
        else if (arg.rfind("player=", 0) == 0) {
            arg = removeFrom(arg, "player=");
            return loadHero(arg, player_data);
        }
        else if (arg.rfind("enemy=", 0) == 0) {
            arg = removeFrom(arg, "enemy=");
            return loadHero(arg, enemy_data);
        }
        else if (arg == "counter=false") {
            counter_enabled = false;
        }
        else if (arg.rfind("settings=",0) == 0) {
            arg = removeFrom(arg, "settings=");
            if (arg.rfind("C:",0) == 0) {
                std::ifstream file(arg);
                std::stringstream buffer;
                buffer << file.rdbuf();
                changeSettings(buffer.str());
            } else {
                changeSettings(arg);
            }
        }
        return true;
}

class player {
    public:
    string name;
    bool is_AI, was_attacked;
    int health, damage, maxHealth, skill, defense, recovery, healing;
    float counter_multiplier;

    player(const string& jsonData) {
        was_attacked = false;
        nlohmann::json json = nlohmann::json::parse(jsonData);
        name = json.value("name", defaultName);
        is_AI = json.value("is_AI", false);
        health = maxHealth = json.value("health", 10);
        damage = json.value("damage", 2);
        skill = json.value("skill", 5);
        defense = json.value("defense", 0);
        recovery = json.value("recovery", 3);
        healing = json.value("healing", 4);
        counter_multiplier = json.value("counter", 1.5);
    }
    void display () {
        if (was_attacked and counter_enabled) {
            cout << name << endl << health << "/" << maxHealth << endl << round(damage*counter_multiplier) << " +" << (counter_multiplier-1)*100 << "%" << endl;
        } else {
            cout << name << endl << health << "/" << maxHealth << endl << damage << endl;
        }
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
            success = heal(target);
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
            if (was_attacked and counter_enabled) {
                target.health -= round(damage * counter_multiplier);
            } else{
                target.health -= damage;
            }
            target.was_attacked = true;
            return true;
        }
        return false;
    }
    bool heal (player & target) {
        target.was_attacked = false;
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
                heal(target);
            }
            else {
                attack(target);
            }
        }
    }
};

void start(string &name, int &level) {
    if (choose_characters) {
        string heroName;

        cout << "Player: ";
        getline (cin, heroName);
        loadHero(heroName, player_data);

        cout << "Enemy: ";
        getline (cin, heroName);
        loadHero(heroName, enemy_data);
    }
    if (player_data.empty()) {
        cout << StringID(102);
        getline ( cin, name );
        name = (name != "") ? name : defaultName;
        cout << StringID(109) << name << "!" << endl;
        player_data = "{\"name\":\""+name+"\"}";
    }
    cout << StringID(108)+" (0-10): ";
    cin >> level;
    level = max(0,min(level,10));
}

int main (int argc, char* argv[]) {
    seed = atoi(ctime(0));
    for (int argno = 1; argno<argc; ++argno) {
        readArg(argv[argno]);
    }
    if (enemy_data.empty()) {
        enemy_data = "{\"is_AI\":true,\"name\":\""+enemyName+"\"}";
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
        cout << StringID(103);
        cin >> input;
        User.act(Enemy, input);
        Enemy.act(User);
        Enemy.display();
    }
    bool victory = (Enemy.health<User.health);
    if(victory) {
        cout <<StringID(104);
    }
    else {
        cout << StringID(105);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
