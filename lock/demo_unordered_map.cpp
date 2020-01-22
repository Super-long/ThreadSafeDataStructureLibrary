#include <bits/stdc++.h>
#include "unordered_map.h"
using namespace std;

/* int main(){
    Threadsafe_unordered_map<int, string> mp;
    vector<std::thread> vec(10); //很奇怪 for循环倒着写就会触发terminate
    for(size_t i = 9; i >= 0; --i){
        if(i&1){
            vec[i] = std::thread(&Threadsafe_unordered_map<int, string>::add_or_update,
                &mp, i, "helle world");
                cout <<i <<"world\n";
    }
        else{
            vec[i] = std::thread(&Threadsafe_unordered_map<int, string>::remove,
                &mp, i+1);
                cout <<i<< "thank\n";
        }
    }
    std::for_each(vec.begin(), vec.end(), std::mem_fn(&std::thread::join));
    cout << "oklaaaaaaa\n";
    std::map<int, string> instance = mp.get_standard_map();
    for(auto T : instance){
        cout << T.first << " " << T.second << endl;
    }
    return 0;
} */

int main(){
    Threadsafe_unordered_map<int, string> mp;
    vector<std::thread> vecone(10);
    vector<std::thread> vectwo(8);
    for(size_t i = 0; i <10; ++i){
        vecone[i] = std::thread(&Threadsafe_unordered_map<int, string>::add_or_update,
            &mp, i, "helle world"); 
    }
    for(size_t i = 0; i < 8; ++i){
        vectwo[i] = std::thread(&Threadsafe_unordered_map<int, string>::remove,
            &mp, i); 
    }
    std::for_each(vecone.begin(), vecone.end(), std::mem_fn(&std::thread::join));
    std::for_each(vectwo.begin(), vectwo.end(), std::mem_fn(&std::thread::join));
    std::map<int, string> instance = mp.get_standard_map();
    for(auto T : instance){
        cout << T.first << " " << T.second << endl;
    }
    return 0;
}