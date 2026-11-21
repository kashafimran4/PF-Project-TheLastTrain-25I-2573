#include "physics.h"
#include "util.h"
const char CONN_STAIRS='S';
const char CONN_LIFT1='1';
const char CONN_LIFT2='2';
bool isDirectlyConnected(int fromF,int fromR,int toF,int toR,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR-1],char& outConnType){
    outConnType=CONN_NONE;
    if(fromF<0||fromF>=NUM_FLOORS||toF<0||toF>=NUM_FLOORS||fromR<0||fromR>=ROOMS_PER_FLOOR||toR<0||toR>=ROOMS_PER_FLOOR)return false;
    if(fromF==toF&&fromR==toR)return false;
    if(fromF==toF&&(fromR-toR==1||toR-fromR==1)){
        int boundary=(fromR<toR)?fromR:toR;
        char connection=horizontalConn[fromF][boundary];
        if(connection!=CONN_NONE){
            outConnType=connection;
            return true;
        }
    }
    if(fromR==0&&toR==0){
        if((fromF==0&&toF==1)||(fromF==1&&toF==0)||(fromF==1&&toF==2)||(fromF==2&&toF==1)||(fromF==3&&toF==4)||(fromF==4&&toF==3)){
            outConnType=CONN_STAIRS;
            return true;
        }
    }
    if(fromF<=1&&toF<=1&&fromR>=1&&fromR<=2&&toR>=1&&toR<=2){
        outConnType=CONN_LIFT1;
        return true;
    }
    bool fromLift2=fromR==3&&(fromF==1||fromF==3||fromF==4);
    bool toLift2=toR==3&&(toF==1||toF==3||toF==4);
    if(fromLift2&&toLift2){
        outConnType=CONN_LIFT2;
        return true;
    }
    return false;
}
bool canTraverse(int targetFloor,int targetRoom,char connType,const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,int emergencyTripsLeft,bool lift2Operational){
    if(targetFloor<0||targetFloor>=NUM_FLOORS||targetRoom<0||targetRoom>=ROOMS_PER_FLOOR)return false;
    if(inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY)return false;
    if(connType==CONN_NONE||connType==CONN_SOLID_WALL)return false;
    if(connType==CONN_LOCKED_DOOR)
        return hasItem(inventory,inventoryCount,ENT_MASTER_KEY);
    if(connType==CONN_LIFT2){
        if(lift2Operational)
            return hasItem(inventory,inventoryCount,ENT_LIFT_PASS);
        return emergencyTripsLeft>0&&hasItem(inventory,inventoryCount,ENT_EMERGENCY_CARD);
    }
    if(connType==CONN_DOOR||connType==CONN_WINDOW||connType==CONN_STAIRS||connType==CONN_LIFT1)
        return true;
    return false;
}
void applyResourceDelta(int& energy,int& time,int deltaEnergy,int deltaTime){
    energy=clampInt(energy+deltaEnergy,0,MAX_ENERGY);
    time=clampInt(time+deltaTime,0,MAX_TIME);
}
void applyScoreDelta(int& score,int deltaScore){
    score+=deltaScore;
    if(score<0)score=0;
}
bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY],int inventoryCount,char itemSymbol){
    if(inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY)return false;
    for(int i=0;i<inventoryCount;i++){
        if(inventory[i]==itemSymbol)return true;
    }
    return false;
}
bool addItem(char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,char itemSymbol){
    if(inventoryCount<0||inventoryCount>=MAX_INVENTORY_CAPACITY)return false;
    inventory[inventoryCount++]=itemSymbol;
    return true;
}
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY],int& inventoryCount,char itemSymbol){
    if(inventoryCount<0||inventoryCount>MAX_INVENTORY_CAPACITY)return false;
    for(int i=0;i<inventoryCount;i++){
        if(inventory[i]==itemSymbol){
            for(int j=i;j<inventoryCount-1;j++)
                inventory[j]=inventory[j+1];
            inventory[--inventoryCount]=ENT_NONE;
            return true;
        }
    }
    return false;
}
