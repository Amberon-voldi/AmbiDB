#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char *kDataFile = "ambidb_records.txt";

struct Record {
    int id = 0;
    std::string name;
    int age = 0;
    std::string department;
    std::string email;
};

// Escape delimiters so each line remains a valid row (simulates tuple storage).
std::string escape_field(const std::string &value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '|') {
            out += "\\|";
        } else if (c == '\\') {
            out += "\\\\";
        } else {
            out += c;
        }
    }
    return out;
}

bool split_fields(const std::string &line, std::vector<std::string> &fields) {
    fields.clear();
    std::string current;
    bool escaped = false;
    for (char c : line) {
        if (escaped) {
            current.push_back(c);
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '|') {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (escaped) {
        return false;
    }
    fields.push_back(current);
    return true;
}

// Parse a stored row back into a Record (simulates record retrieval).
bool parse_record(const std::string &line, Record &rec) {
    std::vector<std::string> fields;
    if (!split_fields(line, fields) || fields.size() != 5) {
        return false;
    }
    try {
        rec.id = std::stoi(fields[0]);
        rec.name = fields[1];
        rec.age = std::stoi(fields[2]);
        rec.department = fields[3];
        rec.email = fields[4];
    } catch (const std::exception &) {
        return false;
    }
    return true;
}

// Serialize a record into a single line (simulates writing a table row).
std::string serialize_record(const Record &rec) {
    std::ostringstream out;
    out << rec.id << '|'
        << escape_field(rec.name) << '|'
        << rec.age << '|'
        << escape_field(rec.department) << '|'
        << escape_field(rec.email);
    return out.str();
}

// Load all rows from the data file into memory (table scan).
bool load_records(std::vector<Record> &records, std::string &error) {
    records.clear();
    std::ifstream in(kDataFile);
    if (!in.is_open()) {
        return true;
    }
    std::string line;
    int line_no = 0;
    while (std::getline(in, line)) {
        ++line_no;
        if (line.empty()) {
            continue;
        }
        Record rec;
        if (!parse_record(line, rec)) {
            error = "Corrupted data at line " + std::to_string(line_no);
            return false;
        }
        records.push_back(rec);
    }
    return true;
}

// Persist all rows to disk (commit for this simple DBMS).
bool save_records(const std::vector<Record> &records, std::string &error) {
    std::ofstream out(kDataFile, std::ios::trunc);
    if (!out) {
        error = "Failed to write to data file.";
        return false;
    }
    for (const auto &rec : records) {
        out << serialize_record(rec) << "\n";
    }
    return true;
}

// Compute next primary key value.
int next_id(const std::vector<Record> &records) {
    int max_id = 0;
    for (const auto &rec : records) {
        if (rec.id > max_id) {
            max_id = rec.id;
        }
    }
    return max_id + 1;
}

int read_int(const std::string &prompt, int min_value, int max_value) {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value) {
            if (value >= min_value && value <= max_value) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cout << "Value must be between " << min_value << " and " << max_value
                      << ".\n";
        } else {
            std::cout << "Invalid number. Try again.\n";
            std::cin.clear();
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::string read_line(const std::string &prompt, bool required = true) {
    while (true) {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        if (!required || !value.empty()) {
            return value;
        }
        std::cout << "Input cannot be empty.\n";
    }
}

void print_record(const Record &rec) {
    std::cout << std::left << std::setw(5) << rec.id << std::setw(18) << rec.name
              << std::setw(6) << rec.age << std::setw(16) << rec.department
              << rec.email << "\n";
}

void display_records(const std::vector<Record> &records) {
    if (records.empty()) {
        std::cout << "No records found.\n";
        return;
    }
    std::cout << std::left << std::setw(5) << "ID" << std::setw(18) << "Name"
              << std::setw(6) << "Age" << std::setw(16) << "Department"
              << "Email\n";
    std::cout << "------------------------------------------------------------\n";
    for (const auto &rec : records) {
        print_record(rec);
    }
}

int find_record_index(const std::vector<Record> &records, int id) {
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool email_exists(const std::vector<Record> &records, const std::string &email,
                  int ignore_id = -1) {
    for (const auto &rec : records) {
        if (rec.email == email && rec.id != ignore_id) {
            return true;
        }
    }
    return false;
}

// CREATE: insert a new row.
void insert_record(std::vector<Record> &records) {
    Record rec;
    rec.id = next_id(records);
    rec.name = read_line("Enter name: ");
    rec.age = read_int("Enter age (16-80): ", 16, 80);
    rec.department = read_line("Enter department: ");
    while (true) {
        rec.email = read_line("Enter email: ");
        if (!email_exists(records, rec.email)) {
            break;
        }
        std::cout << "Email already exists. Use a unique email.\n";
    }
    records.push_back(rec);
    std::cout << "Record inserted with ID " << rec.id << ".\n";
}

// READ: find a row by primary key.
void search_record(const std::vector<Record> &records) {
    int id = read_int("Enter record ID to search: ", 1, 1000000);
    int index = find_record_index(records, id);
    if (index == -1) {
        std::cout << "Record not found.\n";
        return;
    }
    std::cout << "Record found:\n";
    std::cout << std::left << std::setw(5) << "ID" << std::setw(18) << "Name"
              << std::setw(6) << "Age" << std::setw(16) << "Department"
              << "Email\n";
    print_record(records[static_cast<size_t>(index)]);
}

// UPDATE: modify an existing row.
void update_record(std::vector<Record> &records) {
    int id = read_int("Enter record ID to update: ", 1, 1000000);
    int index = find_record_index(records, id);
    if (index == -1) {
        std::cout << "Record not found.\n";
        return;
    }
    Record &rec = records[static_cast<size_t>(index)];
    std::cout << "Updating record (leave empty to keep existing value).\n";
    std::string name = read_line("Enter name [" + rec.name + "]: ", false);
    if (!name.empty()) {
        rec.name = name;
    }
    std::string age_input = read_line("Enter age [" + std::to_string(rec.age) + "]: ", false);
    if (!age_input.empty()) {
        try {
            int age = std::stoi(age_input);
            if (age >= 16 && age <= 80) {
                rec.age = age;
            } else {
                std::cout << "Age out of range, keeping previous value.\n";
            }
        } catch (const std::exception &) {
            std::cout << "Invalid age, keeping previous value.\n";
        }
    }
    std::string dept = read_line("Enter department [" + rec.department + "]: ", false);
    if (!dept.empty()) {
        rec.department = dept;
    }
    while (true) {
        std::string email = read_line("Enter email [" + rec.email + "]: ", false);
        if (email.empty()) {
            break;
        }
        if (!email_exists(records, email, rec.id)) {
            rec.email = email;
            break;
        }
        std::cout << "Email already exists. Use a unique email.\n";
    }
    std::cout << "Record updated.\n";
}

// DELETE: remove a row by primary key.
void delete_record(std::vector<Record> &records) {
    int id = read_int("Enter record ID to delete: ", 1, 1000000);
    int index = find_record_index(records, id);
    if (index == -1) {
        std::cout << "Record not found.\n";
        return;
    }
    records.erase(records.begin() + index);
    std::cout << "Record deleted.\n";
}

void print_menu() {
    std::cout << "\nAmbiDB - Console DBMS\n"
              << "1. Insert record\n"
              << "2. Display all records\n"
              << "3. Search record\n"
              << "4. Update record\n"
              << "5. Delete record\n"
              << "6. Save and Exit\n";
}

}  // namespace

int main() {
    std::vector<Record> records;
    std::string error;
    if (!load_records(records, error)) {
        std::cerr << "Error loading data: " << error << "\n";
        return 1;
    }

    bool running = true;
    while (running) {
        print_menu();
        int choice = read_int("Enter choice: ", 1, 6);
        switch (choice) {
            case 1:
                insert_record(records);
                break;
            case 2:
                display_records(records);
                break;
            case 3:
                search_record(records);
                break;
            case 4:
                update_record(records);
                break;
            case 5:
                delete_record(records);
                break;
            case 6:
                if (!save_records(records, error)) {
                    std::cerr << "Error saving data: " << error << "\n";
                    return 1;
                }
                std::cout << "Data saved to " << kDataFile << ". Goodbye.\n";
                running = false;
                break;
            default:
                break;
        }
    }
    return 0;
}
