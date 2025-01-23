#ifndef B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H
#define B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H

#include <fstream>
#include <vector>

class BTreeIndex {
private:
    struct Node {
        int is_leaf;
        int next_free;
        vector<int> keys;
        vector<int> refs;

        Node(int m) : keys(m, -1), refs(m, -1) {}
    };

    int m;

    // Helper functions
    static int getM();
    static void setM();
    static void readNode(std::fstream& file, int index, Node& node, int m);
    static void writeNode(std::fstream& file, int index, Node& node, int m);
    static int allocateFreeNode(std::fstream& file, int m);
    static void freeNode(std::fstream& file, int index, int m);
    static void split(std::fstream& file, int nodeIndex, int parentIndex, int m);
    static void merge(std::fstream& file, int nodeIndex, int parentIndex, int m);
    static int findKeyIndex(const Node& node, int RecordID, int m);
    static int searchInNode(std::fstream& file, int nodeIndex, int RecordID, int m);
    static void updateParent(std::fstream& file, int parentIndex, int newKey, int newRef, int m);

public:
    static void CreateIndexFileFile(const char* filename, int numberOfRecords, int m);
    static int InsertNewRecordAtIndex(const char* filename, int RecordID, int Reference);
    static void DeleteRecordFromIndex(const char* filename, int RecordID);
    static void DisplayIndexFileContent(const char* filename);
    static int SearchARecord(const char* filename, int RecordID);
};

#endif //B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H