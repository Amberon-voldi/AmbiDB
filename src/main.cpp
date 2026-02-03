#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char *kDefaultDbPath = "ambidb.db";

bool read_file(const std::string &path, std::string &out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    return true;
}

bool exec_sql(sqlite3 *db, const std::string &sql, std::string &error) {
    char *err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        if (err_msg) {
            error = err_msg;
            sqlite3_free(err_msg);
        } else {
            error = sqlite3_errmsg(db);
        }
        return false;
    }
    return true;
}

bool enable_foreign_keys(sqlite3 *db, std::string &error) {
    return exec_sql(db, "PRAGMA foreign_keys = ON;", error);
}

void print_usage() {
    std::cout
        << "AmbiDB - minimal relational DBMS demo\n\n"
        << "Usage:\n"
        << "  ambidb init [db_path]\n"
        << "  ambidb seed [db_path]\n"
        << "  ambidb create-table [db_path] <sql>\n"
        << "  ambidb add-student [db_path] <name> <email> <dept_id>\n"
        << "  ambidb update-student-email [db_path] <student_id> <email>\n"
        << "  ambidb delete-student [db_path] <student_id>\n"
        << "  ambidb list-students [db_path]\n"
        << "  ambidb enroll [db_path] <student_id> <course_id> <semester> <grade>\n"
        << "  ambidb list-enrollments [db_path]\n"
        << "  ambidb query [db_path] <sql>\n";
}

bool open_db(const std::string &path, sqlite3 **db, std::string &error) {
    if (sqlite3_open(path.c_str(), db) != SQLITE_OK) {
        error = sqlite3_errmsg(*db);
        return false;
    }
    if (!enable_foreign_keys(*db, error)) {
        return false;
    }
    return true;
}

bool run_schema(sqlite3 *db, const std::string &schema_path) {
    std::string sql;
    if (!read_file(schema_path, sql)) {
        std::cerr << "Failed to read schema: " << schema_path << "\n";
        return false;
    }
    std::string error;
    if (!exec_sql(db, sql, error)) {
        std::cerr << "Schema error: " << error << "\n";
        return false;
    }
    return true;
}

bool run_seed(sqlite3 *db, const std::string &seed_path) {
    std::string sql;
    if (!read_file(seed_path, sql)) {
        std::cerr << "Failed to read seed data: " << seed_path << "\n";
        return false;
    }
    std::string error;
    if (!exec_sql(db, sql, error)) {
        std::cerr << "Seed error: " << error << "\n";
        return false;
    }
    return true;
}

bool insert_student(sqlite3 *db, const std::string &name, const std::string &email,
                    int dept_id) {
    const char *sql = "INSERT INTO students (name, email, dept_id) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, dept_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insert failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

bool update_student_email(sqlite3 *db, int student_id, const std::string &email) {
    const char *sql = "UPDATE students SET email = ? WHERE student_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, student_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Update failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    if (sqlite3_changes(db) == 0) {
        std::cerr << "No student found with id " << student_id << "\n";
        return false;
    }
    return true;
}

bool delete_student(sqlite3 *db, int student_id) {
    const char *sql = "DELETE FROM students WHERE student_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    sqlite3_bind_int(stmt, 1, student_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Delete failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    if (sqlite3_changes(db) == 0) {
        std::cerr << "No student found with id " << student_id << "\n";
        return false;
    }
    return true;
}

bool list_students(sqlite3 *db) {
    const char *sql =
        "SELECT s.student_id, s.name, s.email, d.name "
        "FROM students s JOIN departments d ON s.dept_id = d.dept_id "
        "ORDER BY s.student_id;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    std::cout << "student_id | name | email | department\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        const unsigned char *email = sqlite3_column_text(stmt, 2);
        const unsigned char *dept = sqlite3_column_text(stmt, 3);
        std::cout << id << " | " << name << " | " << email << " | " << dept << "\n";
    }
    sqlite3_finalize(stmt);
    return true;
}

