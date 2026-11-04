#include <iostream>
#include <string>
#include "Database.h"

using namespace std;

int main() {
    Database database;

    if (!database.connect("data/lms.db")) {
        cout << "Database connection failed.\n";
        return 1;
    }

    if (!database.createTables()) {
        cout << "Table creation failed.\n";
        return 1;
    }

    cout << "\nDatabase connected successfully!\n";

    int choice = 0;

    do {
        cout << "\n===== MODERATED LEARNING SYSTEM =====\n";
        cout << "1. Register student\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Choose: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input.\n";
            continue;
        }

        cin.ignore(10000, '\n');

        if (choice == 1) {
            string name, username, email, password;

            cout << "Full name: ";
            getline(cin, name);

            cout << "Username: ";
            getline(cin, username);

            cout << "Email: ";
            getline(cin, email);

            cout << "Password (minimum 8 characters): ";
            getline(cin, password);

            if (database.registerStudent(
                    username, name, email, password)) {
                cout << "Registration successful!\n";
            } else {
                cout << "Registration failed.\n";
            }
        }
        else if (choice == 2) {
            string username, password;
            User currentUser;

            cout << "Username: ";
            getline(cin, username);

            cout << "Password: ";
            getline(cin, password);

            if (database.login(username, password, currentUser)) {
                cout << "Login successful!\n";
                cout << "Welcome, " << currentUser.fullName << '\n';
                cout << "Role: " << currentUser.role << '\n';
            } else {
                cout << "Incorrect username or password.\n";
            }
        }
        else if (choice == 3) {
            cout << "Thank you for using the LMS.\n";
        }
        else {
            cout << "Invalid option.\n";
        }

    } while (choice != 3);

    return 0;
}