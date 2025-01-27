#include "BTreeIndex.h"
#include "BTreeIndex.cpp"
#include <iomanip>
using namespace std;

void Test1(){
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
            cout << "\nInserting Record ID: " << id << " with Reference: " << ref << endl;
            index.InsertNewRecordAtIndex(id, ref);
            cout << "Current tree state after insertion:" << endl;
            index.DisplayIndexFileContent("BTreeIndex.txt");
        }

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
        return;
    }
}

int main() {
    cout << "============================================\n";
    cout << "       B-Tree Index Management System       \n";
    cout << "============================================\n";
    cout << "Run assignment example? (y/n): ";
    char ans;
    cin >> ans;

    if (ans == 'y' || ans == 'Y') {
        cout << "\nRunning Assignment Example...\n";
        Test1();
    } else {
        BTreeIndex bTreeIndex;
        int M;

        cout << "\n=== Create Index File ===\n";
        cout << "Enter the value of m (order of B-Tree): ";
        cin >> M;

        bTreeIndex.CreateIndexFile("BTreeIndex.txt", M * 2, M);
        cout << "\n✔ Index file created successfully with " << M * 2 << " records and m = " << M << ".\n";

        bTreeIndex.run();
    }

    cout << "\nThank you for using the B-Tree Index Management System!\n";
    return 0;
}