#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include<thread>
#include <cstdlib>

#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")


#define NUM_PLAYERS 7

const std::string names[NUM_PLAYERS] = {
    "Big Chungusia",
    "Mother Russia",
    "I'm Already Dead",
    "The Republic Of Kwyjbo",
    "Bodylande",
    "Controller Player",
    "USA"
};

//These are the model keys
const std::string modelURLs[NUM_PLAYERS] = {
    "099eb620-e63a-11eb-b1e6-81801e9cc28716f1e362-5547-418f-aca8-a557e28c26d5",
    "d0745ab0-e59b-11eb-a606-07a50d4195b5d4fcef4e-10a7-4717-bdd6-555f1fefe761", 
    "60173f80-e59b-11eb-9097-dfae9bafaa077c8a297a-98eb-4a8f-8cd5-16b54df4ebbe",
    "d214f5e0-e59c-11eb-a606-07a50d4195b5cd7c1aeb-be14-4821-8bb3-c8e50729341f", 
    "5f37dbc0-e63a-11eb-ad32-271ee0d9500250a6a18a-1ff8-4316-adc6-820e16e3025e", 
    "d5fabb90-e59c-11eb-a606-07a50d4195b57ab48cc3-6c5d-4d1c-9132-4aee273489df",
    "00557980-e63c-11eb-b1e6-81801e9cc2873e41eb67-29d4-4e2f-91cf-ef17adc605d7" 
};

const unsigned short stats[NUM_PLAYERS][3] =
{
    {40, 35, 25},
    {40, 40, 20},
    {25, 50, 25},
    {40, 30, 30},
    {35, 35, 30},
    {47, 37, 16},
    {50, 25, 25}
};

const std::string actions[4] =
{
    "attacking",
    "defending",
    "advancing",
    "retreating"
};

#define MAX_HP 50.0
#define MAX_SHIELD 50.0

#define INITIAL_LAND 100

struct country
{
    std::string name;
    std::string modelURL = "NONE";
    int land = INITIAL_LAND;
    unsigned short attackPower, defensePower, spyPower;
    int hp = MAX_HP;
    int shield = MAX_SHIELD;

    country(std::string name, std::string modelURL, unsigned short attackPower, unsigned short defensePower, unsigned short spyPower) :
        name(name), modelURL(modelURL), attackPower(attackPower), defensePower(defensePower), spyPower(spyPower) {}

    std::string getVars()
    {
        std::string opt = std::to_string(attackPower) + " ";
        opt += std::to_string(defensePower) + " ";
        opt += std::to_string(spyPower) + " ";
        opt += std::to_string(hp) + " ";
        opt += std::to_string(shield);

        return opt;
    }
};

std::vector<country*> players;
std::vector<std::pair<struct sockaddr_in, SOCKET>*> viewers;
bool running = true;

void clamp(int* val, int min, int max)
{
    if (*val > max)
    {
        *val = max;
    }
    else if (*val < min)
    {
        *val = min;
    }
}

SOCKET createSocket()
{
    SOCKET s;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    return s;
}

struct sockaddr_in createServer(unsigned short port = 1337)
{
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    return server;
}

void bindServer(struct sockaddr_in server, SOCKET socket)
{
    if (bind(socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
    }
}

std::pair<struct sockaddr_in, SOCKET>* acceptConnection(SOCKET socket)
{
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
    SOCKET clientSocket = accept(socket, (struct sockaddr*)&client, &c);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error code : %d", WSAGetLastError());
        return nullptr;
    }

    std::pair<struct sockaddr_in, SOCKET>* opt = new std::pair<struct sockaddr_in, SOCKET>(client, clientSocket);

    return opt;
}

void sendMessage(std::string message)
{
    bool needsClearing = false;

    for (std::pair<struct sockaddr_in, SOCKET>* viewer: viewers)
    {
        send(viewer->second, message.c_str(), message.length(), 0);
        int err = WSAGetLastError();
        if (WSAGetLastError() == 10054)
        {
            std::cout << "Viewer Disconnected " << err << '\n';
            closesocket(viewer->second);
            delete viewer;
            needsClearing = true;
        }

    }

    if (needsClearing)
    {
        for (unsigned int i = 0; i < viewers.size(); i++)
        {
            if (viewers[i]->second == 15987178197214944733)
            {
                viewers[i] = viewers.back();
                viewers.pop_back();
            }
        }
    }
}

void serverLoop(SOCKET hostSocket)
{
    while (running)
    {
        std::pair<struct sockaddr_in, SOCKET>* client = acceptConnection(hostSocket);
        if (client != nullptr) viewers.push_back(client);
    }
}

//return of 1 is attack, 2 is defend, 3 is advance, 4 is retreat, anything else is bad
int getMove(country* attacker, country* target)
{
    std::string cmd = "python3 runModel.py " + attacker->modelURL + " " + target->getVars();
    return system(cmd.c_str());
}

void calcBattleVars(country* player, int action, int* atk, int* def, double* playerLandTake, double* otherPlayerLandTake)
{
    switch (action)
    {
    case 1:
        *atk = (player->attackPower + player->spyPower) * 0.75;
        *def = player->defensePower * 0.125;
        break;
    case 2:
        *def = (player->defensePower + player->spyPower) * 0.25;
        *atk = player->spyPower * 1.0;
        break;
    case 3:
        *atk = player->attackPower * 1.75;
        *playerLandTake += 0.1;
        break;
    case 4:
        *def = player->defensePower * 0.75;
        *atk = player->spyPower * 0.35;
        *otherPlayerLandTake += 0.1;
        break;
    }
}

