# B-Tree Index Management System

This repository contains Assignment 2 for the *IS321 - File Management and Processing course at Cairo University - Faculty of Computer and Artificial Intelligence*. The project was developed by a team of 5 members as part of our coursework.

## Description

The goal of this project is to implement a **B-Tree index** system in C++ to manage fixed-length records in a binary file. The implementation provides functionality to create, store, and manipulate records efficiently while adhering to B-Tree properties. 

### Key Features

#### B-Tree Description
The B-Tree is an index structure that organizes records for efficient search, insertion, and deletion operations. Each node in the B-Tree contains:
- **m descendant records** (IDs of records and references to their actual data).
- **1 status integer** indicating whether the node is a leaf (`0`) or a non-leaf (`1`).

##### Binary File Organization
- **Node 0**: Always stores the index of the next free node. This node is not used for data storage.
- **Empty nodes**: Linked together to form a free list, simplifying the management of available space.
- **Root node**: The first data node (index 1) is always designated as the root.

#### Supported Operations
1. **Creation**
   - Initialize the binary file with a specified number of records (`n`) and branching factor (`m`).
   
2. **Insertion**
   - Add new records to the B-Tree, splitting nodes as necessary while maintaining B-Tree properties.
   - Update the free list after each insertion.

3. **Deletion**
   - Remove records from the B-Tree, merging or redistributing keys between nodes if needed.

4. **Search**
   - Locate a record by its ID and retrieve its reference to the actual data.

5. **Display**
   - Print the contents of the binary file, showing each node on a separate line.

#### Additional Considerations
- The program maintains an in-memory array of visited nodes during insertion and deletion operations for efficient updates.
- The implementation ensures that all B-Tree properties are upheld after every operation.

### Functions Implemented
The following functions are used to manage the B-Tree index:
- `void CreateIndexFileFile(char* filename, int numberOfRecords, int m)`
- `int InsertNewRecordAtIndex(char* filename, int RecordID, int Reference)`
- `void DeleteRecordFromIndex(char* filename, int RecordID)`
- `void DisplayIndexFileContent(char* filename)`
- `int SearchARecord(char* filename, int RecordID)`

## Team Members

This project was completed by a team of 5 students from the Faculty of Computers and Artificial Intelligence at Cairo University. It is part of the coursework for the IS321 - File Management and Processing course.

---

Let us know if you encounter any issues or have suggestions for improvement!
