#ifndef B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H
#define B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H

#include <fstream>
#include <vector>
using namespace std;

class BTreeIndex {
private:
    struct Node {
        int is_leaf;
        int next_free;
        vector<int> keys;
        vector<int> refs;

        Node() : keys(BTreeIndex::getM(), -1),
                      refs(BTreeIndex::getM(), -1) {}
    };

    static int m;

    // Helper functions
    static void readNode(fstream& file, int index, Node& node);
    static void writeNode(fstream& file, int index, Node& node);
    static int allocateFreeNode(fstream& file);
    static void freeNode(fstream& file, int index);
    static void split(fstream& file, int nodeIndex, int parentIndex);
    static void merge(fstream& file, int nodeIndex, int parentIndex);
    static int findKeyIndex(const Node& node, int RecordID);
    static int searchInNode(fstream& file, int nodeIndex, int RecordID);
    static void updateParent(fstream& file, int parentIndex, int newKey, int newRef);

public:
    static void setM(int order);
    static int getM();
    static void CreateIndexFileFile(const char* filename, int numberOfRecords, int m);
    static int InsertNewRecordAtIndex(const char* filename, int RecordID, int Reference);
    void DeleteRecordFromIndex(const char* filename, int RecordID);
    static void DisplayIndexFileContent(const char* filename);
    static int SearchARecord(const char* filename, int RecordID);

    void handleUnderflow(fstream &file, int nodeIndex, vector<pair<int, int>> &parentStack);

    bool canRedistribute(const Node &node);

    void redistributeFromLeft(fstream &file, int nodeIndex, int leftSiblingIndex, int parentIndex, int keyPos);

    void mergeWithLeft(fstream &file, int nodeIndex, int parentIndex, int keyPos);
};

#endif //B_TREE_INDEX_MANAGEMENT_SYSTEM_BTREEINDEX_H