PRAGMA foreign_keys = ON;

INSERT INTO departments (name) VALUES
    ('Computer Science'),
    ('Mathematics'),
    ('Electronics');

INSERT INTO instructors (name, email, dept_id) VALUES
    ('Dr. Rao', 'rao@ambidb.edu', 1),
    ('Dr. Sen', 'sen@ambidb.edu', 2),
    ('Dr. Mehta', 'mehta@ambidb.edu', 3);

INSERT INTO courses (code, title, credits, dept_id, instructor_id) VALUES
    ('CS101', 'Data Structures', 4, 1, 1),
    ('CS102', 'Database Systems', 4, 1, 1),
    ('MA101', 'Discrete Mathematics', 3, 2, 2),
    ('EC101', 'Digital Electronics', 3, 3, 3);

INSERT INTO students (name, email, dept_id) VALUES
    ('Asha Nair', 'asha@ambidb.edu', 1),
    ('Rohan Verma', 'rohan@ambidb.edu', 2),
    ('Meera Iyer', 'meera@ambidb.edu', 1),
    ('Kunal Shah', 'kunal@ambidb.edu', 3);

INSERT INTO enrollments (student_id, course_id, semester, grade) VALUES
    (1, 1, '2025-Fall', 'A'),
    (1, 2, '2025-Fall', 'A-'),
    (2, 3, '2025-Fall', 'B+'),
    (3, 2, '2025-Fall', 'A'),
    (4, 4, '2025-Fall', 'B');
