#include <bits/stdc++.h>
#include "Threadsafe_list.h"
using namespace std;


int main(){
    Threadsafe::Threadsafe_List<int> List;
    std::vector<std::thread> vecone(10);
    std::vector<std::thread> vectwo(10);
    for(size_t i = 0; i < 10; i++){
        vecone[i] = std::thread(&Threadsafe::Threadsafe_List<int>::push_front, &List, i);
    }
    for(size_t i = 0; i < 10; i++){
        vectwo[i] = std::thread(&Threadsafe::Threadsafe_List<int>::update,
        &List, [](int& T){ return true;} ,[](int& T){return T+=10;});
    }
    std::for_each(vecone.begin(), vecone.end(), std::mem_fn(&std::thread::join));
    std::for_each(vectwo.begin(), vectwo.end(), std::mem_fn(&std::thread::join));
    auto T = List.get_list();
    for(auto x : T){
        cout << x << endl;
    }
    return 0;
}