void doBattle(country* player1, country* player2)
{
    std::string msg;
    if (player1->land <= 0 || player2->land <= 0)
        return;

    int init1land = player1->land;
    int init2land = player2->land;
    while ((player1->hp > 0 && player1->land > 0) && (player2->hp > 0 && player2->land > 0))
    {
        int player1Move = getMove(player1, player2);
        int player2Move = getMove(player2, player1);

        msg = player1->name + " is " + actions[player1Move - 1] + " and " + player2->name + " is " + actions[player2Move - 1] + '.';
        sendMessage(msg);
        
        Sleep(2000);

        int p1Damage = 0;
        int p1Resist = 0;

        double p1LandTake = 0; 
        double p2LandTake = 0;

        calcBattleVars(player1, player1Move, &p1Damage, &p1Resist, &p1LandTake, &p2LandTake);

        int p2Damage = 0;
        int p2Resist = 0;

        calcBattleVars(player2, player2Move, &p2Damage, &p2Resist, &p2LandTake, &p1LandTake);

        int dmgToP2 = p1Damage - p2Resist;
        int dmgToP1 = p2Damage - p1Resist;

        clamp(&dmgToP2, 0, MAX_HP + MAX_SHIELD);
        clamp(&dmgToP1, 0, MAX_HP + MAX_SHIELD);

        player1->shield -= dmgToP1;

        if (player1->shield < 0)
        {
            player1->hp += player1->shield;
        }

        player2->shield -= dmgToP2;

        if (player2->shield < 0)
        {
            player2->hp += player2->shield;
        }

        clamp(&player1->shield, 0, MAX_SHIELD);
        clamp(&player2->shield, 0, MAX_SHIELD);

        msg = player1->name + " now has " + std::to_string(player1->hp) + " hp and " + std::to_string(player1->shield) + " shield. ";
        sendMessage(msg);
        Sleep(500);
        msg = player2->name + " now has " + std::to_string(player2->hp) + " hp and " + std::to_string(player2->shield) + " shield.";
        sendMessage(msg);
        Sleep(1500);

        int p1gain = p1LandTake * player2->land;
        int p2gain = p2LandTake * player1->land;

        player1->land += p1gain - p2gain;
        player2->land += p2gain - p1gain;

        std::cout << "p1: " << player1Move << ", " << player1->hp << ", " << player1->shield << " p2: " << player2Move << ", " << player2->hp << ", " << player2->shield << '\n';
    }

    clamp(&player1->hp, 0, MAX_HP);
    clamp(&player2->hp, 0, MAX_HP);
    int p1Take = player2->land * (double(player1->hp) / MAX_HP);
    int p2Take = player1->land * (double(player2->hp) / MAX_HP);

    player1->land += p1Take - p2Take;
    player2->land += p2Take - p1Take;

    int dp1 = player1->land - init1land;
    int dp2 = player2->land - init2land;

    std::cout << player1->land << "  " << player2->land << '\n';

    Sleep(1000);
    if (dp1 > dp2)
    {
        std::string msg = player1->name + " has won the battle and gained " + std::to_string(dp1) + " land!";
        sendMessage(msg);
        std::cout << msg << '\n';
    }
    else if (dp1 < dp2)
    {
        std::string msg = player2->name + " has won the battle and gained " + std::to_string(dp2) + " land!";
        sendMessage(msg);
        std::cout << msg << '\n';
    }
    else
    {
        std::string msg = player1->name + " and " + player2->name + " have tied! No land was exchanged!";
        sendMessage(msg);
        std::cout << msg << '\n';
    }
    Sleep(5000);

    player1->hp = MAX_HP;
    player1->shield = MAX_SHIELD;
    
    player2->hp = MAX_HP;
    player2->shield = MAX_SHIELD;
}

unsigned int playersAlive()
{
    unsigned int opt = 0;

    for (country* player : players)
    {
        if (player->land > 0)
        {
            opt++;
        }
    }

    return opt;
}

int main()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return -1;
    }

    SOCKET hostSocket = createSocket();
    struct sockaddr_in server = createServer();
    bindServer(server, hostSocket);
    listen(hostSocket, 3);

    std::thread server_thread(serverLoop, hostSocket);

    for (unsigned int i = 0; i < NUM_PLAYERS; i++)
    {
        country* player = new country(names[i], modelURLs[i], stats[i][0], stats[i][1], stats[i][2]);
        players.push_back(player);
    }

    std::cout << "Starting in 1 minute\n";
    Sleep(30000);
    std::cout << "Starting in 30 seconds\n";
    Sleep(20000);
    std::cout << "Starting in 10 seconds\n";
    Sleep(10000);

    srand(time(NULL));
    while (playersAlive() > 1)
    {
        unsigned int pid1 = rand() % players.size();
        unsigned int pid2 = rand() % players.size();

        if (pid1 != pid2)
        {
            if (players[pid1]->land > 0 && players[pid2]->land > 0)
            {
                sendMessage("         ");
                Sleep(500);
                std::string msg = players[pid1]->name + " is starting combat with " + players[pid2]->name + "!";
                sendMessage(msg);
                std::cout << msg << '\n';
                Sleep(500);
                msg = players[pid1]->name + " has " + std::to_string(players[pid1]->land) + " land and " + players[pid2]->name + " has " + std::to_string(players[pid2]->land) + " land.";
                sendMessage(msg);
                doBattle(players[pid1], players[pid2]);
            }
        }
    }

    for (country* player : players)
    {
        if (player->land > 0)
        {
            Sleep(1500);
            std::string msg = player->name + " has won by taking over the world!";
            sendMessage(msg);
        }
        delete player;
    }

    running = false;
    
    for (std::pair<struct sockaddr_in, SOCKET>* viewer : viewers)
    {
        closesocket(viewer->second);
        delete viewer;
    }

    closesocket(hostSocket);
    WSACleanup();

    server_thread.join();
    return 0;
}