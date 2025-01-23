#include "BTreeIndex.h"
#include <fstream>
#include <vector>
#include <algorithm>

void BTreeIndex::setM(const int &m) {
    this.m = m;
}

int BTreeIndex::getM() {
    return m;
}

// Read node from file at 'index'
void BTreeIndex::readNode(std::fstream& file, int index, Node& node) {
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
void BTreeIndex::writeNode(std::fstream& file, int index, Node& node) {
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
int BTreeIndex::allocateFreeNode(std::fstream& file) {
    Node headerNode(m);
    readNode(file, 0, headerNode);

    if(headerNode.next_free == -1) return -1; // No free nodes

    Node freeNode(m);
    readNode(file, headerNode.next_free, freeNode);

    // Update header's next free
    int allocatedIndex = headerNode.next_free;
    headerNode.next_free = freeNode.next_free;
    writeNode(file, 0, headerNode);

    return allocatedIndex;
}

// Add node back to free list
void BTreeIndex::freeNode(std::fstream& file, int index) {
    Node headerNode(m);
    readNode(file, 0, headerNode);

    Node nodeToFree(m);
    nodeToFree.next_free = headerNode.next_free;
    nodeToFree.is_leaf = -1; // Mark as free

    // Update header to point to newly freed node
    headerNode.next_free = index;
    writeNode(file, 0, headerNode);
    writeNode(file, index, nodeToFree);
}

// Find insertion position in a node
int BTreeIndex::findKeyIndex(const Node& node, int RecordID) {
    return std::lower_bound(node.keys.begin(), node.keys.end(), RecordID) - node.keys.begin();
}

// Split node and update parent (de simplified version nb2a n3delaha b3deen)
void BTreeIndex::split(std::fstream& file, int nodeIndex, int parentIndex) {
    Node oldNode(m), parentNode(m);
    readNode(file, nodeIndex, oldNode);
    readNode(file, parentIndex, parentNode);

    // Create new node
    int newIndex = allocateFreeNode(file);
    Node newNode(m);
    newNode.is_leaf = oldNode.is_leaf;

    // Split keys and references
    int splitPos = m/2;
    std::copy(oldNode.keys.begin() + splitPos, oldNode.keys.end(), newNode.keys.begin());
    std::copy(oldNode.refs.begin() + splitPos, oldNode.refs.end(), newNode.refs.begin());

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