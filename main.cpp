#include "BTreeIndex.cpp"
#include "bits/stdc++.h"

using namespace std;

int main(){
    BTreeIndex::CreateIndexFileFile("index.bin", 10, 5);

    int current_order = BTreeIndex::getM();
    cout<<current_order;
    return 0;
}