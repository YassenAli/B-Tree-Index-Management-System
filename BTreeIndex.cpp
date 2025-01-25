#include "BTreeIndex.h"
#include <fstream>
#include <vector>
#include <tuple>
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

////////////////////////////////////////////////// Deletion //////////////////////////////////////////////////
void BTreeIndex::DeleteRecordFromIndex(const char* filename, int RecordID) {
    if (SearchARecord(filename, RecordID) == -1) return;

    fstream file(filename, ios::binary | ios::in | ios::out);
    vector<pair<int, int>> parentStack; // (nodeIndex, keyPosition)
    int currentIndex = 1; // Root starts at index 1

    // Phase 1: Find the leaf node and record deletion path
    while (true) {
        Node current;
        readNode(file, currentIndex, current);

        if (current.is_leaf == 0) { // Leaf node found
            int pos = findKeyIndex(current, RecordID);
            if (pos == -1 || current.keys[pos] != RecordID) break;

            // Delete the key and reference
            for (int i = pos; i < m - 1; i++) {
                current.keys[i] = current.keys[i + 1];
                current.refs[i] = current.refs[i + 1];
            }
            current.keys[m - 1] = -1;
            current.refs[m - 1] = -1;
            writeNode(file, currentIndex, current);

            // Check underflow (minimum keys = ceil(m/2) - 1)
            int keyCount = count_if(current.keys.begin(), current.keys.end(),
                                         [](int k) { return k != -1; });
            if (keyCount < (m + 1) / 2 - 1) {
                handleUnderflow(file, currentIndex, parentStack);
            }
            break;
        } else { // Non-leaf node
            int pos = findKeyIndex(current, RecordID);
            parentStack.emplace_back(currentIndex, pos);
            currentIndex = current.refs[pos];
        }
    }
    file.close();
}

// Helper: Handle node underflow
void BTreeIndex::handleUnderflow(fstream& file, int nodeIndex,
                                 vector<pair<int, int>>& parentStack) {
    if (parentStack.empty()) return; // Root underflow

    Node node, parent, sibling;
    readNode(file, nodeIndex, node);
    auto [parentIndex, keyPos] = parentStack.back();
    readNode(file, parentIndex, parent);

    // Try left sibling first
    if (keyPos > 0) {
        int leftSiblingIndex = parent.refs[keyPos - 1];
        readNode(file, leftSiblingIndex, sibling);
        if (canRedistribute(sibling)) {
            redistributeFromLeft(file, nodeIndex, leftSiblingIndex,
                                 parentIndex, keyPos);
            return;
        }
    }

    // Try right sibling
    if (keyPos < m - 1) {
        int rightSiblingIndex = parent.refs[keyPos + 1];
        readNode(file, rightSiblingIndex, sibling);
        if (canRedistribute(sibling)) {
            redistributeFromRight(file, nodeIndex, rightSiblingIndex,
                                  parentIndex, keyPos);
            return;
        }
    }

    // Must merge
    if (keyPos > 0) {
        mergeWithLeft(file, nodeIndex, parentIndex, keyPos);
    } else {
        mergeWithRight(file, nodeIndex, parentIndex, keyPos);
    }
}

// Check if sibling has extra keys for redistribution
bool BTreeIndex::canRedistribute(const Node& node) {
    int keyCount = count_if(node.keys.begin(), node.keys.end(),
                                 [](int k) { return k != -1; });
    return keyCount > (m + 1) / 2 - 1;
}

// Redistribution from left sibling
void BTreeIndex::redistributeFromLeft(fstream& file, int nodeIndex,
                                      int leftSiblingIndex, int parentIndex,
                                      int keyPos) {
    Node node, left, parent;
    readNode(file, nodeIndex, node);
    readNode(file, leftSiblingIndex, left);
    readNode(file, parentIndex, parent);

    // Move last key from left sibling to node
    int lastKey = left.keys.back();
    int lastRef = left.refs.back();
    left.keys.pop_back();
    left.refs.pop_back();
    left.keys.insert(left.keys.begin(), -1);
    left.refs.insert(left.refs.begin(), -1);

    // Update parent key
    parent.keys[keyPos] = lastKey;

    // Insert to node
    node.keys.insert(node.keys.begin(), lastKey);
    node.refs.insert(node.refs.begin(), lastRef);
    node.keys.pop_back();
    node.refs.pop_back();

    writeNode(file, leftSiblingIndex, left);
    writeNode(file, nodeIndex, node);
    writeNode(file, parentIndex, parent);
}

// Merge node with left sibling
void BTreeIndex::mergeWithLeft(fstream& file, int nodeIndex,
                               int parentIndex, int keyPos) {
    Node node, left, parent;
    readNode(file, nodeIndex, node);
    readNode(file, parent.refs[keyPos - 1], left);
    readNode(file, parentIndex, parent);

    // Merge keys and refs
    left.keys.insert(left.keys.end(), node.keys.begin(), node.keys.end());
    left.refs.insert(left.refs.end(), node.refs.begin(), node.refs.end());

    // Remove parent key
    parent.keys.erase(parent.keys.begin() + keyPos - 1);
    parent.refs.erase(parent.refs.begin() + keyPos);

    // Free merged node
    freeNode(file, nodeIndex);

    writeNode(file, parentIndex, parent);
    writeNode(file, parent.refs[keyPos - 1], left);

    // Propagate underflow if needed
    if (count(parent.keys.begin(), parent.keys.end(), -1) > m / 2) {
        handleUnderflow(file, parentIndex, parentStack);
    }
}
