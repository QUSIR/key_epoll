#include<stdio.h>
#include<iostream>
#include "HotKeyService.h"

using namespace std;
int main(){
    HotKeyService key_server;
    key_server.start();
    cout<<"main"<<endl;
    for(;;){

    }
}