bool enroll_student(sqlite3 *db, int student_id, int course_id,
                    const std::string &semester, const std::string &grade) {
    const char *sql =
        "INSERT INTO enrollments (student_id, course_id, semester, grade) "
        "VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_int(stmt, 2, course_id);
    sqlite3_bind_text(stmt, 3, semester.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, grade.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Enrollment failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

bool list_enrollments(sqlite3 *db) {
    const char *sql =
        "SELECT e.enrollment_id, s.name, c.code, c.title, e.semester, e.grade "
        "FROM enrollments e "
        "JOIN students s ON e.student_id = s.student_id "
        "JOIN courses c ON e.course_id = c.course_id "
        "ORDER BY e.enrollment_id;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    std::cout << "enrollment_id | student | course | title | semester | grade\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *student = sqlite3_column_text(stmt, 1);
        const unsigned char *code = sqlite3_column_text(stmt, 2);
        const unsigned char *title = sqlite3_column_text(stmt, 3);
        const unsigned char *semester = sqlite3_column_text(stmt, 4);
        const unsigned char *grade = sqlite3_column_text(stmt, 5);
        std::cout << id << " | " << student << " | " << code << " | " << title
                  << " | " << semester << " | " << grade << "\n";
    }
    sqlite3_finalize(stmt);
    return true;
}

bool run_query(sqlite3 *db, const std::string &sql) {
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    int col_count = sqlite3_column_count(stmt);
    for (int i = 0; i < col_count; ++i) {
        std::cout << sqlite3_column_name(stmt, i);
        if (i < col_count - 1) {
            std::cout << " | ";
        }
    }
    if (col_count > 0) {
        std::cout << "\n";
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < col_count; ++i) {
            const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            if (val) {
                std::cout << val;
            } else {
                std::cout << "NULL";
            }
            if (i < col_count - 1) {
                std::cout << " | ";
            }
        }
        if (col_count > 0) {
            std::cout << "\n";
        }
    }

    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Query failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    std::string db_path = kDefaultDbPath;
    int arg_offset = 2;

    if (argc >= 3) {
        db_path = argv[2];
        arg_offset = 3;
    }

    sqlite3 *db = nullptr;
    std::string error;
    if (!open_db(db_path, &db, error)) {
        std::cerr << "Database open failed: " << error << "\n";
        if (db) {
            sqlite3_close(db);
        }
        return 1;
    }

    bool ok = true;

    if (command == "init") {
        ok = run_schema(db, "sql/schema.sql");
    } else if (command == "seed") {
        ok = run_seed(db, "sql/seed.sql");
    } else if (command == "create-table") {
        if (argc < arg_offset + 1) {
            std::cerr << "Missing SQL statement.\n";
            ok = false;
        } else {
            std::string sql = argv[arg_offset];
            std::string err;
            ok = exec_sql(db, sql, err);
            if (!ok) {
                std::cerr << "Create table error: " << err << "\n";
            }
        }
    } else if (command == "add-student") {
        if (argc < arg_offset + 3) {
            std::cerr << "Missing arguments for add-student.\n";
            ok = false;
        } else {
            std::string name = argv[arg_offset];
            std::string email = argv[arg_offset + 1];
            int dept_id = std::stoi(argv[arg_offset + 2]);
            ok = insert_student(db, name, email, dept_id);
        }
    } else if (command == "update-student-email") {
        if (argc < arg_offset + 2) {
            std::cerr << "Missing arguments for update-student-email.\n";
            ok = false;
        } else {
            int student_id = std::stoi(argv[arg_offset]);
            std::string email = argv[arg_offset + 1];
            ok = update_student_email(db, student_id, email);
        }
    } else if (command == "delete-student") {
        if (argc < arg_offset + 1) {
            std::cerr << "Missing arguments for delete-student.\n";
            ok = false;
        } else {
            int student_id = std::stoi(argv[arg_offset]);
            ok = delete_student(db, student_id);
        }
    } else if (command == "list-students") {
        ok = list_students(db);
    } else if (command == "enroll") {
        if (argc < arg_offset + 4) {
            std::cerr << "Missing arguments for enroll.\n";
            ok = false;
        } else {
            int student_id = std::stoi(argv[arg_offset]);
            int course_id = std::stoi(argv[arg_offset + 1]);
            std::string semester = argv[arg_offset + 2];
            std::string grade = argv[arg_offset + 3];
            ok = enroll_student(db, student_id, course_id, semester, grade);
        }
    } else if (command == "list-enrollments") {
        ok = list_enrollments(db);
    } else if (command == "query") {
        if (argc < arg_offset + 1) {
            std::cerr << "Missing SQL query.\n";
            ok = false;
        } else {
            std::string sql = argv[arg_offset];
            ok = run_query(db, sql);
        }
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage();
        ok = false;
    }

    sqlite3_close(db);
    return ok ? 0 : 1;
}
