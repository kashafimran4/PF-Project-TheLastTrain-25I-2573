#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include "level.h"
#include <iostream>
#include <cstring>
#include <iomanip>
using namespace std;
const char SIM_STAIRS='S';
const char SIM_LIFT1='1';
const char SIM_LIFT2='2';
static bool removeRoomEntity(char entities[MAX_ENTITIES_PER_ROOM],int& count,char symbol){
    if(count<0||count>MAX_ENTITIES_PER_ROOM){
        return false;
    }
    for(int i=0;i<count;i++){
        if(entities[i]==symbol){
            for(int j=i;j<count-1;j++){
                entities[j]=entities[j+1];
            }
            entities[--count]=ENT_NONE;
            return true;
        }
    }
    return false;
}
static bool roomHasEntity(const char entities[MAX_ENTITIES_PER_ROOM],int count,char symbol){
    if(count<0||count>MAX_ENTITIES_PER_ROOM){
        return false;
    }
    for(int i=0;i<count;i++){
        if(entities[i]==symbol){
            return true;
        }
    }
    return false;
}
static void showHelp(){
    cout<<"COMMANDS"<<endl;
    cout<<"move floor room"<<endl;
    cout<<"inspect"<<endl;
    cout<<"collect symbol"<<endl;
    cout<<"use symbol"<<endl;
    cout<<"unlock 427"<<endl;
    cout<<"map"<<endl;
    cout<<"status"<<endl;
    cout<<"objectives"<<endl;
    cout<<"save"<<endl;
    cout<<"load"<<endl;
    cout<<"help"<<endl;
    cout<<"quit"<<endl;
}
bool handleMoveCommand(int targetFloor,int targetRoom,int& currentFloor,int& currentRoom,int& energy,int& time,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1],const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,int& emergencyTripsLeft,bool lift2Operational){
    char connection=CONN_NONE;
    if(!isDirectlyConnected(currentFloor,currentRoom,targetFloor,targetRoom,horizontalConn,connection)){
        cout<<"These rooms are not directly connected."<<endl;
        return false;
    }
    if(connection==SIM_LIFT1&&!canUseLift1(currentFloor,targetFloor)){
        cout<<"Lift 1 cannot be used between these floors."<<endl;
        return false;
    }
    if(connection==SIM_LIFT2&&!canUseLift2(currentFloor,targetFloor,lift2Operational,emergencyTripsLeft)){
        cout<<"Lift 2 is not available."<<endl;
        return false;
    }
    if(!canTraverse(targetFloor,targetRoom,connection,inventory,inventoryCount,emergencyTripsLeft,lift2Operational)){
        cout<<"Access denied. You need the required key or access item."<<endl;
        return false;
    }
    int energyCost=COST_MOVE_ORDINARY_ENERGY;
    int timeCost=COST_MOVE_ORDINARY_TIME;
    if(connection==CONN_WINDOW){
        energyCost=COST_MOVE_WINDOW_ENERGY;
        timeCost=COST_MOVE_WINDOW_TIME;
    }
    else if(connection==SIM_LIFT1||connection==SIM_LIFT2){
        energyCost=COST_MOVE_LIFT_ENERGY;
        timeCost=COST_MOVE_LIFT_TIME;
    }
    applyResourceDelta(energy,time,energyCost,timeCost);
    if(connection==SIM_LIFT2&&!lift2Operational){
        emergencyTripsLeft--;
        cout<<"Emergency lift trip used. Trips left: "<<emergencyTripsLeft<<endl;
    }
    currentFloor=targetFloor;
    currentRoom=targetRoom;
    cout<<"Movement successful. You are now in F"<<currentFloor+1<<"-R"<<currentRoom+1<<"."<<endl;
    return true;
}
bool handleInspectCommand(int currentFloor,int currentRoom,int& energy,int& time,const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCount,bool objectives[NUM_OBJECTIVES],int& score){
    if(currentFloor<0||currentFloor>=NUM_FLOORS||currentRoom<0||currentRoom>=ROOMS_PER_FLOOR||entityCount<0||entityCount>MAX_ENTITIES_PER_ROOM||objectives==NULL){
        return false;
    }
    applyResourceDelta(energy,time,COST_INSPECT_ENERGY,COST_INSPECT_TIME);
    cout<<"Room F"<<currentFloor+1<<"-R"<<currentRoom+1<<endl;
    renderRoomDetails(currentFloor,currentRoom,roomEntities,entityCount);
    if(objectives[OBJ_MASTER_KEY]){
        cout<<"Master Key: collected"<<endl;
    }
    if(objectives[OBJ_CABINET_UNLOCKED]){
        cout<<"Cabinet: unlocked"<<endl;
    }
    if(objectives[OBJ_FUSE_INSTALLED]){
        cout<<"Fuse: installed"<<endl;
    }
    if(objectives[OBJ_LIFT_PASS]){
        cout<<"Lift Pass: collected"<<endl;
    }
    (void)score;
    return true;
}
bool handleCollectCommand(char itemSymbol,int currentFloor,int currentRoom,int& energy,int& time,int& score,char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int& entityCount,char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES]){
    if(currentFloor<0||currentFloor>=NUM_FLOORS||currentRoom<0||currentRoom>=ROOMS_PER_FLOOR||entityCount<0||entityCount>MAX_ENTITIES_PER_ROOM||inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY||inventory==NULL||objectives==NULL){
        return false;
    }
    char* entities=roomEntities[currentFloor][currentRoom];
    if(!roomHasEntity(entities,entityCount,itemSymbol)){
        cout<<"That item is not in this room."<<endl;
        return false;
    }
    if(itemSymbol==ENT_CABINET_TRAIN_KEY&&!objectives[OBJ_CABINET_UNLOCKED]){
        cout<<"Unlock the cabinet before collecting the Train Key."<<endl;
        return false;
    }
    if(itemSymbol==ENT_PASSENGER||itemSymbol==ENT_CAT||itemSymbol==ENT_CONTROL_PANEL){
        cout<<"This object cannot be collected directly. Use the required item or action."<<endl;
        return false;
    }
    if(!addItem(inventory,inventoryCount,itemSymbol)){
        cout<<"Your inventory is full."<<endl;
        return false;
    }
    removeRoomEntity(entities,entityCount,itemSymbol);
    applyResourceDelta(energy,time,COST_ITEM_PICKUP_ENERGY,COST_ITEM_PICKUP_TIME);
    switch(itemSymbol){
        case ENT_MASTER_KEY:
            objectives[OBJ_MASTER_KEY]=true;
            applyScoreDelta(score,SCORE_KEY_COLLECT);
            break;
        case ENT_CLUE_4:
            objectives[OBJ_CLUE_4]=true;
            applyScoreDelta(score,SCORE_CLUE_FOUND);
            break;
        case ENT_CLUE_2:
            objectives[OBJ_CLUE_2]=true;
            applyScoreDelta(score,SCORE_CLUE_FOUND);
            break;
        case ENT_CLUE_7:
            objectives[OBJ_CLUE_7]=true;
            applyScoreDelta(score,SCORE_CLUE_FOUND);
            break;
        case ENT_CABINET_TRAIN_KEY:
            objectives[OBJ_TRAIN_KEY]=true;
            applyScoreDelta(score,SCORE_TRAIN_KEY);
            break;
        case ENT_FUSE:
            objectives[OBJ_FUSE_COLLECTED]=true;
            break;
        case ENT_LIFT_PASS:
            objectives[OBJ_LIFT_PASS]=true;
            applyScoreDelta(score,SCORE_LIFT_PASS);
            break;
        case ENT_CAT_CARRIER:
            objectives[OBJ_CAT_CARRIER]=true;
            break;
        default:
            break;
    }
    cout<<"Collected: "<<getEntityName(itemSymbol)<<endl;
    return true;
}
bool handleUseCommand(char itemSymbol,int currentFloor,int currentRoom,int& energy,int& time,int& score,char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES]){
    if(inventory==NULL||objectives==NULL||inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY){
        return false;
    }
    if(!hasItem(inventory,inventoryCount,itemSymbol)){
        cout<<"You do not have that item."<<endl;
        return false;
    }
    if(itemSymbol==ENT_ENERGY_PACK){
        removeItem(inventory,inventoryCount,itemSymbol);
        applyResourceDelta(energy,time,BONUS_ENERGY_PACK,0);
        cout<<"Energy restored by 25."<<endl;
    }
    else if(itemSymbol==ENT_EMERGENCY_TOOLKIT){
        removeItem(inventory,inventoryCount,itemSymbol);
        applyResourceDelta(energy,time,BONUS_EMERGENCY_TOOLKIT,0);
        cout<<"Energy restored by 15."<<endl;
    }
    else if(itemSymbol==ENT_OBSERVATION_TOKEN){
        removeItem(inventory,inventoryCount,itemSymbol);
        applyResourceDelta(energy,time,0,BONUS_OBSERVATION_TOKEN);
        cout<<"Time restored by 15."<<endl;
    }
    else if(itemSymbol==ENT_FUSE){
        if(currentFloor!=2||currentRoom!=3){
            cout<<"You must be at the Control Panel in F3-R4 to install the Fuse."<<endl;
            return false;
        }
        if(!installFuseControlPanel(inventory,inventoryCount,objectives,score)){
            cout<<"Fuse installation failed."<<endl;
            return false;
        }
        applyResourceDelta(energy,time,COST_ITEM_USE_ENERGY,COST_ITEM_USE_TIME);
        cout<<"Fuse installed. Lift 2 is now operational."<<endl;
    }
    else if(itemSymbol==ENT_CAT_CARRIER){
        if(currentFloor!=2||currentRoom!=2||objectives[OBJ_CAT_SECURED]){
            cout<<"You must be in F3-R3 with the cat to use the Cat Carrier."<<endl;
            return false;
        }
        objectives[OBJ_CAT_SECURED]=true;
        removeItem(inventory,inventoryCount,itemSymbol);
        applyResourceDelta(energy,time,COST_ITEM_USE_ENERGY,COST_ITEM_USE_TIME);
        applyScoreDelta(score,SCORE_SECURE_CAT);
        cout<<"The cat is safely secured in the carrier."<<endl;
    }
    else if(itemSymbol==ENT_MEDICAL_KIT){
        if(currentFloor!=3||currentRoom!=0||objectives[OBJ_PASSENGER_RESCUED]){
            cout<<"You must be in F4-R1 with the passenger to use the Medical Kit."<<endl;
            return false;
        }
        objectives[OBJ_PASSENGER_RESCUED]=true;
        removeItem(inventory,inventoryCount,itemSymbol);
        applyResourceDelta(energy,time,COST_ITEM_USE_ENERGY,COST_ITEM_USE_TIME);
        applyScoreDelta(score,SCORE_RESCUE_PASSENGER);
        cout<<"Passenger rescued successfully."<<endl;
    }
    else if(itemSymbol==ENT_EMERGENCY_CARD){
        cout<<"The Emergency Access Card is used automatically when accessing Lift 2 before repair."<<endl;
        return false;
    }
    else{
        cout<<"This item cannot be used here."<<endl;
        return false;
    }
    return true;
}
bool handleUnlockCabinetCommand(const char* code,int currentFloor,int currentRoom,bool objectives[NUM_OBJECTIVES],int& score,char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int& entityCount){
    if(code==NULL||objectives==NULL){
        return false;
    }
    if(currentFloor!=4||currentRoom!=0){
        cout<<"The cabinet is located in F5-R1."<<endl;
        return false;
    }
    if(objectives[OBJ_CABINET_UNLOCKED]){
        cout<<"The cabinet is already unlocked."<<endl;
        return false;
    }
    if(!verifyCabinetCode(code,objectives)){
        cout<<"Incorrect code or missing clues. Collect clues 4, 2, and 7 first."<<endl;
        return false;
    }
    objectives[OBJ_CABINET_UNLOCKED]=true;
    applyScoreDelta(score,SCORE_CABINET_UNLOCK);
    cout<<"Cabinet unlocked successfully. You can now collect the Train Key."<<endl;
    (void)roomEntities;
    (void)entityCount;
    return true;
}
bool checkEndConditions(int energy,int time,const bool objectives[NUM_OBJECTIVES]){
    if(objectives==NULL){
        return true;
    }
    return objectives[OBJ_MISSION_WON]||objectives[OBJ_MISSION_FAILED]||energy<=0||time<=0;
}
void runSimulation(int& playerFloor,int& playerRoom,int& energy,int& time,int& score,char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES],int& emergencyTripsLeft,char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1],char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    if(playerFloor<0||playerFloor>=NUM_FLOORS||playerRoom<0||playerRoom>=ROOMS_PER_FLOOR||inventory==NULL||objectives==NULL){
        cout<<"Invalid starting room or game state."<<endl;
        return;
    }
    bool running=true;
    bool victory=objectives[OBJ_MISSION_WON];
    char command[32];
    while(running){
        bool lift2Operational=objectives[OBJ_FUSE_INSTALLED];
        cout<<"THE LAST TRAIN"<<endl;
        renderDashboard(playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,emergencyTripsLeft);
        renderAsciiBuilding(playerFloor,playerRoom);
        if(checkEndConditions(energy,time,objectives)){
            break;
        }
        cout<<"Enter command (type help): ";
        if(!(cin>>setw(sizeof(command))>>command)){
            cin.clear();
            cin.ignore(10000,'\n');
            cout<<"Invalid input."<<endl;
            continue;
        }
        if(strcmp(command,"move")==0){
            int floor,room;
            if(!(cin>>floor>>room)){
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Invalid movement input."<<endl;
                continue;
            }
            handleMoveCommand(floor-1,room-1,playerFloor,playerRoom,energy,time,horizontalConn,inventory,inventoryCount,emergencyTripsLeft,lift2Operational);
        }
        else if(strcmp(command,"inspect")==0){
            handleInspectCommand(playerFloor,playerRoom,energy,time,roomEntities,entityCounts[playerFloor][playerRoom],objectives,score);
        }
        else if(strcmp(command,"collect")==0){
            char symbol;
            if(cin>>symbol){
                int oldInventoryCount=inventoryCount;
                handleCollectCommand(symbol,playerFloor,playerRoom,energy,time,score,roomEntities,entityCounts[playerFloor][playerRoom],inventory,inventoryCount,objectives);
                if(inventoryCount>oldInventoryCount&&symbol==ENT_EMERGENCY_CARD){
                    emergencyTripsLeft=3;
                    cout<<"You received 3 emergency Lift 2 trips."<<endl;
                }
            }
            else{
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Missing item symbol."<<endl;
            }
        }
        else if(strcmp(command,"use")==0){
            char symbol;
            if(cin>>symbol){
                handleUseCommand(symbol,playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives);
            }
            else{
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Missing item symbol."<<endl;
            }
        }
        else if(strcmp(command,"unlock")==0){
            char code[32];
            if(cin>>setw(sizeof(code))>>code){
                handleUnlockCabinetCommand(code,playerFloor,playerRoom,objectives,score,roomEntities,entityCounts[playerFloor][playerRoom]);
            }
            else{
                cin.clear();
                cin.ignore(10000,'\n');
                cout<<"Missing cabinet code."<<endl;
            }
        }
        else if(strcmp(command,"map")==0){
            renderAsciiBuilding(playerFloor,playerRoom);
        }
        else if(strcmp(command,"status")==0){
            renderDashboard(playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,emergencyTripsLeft);
        }
        else if(strcmp(command,"objectives")==0){
            renderObjectivesChecklist(objectives);
        }
        else if(strcmp(command,"save")==0){
            if(saveGameStateToFile("out/savegame.txt",playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives,emergencyTripsLeft,roomEntities,entityCounts)){
                cout<<"Game saved successfully."<<endl;
            }
            else{
                cout<<"Could not save game. Check the out folder."<<endl;
            }
        }
        else if(strcmp(command,"load")==0){
            if(loadGameStateFromFile("out/savegame.txt",playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives,emergencyTripsLeft,roomEntities,entityCounts)){
                cout<<"Saved game loaded successfully."<<endl;
            }
            else{
                cout<<"Could not load saved game."<<endl;
            }
        }
        else if(strcmp(command,"help")==0){
            showHelp();
        }
        else if(strcmp(command,"quit")==0){
            cout<<"Game paused. Save before exiting if you want to resume later."<<endl;
            running=false;
        }
        else{
            cout<<"Unknown command. Type help to see available commands."<<endl;
        }
        if(energy<=0||time<=0){
            objectives[OBJ_MISSION_FAILED]=true;
            objectives[OBJ_MISSION_WON]=false;
            victory=false;
            running=false;
            cout<<"MISSION FAILED: You ran out of energy or time."<<endl;
            break;
        }
        if(!objectives[OBJ_MISSION_WON]&&objectives[OBJ_TRAIN_KEY]&&objectives[OBJ_PASSENGER_RESCUED]&&objectives[OBJ_CAT_SECURED]&&objectives[OBJ_FUSE_INSTALLED]&&objectives[OBJ_LIFT_PASS]&&playerFloor==4&&playerRoom==3){
            objectives[OBJ_TRAIN_PREPARED]=true;
            applyScoreDelta(score,SCORE_PREPARE_TRAIN);
            objectives[OBJ_MISSION_WON]=true;
            applyScoreDelta(score,SCORE_MISSION_VICTORY);
            cout<<"All objectives completed! You boarded the train!"<<endl;
            running=false;
            victory=true;
        }
    }
    if(energy<=0||time<=0){
        objectives[OBJ_MISSION_FAILED]=true;
        objectives[OBJ_MISSION_WON]=false;
        victory=false;
    }
    if(victory){
        cout<<"MISSION VICTORY!"<<endl;
    }
    if(!saveGameStateToFile("out/savegame.txt",playerFloor,playerRoom,energy,time,score,inventory,inventoryCount,objectives,emergencyTripsLeft,roomEntities,entityCounts)){
        cout<<"Warning: final save could not be written."<<endl;
    }
    writeGameSummaryReport("out/game_summary.txt",score,energy,time,objectives,victory);
}
