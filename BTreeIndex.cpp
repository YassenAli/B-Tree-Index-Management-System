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
////////////////////////////////////////////////// Insertion //////////////////////////////////////////////////
int BTreeIndex::InsertNewRecordAtIndex(const char* filename, int RecordID, int Reference) {
    fstream file(filename, ios::binary | ios::in | ios::out);
    if (!file.is_open()) throw runtime_error("Failed to open index file");

    vector<int> parentStack; // Stores parent node indices
    int currentIndex = 1;    // Start at root
    Node currentNode;
    bool isRoot = true;

    // Traverse to appropriate leaf node
    while (true) {
        readNode(file, currentIndex, currentNode);

        if (currentNode.is_leaf == 0) break; // Found leaf

        // Find insertion path
        int pos = findKeyIndex(currentNode, RecordID);
        parentStack.push_back(currentIndex);
        currentIndex = currentNode.refs[pos];
        isRoot = false;
    }

    // Check for duplicate
    int existingPos = findKeyIndex(currentNode, RecordID);
    if (existingPos < m && currentNode.keys[existingPos] == RecordID) {
        file.close();
        return -1;
    }

    // Insert in sorted position
    int insertPos = findKeyIndex(currentNode, RecordID);
    for (int i = m-1; i > insertPos; i--) {
        currentNode.keys[i] = currentNode.keys[i-1];
        currentNode.refs[i] = currentNode.refs[i-1];
    }
    currentNode.keys[insertPos] = RecordID;
    currentNode.refs[insertPos] = Reference;
    writeNode(file, currentIndex, currentNode);

    // Handle overflow
    int keysCount = count_if(currentNode.keys.begin(), currentNode.keys.end(),
                             [](int k){ return k != -1; });

    while (keysCount > m-1) { // Needs split
        int newIndex = allocateFreeNode(file);
        if (newIndex == -1) {
            file.close();
            return -1; // No space
        }

        Node newNode;
        int splitPos = m/2;
        int promotedKey = currentNode.keys[splitPos];

        // Split keys and refs
        copy(currentNode.keys.begin() + splitPos + 1, currentNode.keys.end(), newNode.keys.begin());
        copy(currentNode.refs.begin() + splitPos + 1, currentNode.refs.end(), newNode.refs.begin());
        fill(currentNode.keys.begin() + splitPos, currentNode.keys.end(), -1);
        fill(currentNode.refs.begin() + splitPos, currentNode.refs.end(), -1);

        // Set node types
        newNode.is_leaf = currentNode.is_leaf;
        writeNode(file, newIndex, newNode);
        writeNode(file, currentIndex, currentNode);

        // Prepare promoted key for parent
        if (parentStack.empty()) { // Splitting root
            Node newRoot;
            newRoot.is_leaf = 1;
            newRoot.keys[0] = promotedKey;
            newRoot.refs[0] = currentIndex;
            newRoot.refs[1] = newIndex;
            writeNode(file, 1, newRoot);
            break;
        } else { // Update parent
            int parentIndex = parentStack.back();
            parentStack.pop_back();
            Node parentNode;
            readNode(file, parentIndex, parentNode);

            // Find insert position in parent
            int parentPos = findKeyIndex(parentNode, promotedKey);
            for (int i = m-1; i > parentPos; i--) {
                parentNode.keys[i] = parentNode.keys[i-1];
                parentNode.refs[i+1] = parentNode.refs[i];
            }
            parentNode.keys[parentPos] = promotedKey;
            parentNode.refs[parentPos+1] = newIndex;
            writeNode(file, parentIndex, parentNode);

            // Move up to check parent
            currentIndex = parentIndex;
            currentNode = parentNode;
            keysCount = count_if(currentNode.keys.begin(), currentNode.keys.end(),
                                 [](int k){ return k != -1; });
        }
    }

    file.close();
    return currentIndex;
}

////////////////////////////////////////////////// Deletion //////////////////////////////////////////////////
void BTreeIndex::DeleteRecordFromIndex(const char* filename, int RecordID) {
    fstream file(filename, ios::binary | ios::in | ios::out);
    if (!file.is_open()) throw runtime_error("File open failed");

    vector<tuple<int, int, int>> path; // (currentIndex, parentIndex, childPos)
    int current = 1; // Root node
    int parent = -1, childPos = -1;
    Node current_node;

    // Phase 1: Find leaf node containing the key
    while (true) {
        readNode(file, current, current_node);
        if (current_node.is_leaf == 0) break;

        int pos = findKeyIndex(current_node, RecordID);
        path.emplace_back(current, parent, pos);
        parent = current;
        current = current_node.refs[pos];
    }

    // Check if key exists
    int pos = findKeyIndex(current_node, RecordID);
    if (pos >= m || current_node.keys[pos] != RecordID) {
        file.close();
        return;
    }

    // Phase 2: Delete from leaf node
    for (int i = pos; i < m-1; i++) {
        current_node.keys[i] = current_node.keys[i+1];
        current_node.refs[i] = current_node.refs[i+1];
    }
    current_node.keys[m-1] = current_node.refs[m-1] = -1;
    writeNode(file, current, current_node);

    // Phase 3: Handle underflow
    int min_keys = (m % 2 == 0) ? (m/2 - 1) : (m/2);
    int key_count = count_if(current_node.keys.begin(), current_node.keys.end(),
                                  [](int k){ return k != -1; });

    while (key_count < min_keys && !path.empty()) {
        auto [nodeIndex, parentIndex, childPos] = path.back();
        path.pop_back();

        Node parentNode;
        readNode(file, parentIndex, parentNode);

        // Try borrow from left sibling
        if (childPos > 0) {
            int leftSibling = parentNode.refs[childPos-1];
            Node leftNode;
            readNode(file, leftSibling, leftNode);

            int left_keys = count_if(leftNode.keys.begin(), leftNode.keys.end(),
                                          [](int k){ return k != -1; });
            if (left_keys > min_keys) {
                // Borrow logic
                // ... (implementation omitted for brevity)
                writeNode(file, nodeIndex, current_node);
                writeNode(file, leftSibling, leftNode);
                writeNode(file, parentIndex, parentNode);
                return;
            }
        }

        // Try borrow from right sibling
        if (childPos < m-1 && parentNode.refs[childPos+1] != -1) {
            int rightSibling = parentNode.refs[childPos+1];
            Node rightNode;
            readNode(file, rightSibling, rightNode);

            // Similar borrow logic
            // ... 
            return;
        }

        // Merge with sibling
        if (childPos > 0) { // Merge with left
            merge(file, nodeIndex, parentIndex);
        } else { // Merge with right
            merge(file, parentNode.refs[childPos+1], parentIndex);
        }

        // Update current node to parent
        current = parentIndex;
        readNode(file, current, current_node);
        key_count = count_if(current_node.keys.begin(), current_node.keys.end(),
                                  [](int k){ return k != -1; });
    }

    // Handle root underflow
    if (key_count == 0 && current == 1) {
        freeNode(file, current);
    }

    file.close();
}