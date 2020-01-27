#include "parallel_find.h"
#include <iostream>
using std::cout;
using std::endl;

int main(){
    std::vector<int> results{5,6,7,8,9,45,12,23,45,656,56,464,6,21621};
    if(parallel_find(results.begin(), results.end(), 233) == results.end())
        cout << "not find.\n";
    else 
        cout << "find!\n";
    return 0;
}