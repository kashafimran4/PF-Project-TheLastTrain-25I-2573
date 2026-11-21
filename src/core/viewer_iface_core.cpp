#include "viewer_iface.h"
#include "util.h"
#include <iostream>
using namespace std;
static const char* itemName(char symbol){
    switch(symbol){
        case ENT_MASTER_KEY:return "Master Key";
        case ENT_ENERGY_PACK:return "Energy Pack";
        case ENT_EMERGENCY_TOOLKIT:return "Emergency Toolkit";
        case ENT_CLUE_4:return "Clue 4";
        case ENT_FUSE:return "Fuse";
        case ENT_CAT:return "Cat";
        case ENT_CONTROL_PANEL:return "Control Panel";
        case ENT_LIFT_PASS:return "Lift Pass";
        case ENT_PASSENGER:return "Passenger";
        case ENT_MEDICAL_KIT:return "Medical Kit";
        case ENT_EMERGENCY_CARD:return "Emergency Access Card";
        case ENT_CLUE_2:return "Clue 2";
        case ENT_CLUE_7:return "Clue 7";
        case ENT_CABINET_TRAIN_KEY:return "Cabinet Train Key";
        case ENT_CAT_CARRIER:return "Cat Carrier";
        case ENT_OBSERVATION_TOKEN:return "Observation Token";
        default:return "Unknown Item";
    }
}
void renderDashboard(int playerFloor,int playerRoom,int energy,int time,int score,
                     const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,
                     int emergencyTripsLeft){
    printHeader("PLAYER DASHBOARD");
    cout<<"Location: F"<<playerFloor+1<<"-R"<<playerRoom+1<<endl;
    cout<<"Energy: "<<clampInt(energy,0,MAX_ENERGY)<<"/"<<MAX_ENERGY<<endl;
    cout<<"Time: "<<clampInt(time,0,MAX_TIME)<<"/"<<MAX_TIME<<endl;
    cout<<"Score: "<<score<<endl;
    cout<<"Emergency Lift Trips: "<<emergencyTripsLeft<<endl;
    cout<<"Inventory ("<<inventoryCount<<"/"<<MAX_INVENTORY_CAPACITY<<"): ";
    if(inventoryCount==0)cout<<"Empty";
    else{
        for(int i=0;i<inventoryCount;i++){
            if(i>0)cout<<", ";
            cout<<itemName(inventory[i]);
        }
    }
    cout<<endl;
}
void renderRoomDetails(int floor,int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount){
    if(floor<0||floor>=NUM_FLOORS||room<0||room>=ROOMS_PER_FLOOR){
        cout<<"Invalid room."<<endl;
        return;
    }
    if(entityCount<0||entityCount>MAX_ENTITIES_PER_ROOM){
        cout<<"Room data is invalid."<<endl;
        return;
    }
    cout<<"Room contents: ";
    if(entityCount==0){
        cout<<"No remaining objects."<<endl;
        return;
    }
    cout<<endl;
    for(int i=0;i<entityCount;i++)
        cout<<" - "<<itemName(roomEntities[floor][room][i])<<" ("<<roomEntities[floor][room][i]<<")"<<endl;
}
void renderAsciiBuilding(int playerFloor,int playerRoom){
    printHeader("BUILDING MAP");
    for(int floor=NUM_FLOORS-1;floor>=0;floor--){
        cout<<"Floor "<<floor+1<<"  ";
        for(int room=0;room<ROOMS_PER_FLOOR;room++){
            if(floor==playerFloor&&room==playerRoom)cout<<"[ YOU ]";
            else cout<<"[ F"<<floor+1<<"-R"<<room+1<<" ]";
            if(room<ROOMS_PER_FLOOR-1)cout<<"---";
        }
        cout<<endl;
    }
    cout<<"Stairs: F1-R1 <-> F2-R1; F2-R1 <-> F3-R1; F4-R1 <-> F5-R1"<<endl;
    cout<<"Lift 1: Floors 1 and 2, Rooms 2 and 3"<<endl;
    cout<<"Lift 2: F2-R4, F4-R4, F5-R4"<<endl;
}
void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES]){
    if(objectives==NULL){
        cout<<"Objectives are unavailable."<<endl;
        return;
    }
    const char* names[NUM_OBJECTIVES]={
        "Master Key collected","Clue 4 found","Clue 2 found","Clue 7 found",
        "Cabinet unlocked","Train Key collected","Fuse collected","Fuse installed",
        "Lift Pass collected","Lift 2 activated","Passenger rescued","Cat Carrier collected",
        "Cat secured","Train prepared","Mission won","Mission failed"
    };
    printHeader("OBJECTIVES");
    for(int i=0;i<NUM_OBJECTIVES;i++)
        cout<<(objectives[i]?"[X] ":"[ ] ")<<names[i]<<endl;
}
