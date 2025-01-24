#include "BTreeIndex.h"
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

int BTreeIndex::m = 0;

void BTreeIndex::setM(int order) {
    BTreeIndex::m = order;
}

int BTreeIndex::getM() {
    return BTreeIndex::m;
}

// Read node from file at 'index'
void BTreeIndex::readNode(fstream& file, int index, Node& node) {
    file.seekg(index * (sizeof(int) * (2 + 2 * m))); // Calculate node position
    file.read(reinterpret_cast<char*>(&node.is_leaf), sizeof(int));
    file.read(reinterpret_cast<char*>(&node.next_free), sizeof(int));

    // Read keys and references
    for(int i = 0; i < m; i++) {
        file.read(reinterpret_cast<char*>(&node.keys[i]), sizeof(int));
        file.read(reinterpret_cast<char*>(&node.refs[i]), sizeof(int));
    }
}

// Write node to file at 'index'
void BTreeIndex::writeNode(fstream& file, int index, Node& node) {
    file.seekp(index * (sizeof(int) * (2 + 2 * m)));
    file.write(reinterpret_cast<const char*>(&node.is_leaf), sizeof(int));
    file.write(reinterpret_cast<const char*>(&node.next_free), sizeof(int));

    for(int i = 0; i < m; i++) {
        file.write(reinterpret_cast<const char*>(&node.keys[i]), sizeof(int));
        file.write(reinterpret_cast<const char*>(&node.refs[i]), sizeof(int));
    }
    file.flush();
}

// Allocate a free node from the free list
int BTreeIndex::allocateFreeNode(fstream& file) {
    Node headerNode;
    readNode(file, 0, headerNode);

    if(headerNode.next_free == -1) return -1; // No free nodes

    Node freeNode;
    readNode(file, headerNode.next_free, freeNode);

    // Update header's next free
    int allocatedIndex = headerNode.next_free;
    headerNode.next_free = freeNode.next_free;
    writeNode(file, 0, headerNode);

    return allocatedIndex;
}

// Add node back to free list
void BTreeIndex::freeNode(fstream& file, int index) {
    Node headerNode;
    readNode(file, 0, headerNode);

    Node nodeToFree;
    nodeToFree.next_free = headerNode.next_free;
    nodeToFree.is_leaf = -1; // Mark as free

    // Update header to point to newly freed node
    headerNode.next_free = index;
    writeNode(file, 0, headerNode);
    writeNode(file, index, nodeToFree);
}

// Find insertion position in a node
int BTreeIndex::findKeyIndex(const Node& node, int RecordID) {
    return lower_bound(node.keys.begin(), node.keys.end(), RecordID) - node.keys.begin();
}

// Split node and update parent (de simplified version nb2a n3delaha b3deen)
void BTreeIndex::split(fstream& file, int nodeIndex, int parentIndex) {
    Node oldNode, parentNode;
    readNode(file, nodeIndex, oldNode);
    readNode(file, parentIndex, parentNode);

    // Create new node
    int newIndex = allocateFreeNode(file);
    Node newNode;
    newNode.is_leaf = oldNode.is_leaf;

    // Split keys and references
    int splitPos = m/2;
    copy(oldNode.keys.begin() + splitPos, oldNode.keys.end(), newNode.keys.begin());
    copy(oldNode.refs.begin() + splitPos, oldNode.refs.end(), newNode.refs.begin());

    // Update parent
    int promotedKey = oldNode.keys[splitPos];
    int insertPos = findKeyIndex(parentNode, promotedKey);

    // Shift parent keys/references
    for(int i = m-1; i > insertPos; i--) {
        parentNode.keys[i] = parentNode.keys[i-1];
        parentNode.refs[i] = parentNode.refs[i-1];
    }

    parentNode.keys[insertPos] = promotedKey;
    parentNode.refs[insertPos] = newIndex;

    // Write changes
    writeNode(file, nodeIndex, oldNode);
    writeNode(file, newIndex, newNode);
    writeNode(file, parentIndex, parentNode);
}

void BTreeIndex::CreateIndexFileFile(const char* filename, int numberOfRecords, int m) {
    setM(m);

    fstream file(filename, ios::binary | ios::out);
    if (!file.is_open()) {
        throw runtime_error("Failed to create index file");
    }

    // Initialize header node (index 0)
    Node headerNode;
    headerNode.is_leaf = -1;
    headerNode.next_free = (numberOfRecords > 1) ? 1 : -1; // First free node

    // Initialize all keys and refs to -1
    fill(headerNode.keys.begin(), headerNode.keys.end(), -1);
    fill(headerNode.refs.begin(), headerNode.refs.end(), -1);

    writeNode(file, 0, headerNode);

    // Initialize free nodes (index 1 to n-1)
    for (int i = 1; i < numberOfRecords; i++) {
        Node freeNode;
        freeNode.is_leaf = -1;
        freeNode.next_free = (i < numberOfRecords - 1) ? i + 1 : -1;

        fill(freeNode.keys.begin(), freeNode.keys.end(), -1);
        fill(freeNode.refs.begin(), freeNode.refs.end(), -1);

        writeNode(file, i, freeNode);
    }

    file.close();
}
