#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

// ---------------- DB wrapper ----------------
class DB {
private:
    sqlite3* db;
public:
    DB(const string& name) {
        if (sqlite3_open(name.c_str(), &db)) {
            cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
            db = nullptr;
        }
    }
    ~DB() {
        if (db) sqlite3_close(db);
    }
    sqlite3* getDB() { return db; }

    bool execute(const string& q) {
        char* err = nullptr;
        if (!db) return false;
        if (sqlite3_exec(db, q.c_str(), 0, 0, &err) != SQLITE_OK) {
            cerr << "SQL Error: " << (err ? err : "unknown") << endl;
            if (err) sqlite3_free(err);
            return false;
        }
        return true;
    }
};

// ---------------- Model: User ----------------
class User {
public:
    int id;
    string name;
    int age;

    User(): id(0), name(""), age(0) {}
    User(int i, string n, int a) : id(i), name(n), age(a) {}
    User(string n, int a) : id(0), name(n), age(a) {}

    static string tableName() { return "users"; }
    static string primaryKey() { return "id"; }

    static string createTableSQL() {
        return "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INT);";
    }

    string insertSQL() const {
        return "INSERT INTO users(name, age) VALUES(?, ?);";
    }

    string updateSQL() const {
        return "UPDATE users SET name = ?, age = ? WHERE id = ?;";
    }

    static User fromRow(sqlite3_stmt* stmt) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* txt = sqlite3_column_text(stmt, 1);
        string name = txt ? reinterpret_cast<const char*>(txt) : "";
        int age = sqlite3_column_int(stmt, 2);
        return User(id, name, age);
    }
};

// ---------------- Model: Book (One-to-Many example) ----------------
class Book {
public:
    int id;
    string title;
    int user_id;

    Book(): id(0), title(""), user_id(0) {}
    Book(int i, string t, int uid): id(i), title(t), user_id(uid) {}
    Book(string t, int uid): id(0), title(t), user_id(uid) {}

    static string tableName() { return "books"; }
    static string primaryKey() { return "id"; }
    static string createTableSQL() {
        return "CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT, user_id INT, "
               "FOREIGN KEY(user_id) REFERENCES users(id));";
    }
    string insertSQL() const {
        return "INSERT INTO books(title, user_id) VALUES(?, ?);";
    }
    static Book fromRow(sqlite3_stmt* stmt) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* t = sqlite3_column_text(stmt, 1);
        string title = t ? reinterpret_cast<const char*>(t) : "";
        int uid = sqlite3_column_int(stmt, 2);
        return Book(id, title, uid);
    }
};

// ---------------- Generic Repository ----------------
template <typename T>
class Repository {
private:
    DB& db;
public:
    Repository(DB& database) : db(database) {
        db.execute(T::createTableSQL());
    }

    bool save(const T& obj) {
        sqlite3_stmt* stmt = nullptr;
        string q = obj.insertSQL();
        if (sqlite3_prepare_v2(db.getDB(), q.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Prepare failed: " << sqlite3_errmsg(db.getDB()) << endl;
            return false;
        }
        bool ok = bindAndExecuteInsert(obj, stmt);
        sqlite3_finalize(stmt);
        return ok;
    }

    vector<T> loadAll() {
        vector<T> items;
        string q = "SELECT * FROM " + T::tableName() + ";";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db.getDB(), q.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                items.push_back(T::fromRow(stmt));
            }
        } else {
            cerr << "Prepare failed: " << sqlite3_errmsg(db.getDB()) << endl;
        }
        sqlite3_finalize(stmt);
        return items;
    }

    T findById(int id) {
        T result;
        string q = "SELECT * FROM " + T::tableName() + " WHERE " + T::primaryKey() + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db.getDB(), q.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = T::fromRow(stmt);
            }
        } else {
            cerr << "Prepare failed: " << sqlite3_errmsg(db.getDB()) << endl;
        }
        sqlite3_finalize(stmt);
        return result;
    }

    bool update(const T& obj) {
        sqlite3_stmt* stmt = nullptr;
        string q = obj.updateSQL();
        if (sqlite3_prepare_v2(db.getDB(), q.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Prepare failed: " << sqlite3_errmsg(db.getDB()) << endl;
            return false;
        }
        bool ok = bindAndExecuteUpdate(obj, stmt);
        sqlite3_finalize(stmt);
        return ok;
    }

    bool deleteById(int id) {
        string q = "DELETE FROM " + T::tableName() + " WHERE " + T::primaryKey() + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db.getDB(), q.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Prepare failed: " << sqlite3_errmsg(db.getDB()) << endl;
            return false;
        }
        sqlite3_bind_int(stmt, 1, id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE);
    }

private:
    bool bindAndExecuteInsert(const User& u, sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, u.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, u.age);
        int rc = sqlite3_step(stmt);
        return (rc == SQLITE_DONE);
    }
    bool bindAndExecuteUpdate(const User& u, sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, u.name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, u.age);
        sqlite3_bind_int(stmt, 3, u.id);
        int rc = sqlite3_step(stmt);
        return (rc == SQLITE_DONE);
    }
    bool bindAndExecuteInsert(const Book& b, sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, b.title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, b.user_id);
        int rc = sqlite3_step(stmt);
        return (rc == SQLITE_DONE);
    }
};
