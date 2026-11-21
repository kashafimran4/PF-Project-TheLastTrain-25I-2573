#include "level.h"
#include "util.h"
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;
static bool isEntitySymbol(char c){
    return c==ENT_MASTER_KEY||c==ENT_ENERGY_PACK||c==ENT_EMERGENCY_TOOLKIT||c==ENT_CLUE_4||c==ENT_FUSE||c==ENT_CAT||c==ENT_CONTROL_PANEL||c==ENT_LIFT_PASS||c==ENT_PASSENGER||c==ENT_MEDICAL_KIT||c==ENT_EMERGENCY_CARD||c==ENT_CLUE_2||c==ENT_CLUE_7||c==ENT_CABINET_TRAIN_KEY||c==ENT_CAT_CARRIER||c==ENT_OBSERVATION_TOKEN;
}
static bool readBoolean(const char* value){
    return strcmp(value,"YES")==0||strcmp(value,"TRUE")==0||strcmp(value,"1")==0;
}
static void addEntity(char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR],int floor,int room,char entity){
    if(floor<0||floor>=NUM_FLOORS||room<0||room>=ROOMS_PER_FLOOR)return;
    if(entityCounts[floor][room]>=MAX_ENTITIES_PER_ROOM)return;
    roomEntities[floor][room][entityCounts[floor][room]++]=entity;
}
bool loadBuildingMap(const char* filename,char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1],char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    FILE* file=fopen(filename,"r");
    if(file==NULL)return false;
    bool floorsSeen[NUM_FLOORS]={false};
    for(int f=0;f<NUM_FLOORS;f++){
        for(int r=0;r<ROOMS_PER_FLOOR-1;r++)horizontalConn[f][r]=CONN_NONE;
        for(int r=0;r<ROOMS_PER_FLOOR;r++){
            entityCounts[f][r]=0;
            for(int e=0;e<MAX_ENTITIES_PER_ROOM;e++)roomEntities[f][r][e]=ENT_NONE;
        }
    }
    char line[MAX_LINE_LEN];
    while(fgets(line,sizeof(line),file)){
        int positions[ROOMS_PER_FLOOR],found=0;
        char* p=line;
        while((p=strstr(p,"F"))!=NULL){
            int floorNumber,roomNumber,used=0;
            if(sscanf(p,"F%d-R%d%n",&floorNumber,&roomNumber,&used)==2&&floorNumber>=1&&floorNumber<=NUM_FLOORS&&roomNumber>=1&&roomNumber<=ROOMS_PER_FLOOR){
                if(found<ROOMS_PER_FLOOR)positions[found++]=(int)(p-line);
                p+=used;
            }else p++;
        }
        if(found!=ROOMS_PER_FLOOR)continue;
        int floorNumber=0;
        if(sscanf(line+positions[0],"F%d",&floorNumber)!=1)continue;
        if(floorNumber<1||floorNumber>NUM_FLOORS||floorsSeen[floorNumber-1])continue;
        bool validFloor=true;
        for(int r=0;r<ROOMS_PER_FLOOR;r++){
            int labelFloor=0,roomNumber=0;
            if(sscanf(line+positions[r],"F%d-R%d",&labelFloor,&roomNumber)!=2||labelFloor!=floorNumber||roomNumber!=r+1)validFloor=false;
        }
        if(!validFloor)continue;
        floorsSeen[floorNumber-1]=true;
        for(int r=0;r<ROOMS_PER_FLOOR-1;r++){
            int start=positions[r]+5,end=positions[r+1];
            for(int x=start;x<end&&line[x]!='\0';x++){
                char c=line[x];
                if(c=='L'&&line[x+1]>='0'&&line[x+1]<='9')continue;
                if(c==CONN_DOOR||c==CONN_WINDOW||c==CONN_LOCKED_DOOR||c==CONN_SOLID_WALL){
                    horizontalConn[floorNumber-1][r]=c;
                    break;
                }
            }
        }
        if(!fgets(line,sizeof(line),file))break;
        int lineLength=(int)strlen(line);
        for(int x=0;x<lineLength;x++){
            if(!isEntitySymbol(line[x]))continue;
            int room=-1;
            for(int r=0;r<ROOMS_PER_FLOOR;r++){
                int left=positions[r],right=(r<ROOMS_PER_FLOOR-1)?positions[r+1]:positions[r]+12;
                if(x>=left&&x<right){
                    room=r;
                    break;
                }
            }
            if(room!=-1)addEntity(roomEntities,entityCounts,floorNumber-1,room,line[x]);
        }
    }
    fclose(file);
    for(int f=0;f<NUM_FLOORS;f++)if(!floorsSeen[f])return false;
    return true;
}
bool loadInitialState(const char* filename,int& playerFloor,int& playerRoom,int& energy,int& time,int& score,char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES],int& emergencyTripsLeft){
    ifstream file(filename);
    if(!file.is_open())return false;
    playerFloor=-1;
    playerRoom=-1;
    energy=INITIAL_ENERGY;
    time=INITIAL_TIME;
    score=INITIAL_SCORE;
    inventoryCount=0;
    emergencyTripsLeft=0;
    for(int i=0;i<MAX_INVENTORY_CAPACITY;i++)inventory[i]=ENT_NONE;
    for(int i=0;i<NUM_OBJECTIVES;i++)objectives[i]=false;
    char line[MAX_LINE_LEN];
    while(file.getline(line,sizeof(line))){
        char* colon=strchr(line,':');
        if(colon==NULL)continue;
        *colon='\0';
        char* key=line;
        char* value=colon+1;
        trimWhitespace(key);
        trimWhitespace(value);
        if(strcmp(key,"START_ROOM")==0){
            if(strcmp(value,"PLAYER_SELECTS")==0){
                playerFloor=-1;
                playerRoom=-1;
            }else{
                int floorNumber,roomNumber;
                if(sscanf(value,"F%d-R%d",&floorNumber,&roomNumber)!=2||floorNumber<1||floorNumber>NUM_FLOORS||roomNumber<1||roomNumber>ROOMS_PER_FLOOR){
                    file.close();
                    return false;
                }
                playerFloor=floorNumber-1;
                playerRoom=roomNumber-1;
            }
        }else if(strcmp(key,"INITIAL_ENERGY")==0)energy=atoi(value);
        else if(strcmp(key,"INITIAL_TIME")==0)time=atoi(value);
        else if(strcmp(key,"INITIAL_SCORE")==0)score=atoi(value);
        else if(strcmp(key,"EMERGENCY_TRIPS_LEFT")==0)emergencyTripsLeft=atoi(value);
        else if(strcmp(key,"MASTER_KEY_COLLECTED")==0)objectives[OBJ_MASTER_KEY]=readBoolean(value);
        else if(strcmp(key,"PASSENGER_RESCUED")==0)objectives[OBJ_PASSENGER_RESCUED]=readBoolean(value);
        else if(strcmp(key,"CAT_RESCUED")==0)objectives[OBJ_CAT_SECURED]=readBoolean(value);
        else if(strcmp(key,"CABINET_UNLOCKED")==0)objectives[OBJ_CABINET_UNLOCKED]=readBoolean(value);
        else if(strcmp(key,"TRAIN_KEY_COLLECTED")==0)objectives[OBJ_TRAIN_KEY]=readBoolean(value);
        else if(strcmp(key,"FUSE_COLLECTED")==0)objectives[OBJ_FUSE_COLLECTED]=readBoolean(value);
        else if(strcmp(key,"FUSE_INSTALLED")==0){
            objectives[OBJ_FUSE_INSTALLED]=readBoolean(value);
            if(objectives[OBJ_FUSE_INSTALLED])objectives[OBJ_FUSE_COLLECTED]=true;
        }else if(strcmp(key,"LIFT_PASS_COLLECTED")==0)objectives[OBJ_LIFT_PASS]=readBoolean(value);
        else if(strcmp(key,"LIFT2_ACTIVE")==0)objectives[OBJ_LIFT2_ACTIVE]=readBoolean(value);
        else if(strcmp(key,"TRAIN_PREPARED")==0)objectives[OBJ_TRAIN_PREPARED]=readBoolean(value);
        else if(strcmp(key,"MISSION_WON")==0)objectives[OBJ_MISSION_WON]=readBoolean(value);
        else if(strcmp(key,"MISSION_FAILED")==0)objectives[OBJ_MISSION_FAILED]=readBoolean(value);
        else if(strcmp(key,"CLUES_COLLECTED")==0){
            if(strchr(value,'4'))objectives[OBJ_CLUE_4]=true;
            if(strchr(value,'2'))objectives[OBJ_CLUE_2]=true;
            if(strchr(value,'7'))objectives[OBJ_CLUE_7]=true;
        }else if(strcmp(key,"INITIAL_INVENTORY")==0&&strcmp(value,"NONE")!=0){
            for(int i=0;value[i]!='\0';i++){
                if(isEntitySymbol(value[i])){
                    if(inventoryCount>=MAX_INVENTORY_CAPACITY){
                        file.close();
                        return false;
                    }
                    inventory[inventoryCount++]=value[i];
                }
            }
        }
    }
    file.close();
    energy=clampInt(energy,0,MAX_ENERGY);
    time=clampInt(time,0,MAX_TIME);
    if(score<0)score=0;
    if(emergencyTripsLeft<0)emergencyTripsLeft=0;
    return true;
}
bool validateStartingRoom(int startFloor,int startRoom,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1]){
    if(startFloor<0||startFloor>=NUM_FLOORS||startRoom<0||startRoom>=ROOMS_PER_FLOOR)return false;
    for(int f=0;f<NUM_FLOORS;f++){
        for(int r=0;r<ROOMS_PER_FLOOR-1;r++){
            char c=horizontalConn[f][r];
            if(c!=CONN_NONE&&c!=CONN_DOOR&&c!=CONN_WINDOW&&c!=CONN_LOCKED_DOOR&&c!=CONN_SOLID_WALL)return false;
        }
    }
    return true;
}
const char* getEntityName(char entitySymbol){
    switch(entitySymbol){
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
        case ENT_CABINET_TRAIN_KEY:return "Cabinet / Train Key";
        case ENT_CAT_CARRIER:return "Cat Carrier";
        case ENT_OBSERVATION_TOKEN:return "Observation Token";
        default:return "Unknown Entity";
    }
}
