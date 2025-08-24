# gcpporm.h
 gcpporm  🚀 gcpporm – A lightweight, header-only ORM for C++  gcpporm is a simple ORM (Object Relational Mapper) written in C++. It allows you to map C++ classes directly to database tables and perform CRUD operations easily. Inspired by Mongoose (Node.js) and Hibernate (Java), but built for C++ developers. 

✨ Features
Header-only (#include "gcpporm.h")
Simple CRUD operations (Create, Read, Update, Delete)
Example Models: User, Book (with one-to-many relationship)
Uses SQLite3 as the backend
Easy to extend for other databases

📦 Installation

1. Install SQLite3 (if not already installed).

On Ubuntu/Debian:

sudo apt-get install libsqlite3-dev

On Windows: download from SQLite Downloads

2. Clone this repository:

git clone https://github.com/gaikwad shubham/gcpporm.git
cd gcpporm

3. Copy gcpporm.h into your project and include it:

#include "gcpporm.h"

🛠️ Usage Example

#include <iostream>
#include "gcpporm.h"
using namespace std;

int main() {
    // 1) Connect DB
    DB db("test.db");

    // 2) Create repositories
    Repository<User> users(db);
    Repository<Book> books(db);

    // --- CREATE ---
    cout << "\n--- CREATE ---" << endl;
    User u1("Alice", 25);
    users.save(u1);

    User u2("Bob", 30);
    users.save(u2);

    Book b1("C++ Guide", 1); // Alice's book
    books.save(b1);

    Book b2("Design Patterns", 1); // Alice's book
    books.save(b2);

    Book b3("Databases 101", 2); // Bob's book
    books.save(b3);

    // --- READ ---
    cout << "\n--- READ (All Users) ---" << endl;
    for (auto& usr : users.loadAll()) {
        cout << usr.id << " - " << usr.name << " (" << usr.age << ")" << endl;
    }

    cout << "\n--- READ (Find User by ID=1) ---" << endl;
    User alice = users.findById(1);
    cout << alice.id << " - " << alice.name << " (" << alice.age << ")" << endl;

    cout << "\n--- READ (All Books) ---" << endl;
    for (auto& bk : books.loadAll()) {
        cout << bk.id << " - " << bk.title << " (UserID=" << bk.user_id << ")" << endl;
    }

    // --- UPDATE ---
    cout << "\n--- UPDATE ---" << endl;
    alice.age = 26; // update Alice’s age
    users.update(alice);

    User updatedAlice = users.findById(1);
    cout << "Updated: " << updatedAlice.id << " - " << updatedAlice.name 
         << " (" << updatedAlice.age << ")" << endl;

    // --- DELETE ---
    cout << "\n--- DELETE ---" << endl;
    users.deleteById(2); // delete Bob

    cout << "Remaining users:" << endl;
    for (auto& usr : users.loadAll()) {
        cout << usr.id << " - " << usr.name << " (" << usr.age << ")" << endl;
    }

    return 0;
}


🌟 Goals

Make C++ usable in modern backend development

Add support for multiple databases (PostgreSQL, MySQL, MongoDB, etc.)

Provide a universal ORM for C++, like Mongoose for Node.js


