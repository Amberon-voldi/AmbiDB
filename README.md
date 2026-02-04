# AmbiDB (C++)

AmbiDB is a console-based DBMS project in C++ that demonstrates core DBMS concepts: data storage, retrieval, CRUD operations, and basic integrity checks. Records are persisted to a file to mimic a simple database.

## Features

- Menu-driven console interface
- `Record` structure representing a table row
- CRUD operations: Insert, Display, Search, Update, Delete
- File-based persistence (simple text table)
- Input validation and error handling
- Modular functions for each operation

## Data Model (Single Table)

Each record represents a student-like entity:

- `id` (primary key)
- `name`
- `age`
- `department`
- `email` (unique within the file)

This is enough to demonstrate DBMS fundamentals in a compact, viva-friendly format.

## Build (macOS)

```
clang++ -std=c++17 -O2 src/main.cpp -o ambidb
```

## Steps to Access (Run the CLI)

1. Build the project (command above).
2. Run: `./ambidb`
3. Use the menu to insert/search/update/delete records.
4. Data is saved to `ambidb_records.txt` on exit.

## Menu Options

```
1. Insert record
2. Display all records
3. Search record
4. Update record
5. Delete record
6. Save and Exit
```

## File Storage Format

Records are stored in a text file where each line is a record:

```
id|name|age|department|email
```

The program escapes `|` and `\\` to keep parsing reliable.

## Error Handling

- Input validation for numeric fields
- Email uniqueness check
- Graceful handling of corrupted file rows

## Files

- [src/main.cpp](src/main.cpp) — main menu, CRUD, file I/O

## Notes for Viva

1. Explain how file storage simulates a table.
2. Show CRUD flow and where input validation happens.
3. Discuss primary key (`id`) and uniqueness (`email`).
4. Demonstrate persistence across program runs.



