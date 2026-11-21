#include "switches.h"
#include "util.h"
#include <cstring>
bool canUseLift1(int fromFloor,int toFloor){
    if(fromFloor<0||fromFloor>=NUM_FLOORS||toFloor<0||toFloor>=NUM_FLOORS)return false;
    return (fromFloor==0&&toFloor==1)||(fromFloor==1&&toFloor==0);
}
bool canUseLift2(int fromFloor,int toFloor,bool lift2Operational,int emergencyTripsLeft){
    if(fromFloor<0||fromFloor>=NUM_FLOORS||toFloor<0||toFloor>=NUM_FLOORS||fromFloor==toFloor)return false;
    bool validFrom=(fromFloor==1||fromFloor==3||fromFloor==4);
    bool validTo=(toFloor==1||toFloor==3||toFloor==4);
    if(!validFrom||!validTo)return false;
    if(lift2Operational)return true;
    return emergencyTripsLeft>0;
}
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount){
    if(inventory==NULL||inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY)return false;
    for(int i=0;i<inventoryCount;i++){
        if(inventory[i]==ENT_MASTER_KEY)return true;
    }
    return false;
}
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,bool objectives[NUM_OBJECTIVES],int& score){
    if(inventory==NULL||objectives==NULL||inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY)return false;
    if(objectives[OBJ_FUSE_INSTALLED]||!objectives[OBJ_FUSE_COLLECTED])return false;
    int fuseIndex=-1;
    for(int i=0;i<inventoryCount;i++){
        if(inventory[i]==ENT_FUSE){
            fuseIndex=i;
            break;
        }
    }
    if(fuseIndex<0)return false;
    for(int i=fuseIndex;i<inventoryCount-1;i++)inventory[i]=inventory[i+1];
    inventory[--inventoryCount]=ENT_NONE;
    objectives[OBJ_FUSE_INSTALLED]=true;
    score+=SCORE_INSTALL_FUSE;
    if(score<0)score=0;
    return true;
}
bool verifyCabinetCode(const char* enteredCode,const bool objectives[NUM_OBJECTIVES]){
    if(enteredCode==NULL||objectives==NULL)return false;
    if(!objectives[OBJ_CLUE_4]||!objectives[OBJ_CLUE_2]||!objectives[OBJ_CLUE_7])return false;
    return strcmp(enteredCode,"427")==0;
}
