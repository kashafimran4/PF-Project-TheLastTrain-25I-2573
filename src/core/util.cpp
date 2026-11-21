#include "util.h"
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
using namespace std;
int clampInt(int val,int minVal,int maxVal){
    if(minVal>maxVal){
        int temp=minVal;
        minVal=maxVal;
        maxVal=temp;
    }
    if(val<minVal)return minVal;
    if(val>maxVal)return maxVal;
    return val;
}
void trimWhitespace(char* str){
    if(str==NULL)return;
    int start=0;
    while(str[start]&&isspace((unsigned char)str[start]))start++;
    int end=(int)strlen(str)-1;
    while(end>=start&&isspace((unsigned char)str[end]))end--;
    int length=end-start+1;
    for(int i=0;i<length;i++)str[i]=str[start+i];
    str[length]='\0';
}
bool stringEquals(const char* a,const char* b){
    if(a==NULL||b==NULL)return a==b;
    return strcmp(a,b)==0;
}
void printHeader(const char* title){
    const char* heading=(title==NULL)?"":title;
    cout<<heading<<endl;
}
void clearScreen(){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
