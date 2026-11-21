#include "logger.h"
#include "util.h"
#include <fstream>
#include <string>
using namespace std;
bool saveGameStateToFile(const char* filename,int playerFloor,int playerRoom,int energy,int time,int score,const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,const bool objectives[NUM_OBJECTIVES],int emergencyTripsLeft,const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    if(filename==NULL||inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY||playerFloor<-1||playerFloor>=NUM_FLOORS||playerRoom<-1||playerRoom>=ROOMS_PER_FLOOR||(playerFloor==-1)!=(playerRoom==-1))return false;
    ofstream file(filename);
    if(!file.is_open())return false;
    file<<"PLAYER_FLOOR: "<<playerFloor<<endl;
    file<<"PLAYER_ROOM: "<<playerRoom<<endl;
    file<<"ENERGY: "<<clampInt(energy,0,MAX_ENERGY)<<endl;
    file<<"TIME: "<<clampInt(time,0,MAX_TIME)<<endl;
    file<<"SCORE: "<<(score<0?0:score)<<endl;
    file<<"INVENTORY_COUNT: "<<inventoryCount<<endl;
    file<<"INVENTORY: ";
    for(int i=0;i<inventoryCount;i++)file<<(int)(unsigned char)inventory[i]<<' ';
    file<<endl;
    file<<"EMERGENCY_TRIPS_LEFT: "<<(emergencyTripsLeft<0?0:emergencyTripsLeft)<<endl;
    file<<"OBJECTIVES: ";
    for(int i=0;i<NUM_OBJECTIVES;i++)file<<(objectives[i]?1:0)<<' ';
    file<<endl<<"ROOM_ENTITIES:"<<endl;
    for(int f=0;f<NUM_FLOORS;f++){
        for(int r=0;r<ROOMS_PER_FLOOR;r++){
            if(entityCounts[f][r]<0||entityCounts[f][r]>MAX_ENTITIES_PER_ROOM){
                file.close();
                return false;
            }
            file<<f<<' '<<r<<' '<<entityCounts[f][r]<<' ';
            for(int e=0;e<entityCounts[f][r];e++)file<<(int)(unsigned char)roomEntities[f][r][e]<<' ';
            file<<endl;
        }
    }
    file.close();
    return !file.fail();
}
bool loadGameStateFromFile(const char* filename,int& playerFloor,int& playerRoom,int& energy,int& time,int& score,char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES],int& emergencyTripsLeft,char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    if(filename==NULL)return false;
    ifstream file(filename);
    if(!file.is_open())return false;
    char label[64];
    int loadedFloor,loadedRoom,loadedEnergy,loadedTime,loadedScore,loadedInventoryCount,loadedTrips;
    char loadedInventory[MAX_INVENTORY_CAPACITY];
    bool loadedObjectives[NUM_OBJECTIVES];
    char loadedEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int loadedCounts[NUM_FLOORS][ROOMS_PER_FLOOR];
    if(!(file>>label>>loadedFloor)||string(label)!="PLAYER_FLOOR:")return false;
    if(!(file>>label>>loadedRoom)||string(label)!="PLAYER_ROOM:")return false;
    if(!(file>>label>>loadedEnergy)||string(label)!="ENERGY:")return false;
    if(!(file>>label>>loadedTime)||string(label)!="TIME:")return false;
    if(!(file>>label>>loadedScore)||string(label)!="SCORE:")return false;
    if(!(file>>label>>loadedInventoryCount)||string(label)!="INVENTORY_COUNT:"||loadedInventoryCount<0||loadedInventoryCount>MAX_INVENTORY_CAPACITY)return false;
    if(!(file>>label)||string(label)!="INVENTORY:")return false;
    for(int i=0;i<MAX_INVENTORY_CAPACITY;i++)loadedInventory[i]=' ';
    for(int i=0;i<loadedInventoryCount;i++){
        int symbol;
        if(!(file>>symbol)||symbol<0||symbol>255)return false;
        loadedInventory[i]=(char)(unsigned char)symbol;
    }
    if(!(file>>label>>loadedTrips)||string(label)!="EMERGENCY_TRIPS_LEFT:")return false;
    if(!(file>>label)||string(label)!="OBJECTIVES:")return false;
    for(int i=0;i<NUM_OBJECTIVES;i++){
        int value;
        if(!(file>>value)||(value!=0&&value!=1))return false;
        loadedObjectives[i]=(value==1);
    }
    if(!(file>>label)||string(label)!="ROOM_ENTITIES:")return false;
    for(int f=0;f<NUM_FLOORS;f++){
        for(int r=0;r<ROOMS_PER_FLOOR;r++){
            int readFloor,readRoom,count;
            if(!(file>>readFloor>>readRoom>>count)||readFloor!=f||readRoom!=r||count<0||count>MAX_ENTITIES_PER_ROOM)return false;
            loadedCounts[f][r]=count;
            for(int e=0;e<MAX_ENTITIES_PER_ROOM;e++)loadedEntities[f][r][e]=' ';
            for(int e=0;e<count;e++){
                int symbol;
                if(!(file>>symbol)||symbol<0||symbol>255)return false;
                loadedEntities[f][r][e]=(char)(unsigned char)symbol;
            }
        }
    }
    if(loadedFloor<-1||loadedFloor>=NUM_FLOORS||loadedRoom<-1||loadedRoom>=ROOMS_PER_FLOOR||(loadedFloor==-1)!=(loadedRoom==-1))return false;
    playerFloor=loadedFloor;
    playerRoom=loadedRoom;
    energy=clampInt(loadedEnergy,0,MAX_ENERGY);
    time=clampInt(loadedTime,0,MAX_TIME);
    score=loadedScore<0?0:loadedScore;
    inventoryCount=loadedInventoryCount;
    for(int i=0;i<MAX_INVENTORY_CAPACITY;i++)inventory[i]=loadedInventory[i];
    emergencyTripsLeft=loadedTrips<0?0:loadedTrips;
    for(int i=0;i<NUM_OBJECTIVES;i++)objectives[i]=loadedObjectives[i];
    for(int f=0;f<NUM_FLOORS;f++){
        for(int r=0;r<ROOMS_PER_FLOOR;r++){
            entityCounts[f][r]=loadedCounts[f][r];
            for(int e=0;e<MAX_ENTITIES_PER_ROOM;e++)roomEntities[f][r][e]=loadedEntities[f][r][e];
        }
    }
    return true;
}
void writeGameSummaryReport(const char* filename,int score,int remainingEnergy,int remainingTime,const bool objectives[NUM_OBJECTIVES],bool victory){
    if(filename==NULL)return;
    ofstream file(filename);
    if(!file.is_open())return;
    file<<"THE LAST TRAIN - GAME SUMMARY"<<endl;
    file<<"Result: "<<(victory?"VICTORY":"MISSION FAILED")<<endl;
    file<<"Final Score: "<<score<<endl;
    file<<"Remaining Energy: "<<clampInt(remainingEnergy,0,MAX_ENERGY)<<endl;
    file<<"Remaining Time: "<<clampInt(remainingTime,0,MAX_TIME)<<endl;
    file<<"OBJECTIVES:"<<endl;
    file<<"Master Key: "<<(objectives[OBJ_MASTER_KEY]?"Completed":"Not Completed")<<endl;
    file<<"Clue 4: "<<(objectives[OBJ_CLUE_4]?"Found":"Not Found")<<endl;
    file<<"Clue 2: "<<(objectives[OBJ_CLUE_2]?"Found":"Not Found")<<endl;
    file<<"Clue 7: "<<(objectives[OBJ_CLUE_7]?"Found":"Not Found")<<endl;
    file<<"Cabinet Unlocked: "<<(objectives[OBJ_CABINET_UNLOCKED]?"Yes":"No")<<endl;
    file<<"Train Key: "<<(objectives[OBJ_TRAIN_KEY]?"Collected":"Not Collected")<<endl;
    file<<"Fuse Installed: "<<(objectives[OBJ_FUSE_INSTALLED]?"Yes":"No")<<endl;
    file<<"Lift Pass: "<<(objectives[OBJ_LIFT_PASS]?"Collected":"Not Collected")<<endl;
    file<<"Passenger Rescued: "<<(objectives[OBJ_PASSENGER_RESCUED]?"Yes":"No")<<endl;
    file<<"Cat Secured: "<<(objectives[OBJ_CAT_SECURED]?"Yes":"No")<<endl;
    file<<"Train Prepared: "<<(objectives[OBJ_TRAIN_PREPARED]?"Yes":"No")<<endl;
    file<<"Mission Won: "<<(objectives[OBJ_MISSION_WON]?"Yes":"No")<<endl;
}
