#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "viewer_iface.h"
#include <iostream>
using namespace std;
int main(){
    int playerFloor=-1;
    int playerRoom=-1;
    int energy=INITIAL_ENERGY;
    int time=INITIAL_TIME;
    int score=INITIAL_SCORE;
    char inventory[MAX_INVENTORY_CAPACITY];
    int inventoryCount=0;
    int emergencyTripsLeft=0;
    bool objectives[NUM_OBJECTIVES]={false};
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1];
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]={0};
    const char* mapFile="data/building.txt";
    const char* initStateFile="data/initial-state.txt";
    if(!loadBuildingMap(mapFile,horizontalConn,roomEntities,entityCounts)){
        cerr<<"Error: Could not load "<<mapFile<<endl;
        return 1;
    }
    if(!loadInitialState(initStateFile,playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives,emergencyTripsLeft)){
        cerr<<"Error: Could not load "<<initStateFile<<endl;
        return 1;
    }
    if(playerFloor==-1&&playerRoom==-1){
        int floor,room;
        bool validStart=false;
        while(!validStart){
            cout<<"Select your starting room."<<endl;
            cout<<"Enter floor (1-5): ";
            if(!(cin>>floor)){
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Invalid floor input."<<endl;
                continue;
            }
            cout<<"Enter room (1-4): ";
            if(!(cin>>room)){
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Invalid room input."<<endl;
                continue;
            }
            if(validateStartingRoom(floor-1,room-1,horizontalConn)){
                playerFloor=floor-1;
                playerRoom=room-1;
                validStart=true;
            }
            else{
                cout<<"Invalid starting room. Try again."<<endl;
            }
        }
    }
    cout<<"==========================================="<<endl;
    cout<<" THE LAST TRAIN - SIMULATION INITIALIZED "<<endl;
    cout<<"==========================================="<<endl;
    runSimulation(playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives,emergencyTripsLeft,horizontalConn,roomEntities,entityCounts);
    return 0;
}

