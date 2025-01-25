#include "BTreeIndex.h"
#include "BTreeIndex.cpp"
using namespace std;

int main() {
    const char* filename = "index.bin";
    const int initialRecords = 10;
    const int m = 5;

    try {
        BTreeIndex index;

        index.CreateIndexFile("BTreeIndex.txt", initialRecords, m);
        cout << "=== Initial File Created ===" << endl;
        index.DisplayIndexFileContent("BTreeIndex.txt");

        vector<pair<int, int>> insertions = {
                {3, 12}, {7, 24}, {10, 48}, {24, 60}, {14, 72},
                {19, 84}, {30, 96}, {15, 108}, {1, 120}, {5, 132},
                {2, 144}, {8, 156}, {9, 168}, {6, 180}, {11, 192},
                {12, 204}, {17, 216}, {18, 228}, {32, 240}
        };

        cout << "\n=== Performing Insertions ===" << endl;
        for (const auto& [id, ref] : insertions) {
            int result = index.InsertNewRecordAtIndex(id, ref);
            if (result == -1) {
                cout << "Insertion failed for Record ID: " << id << endl;
            } else {
                cout << "Insertion successful for Record ID: " << id << endl;
                index.DisplayIndexFileContent("BTreeIndex.txt");
            }
        }
        index.DisplayIndexFileContent("BTreeIndex.txt");

        cout << "\n=== Testing Searches ===" << endl;
        vector<int> searchTests = {3, 10, 15, 99};
        for (int id : searchTests) {
            int ref = index.SearchARecord("BTreeIndex.txt", id);
            cout << "Search for " << id << ": "
                 << (ref != -1 ? "Found (Ref: " + to_string(ref) + ")" : "Not found")
                 << endl;
        }

        vector<int> deletions = {10, 9, 8};
        cout << "\n=== Performing Deletions ===" << endl;
        for (int id : deletions) {
            index.DeleteRecordFromIndex("BTreeIndex.txt", id, m);
            cout << "After deleting " << id << ":" << endl;
            index.DisplayIndexFileContent("BTreeIndex.txt");
        }

        cout << "\n=== Final State ===" << endl;
        index.DisplayIndexFileContent("BTreeIndex.txt");

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    /*
     BTreeIndex bTreeIndex;
    cout << "1. Create Index File" << endl;
    cout << "Enter the m records: ";
    int M;
    cin >> M;
    bTreeIndex.CreateIndexFile("BTreeIndex.txt", M * 2, M);
    cout << "\nIndex file created successfully." << endl;
    bTreeIndex.run();*/





    /////////////////for test insert code/////////////////////////
//    bTreeIndex.CreateIndexFile("BTreeIndex.txt", 10, 5);
//    bTreeIndex.InsertNewRecordAtIndex(3, 12);
//    bTreeIndex.InsertNewRecordAtIndex(7, 24);
//    bTreeIndex.InsertNewRecordAtIndex(10, 48);
//    bTreeIndex.InsertNewRecordAtIndex(24, 60);
//    bTreeIndex.InsertNewRecordAtIndex(14, 72);
//    bTreeIndex.InsertNewRecordAtIndex(19, 84);
//    bTreeIndex.InsertNewRecordAtIndex(30, 96);
//    bTreeIndex.InsertNewRecordAtIndex(15, 108);
//    bTreeIndex.InsertNewRecordAtIndex(1, 120);
//    bTreeIndex.InsertNewRecordAtIndex(5, 132);
//    bTreeIndex.InsertNewRecordAtIndex(2, 144);
//    bTreeIndex.InsertNewRecordAtIndex(8, 156);
//    bTreeIndex.InsertNewRecordAtIndex(9, 168);
//    bTreeIndex.InsertNewRecordAtIndex(6, 180);
//    bTreeIndex.InsertNewRecordAtIndex(11, 192);
//    bTreeIndex.InsertNewRecordAtIndex(12, 204);
//    bTreeIndex.InsertNewRecordAtIndex(17, 216);
//    bTreeIndex.InsertNewRecordAtIndex(18, 228);
//    bTreeIndex.InsertNewRecordAtIndex(32, 240);


    return 0;
}