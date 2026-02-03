# AmbiDB (C++)

AmbiDB is a minimal relational DBMS demonstration built in C++ on top of SQLite. It covers undergraduate DBMS concepts: normalization, schema design, SQL-based CRUD, foreign keys, and joins. The CLI focuses on structured records for a university domain.

## Features

- 3NF schema with proper primary and foreign keys
- SQL-based CRUD operations (students + enrollments)
- Dynamic table creation via CLI
- Sample data and join queries
- Error handling and input validation

## Schema Overview (3NF)

Entities are separated to eliminate redundancy:

- `departments` (master list)
- `students` (references `departments`)
- `instructors` (references `departments`)
- `courses` (references `departments` and `instructors`)
- `enrollments` (junction table for `students` and `courses`)

All non-key attributes depend on the key, the whole key, and nothing but the key, satisfying 3NF.

See [sql/schema.sql](sql/schema.sql) for full DDL.

## Build (macOS)

1. Install SQLite dev headers if needed (Homebrew): `brew install sqlite`
2. Compile:
	 `clang++ -std=c++17 -O2 -I/opt/homebrew/include src/main.cpp -L/opt/homebrew/lib -lsqlite3 -o ambidb`

If SQLite is already in the system include paths, you can omit `-I` and `-L`.

## Steps to Access (Run the CLI)

1. Initialize database and tables:
	 `./ambidb init ambidb.db`
2. Load sample data:
	 `./ambidb seed ambidb.db`
3. Run CRUD commands or ad-hoc SQL queries.

## CLI Commands

```
ambidb init [db_path]
ambidb seed [db_path]
ambidb create-table [db_path] <sql>
ambidb add-student [db_path] <name> <email> <dept_id>
ambidb update-student-email [db_path] <student_id> <email>
ambidb delete-student [db_path] <student_id>
ambidb list-students [db_path]
ambidb enroll [db_path] <student_id> <course_id> <semester> <grade>
ambidb list-enrollments [db_path]
ambidb query [db_path] <sql>
```

## Example Usage

### CRUD

- Create a student:
	`./ambidb add-student ambidb.db "Neel Das" neel@ambidb.edu 1`

- Read (list students with department):
	`./ambidb list-students ambidb.db`

- Update:
	`./ambidb update-student-email ambidb.db 1 newmail@ambidb.edu`

- Delete:
	`./ambidb delete-student ambidb.db 4`

### Dynamic Table Creation

```
./ambidb create-table ambidb.db "CREATE TABLE IF NOT EXISTS clubs (club_id INTEGER PRIMARY KEY, name TEXT UNIQUE);"
```

### Join Queries (Relationships)

List students with the courses they took:

```
./ambidb query ambidb.db "SELECT s.name, c.code, c.title, e.semester, e.grade FROM enrollments e JOIN students s ON e.student_id = s.student_id JOIN courses c ON e.course_id = c.course_id ORDER BY s.name;"
```

List departments and their courses:

```
./ambidb query ambidb.db "SELECT d.name AS department, c.code, c.title FROM courses c JOIN departments d ON c.dept_id = d.dept_id ORDER BY d.name, c.code;"
```

## Error Handling

The CLI validates arguments and reports SQL errors returned by SQLite. Foreign key constraints are enforced via `PRAGMA foreign_keys = ON;`.

## Files

- [src/main.cpp](src/main.cpp) — CLI implementation and SQL CRUD logic
- [sql/schema.sql](sql/schema.sql) — DDL with normalization and constraints
- [sql/seed.sql](sql/seed.sql) — sample data for demo and viva

## Notes for Viva

1. Explain 3NF and how each table removes redundancy.
2. Point out foreign keys and how they enforce referential integrity.
3. Demonstrate join queries and CRUD flow.
4. Show dynamic table creation via CLI.

