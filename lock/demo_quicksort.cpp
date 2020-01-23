#include "Threadpool_quicksort.h"
#include <bits/stdc++.h>
using namespace std;

int main(){
    std::list<int> li = {5,6,9,8,1,3,15,65,89,45,2,1,56,2,78,659,451,632164};
    auto T = parallel_quicksort(li);
    for(auto x : T){
        cout << x << " ";
    }
    return 0;
}