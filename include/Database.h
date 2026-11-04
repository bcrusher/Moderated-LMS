
#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

struct User {
    int id = 0;
    std::string username;
    std::string fullName;
    std::string role;
};

class Database {
private:
    sqlite3* db = nullptr;

    std::string toHex(const unsigned char* data, int length) {
        std::ostringstream result;
        result << std::hex << std::setfill('0');

        for (int i = 0; i < length; i++) {
            result << std::setw(2) << static_cast<int>(data[i]);
        }

        return result.str();
    }

    bool hashPassword(const std::string& password,
                      const std::string& salt,
                      std::string& hash) {
        unsigned char output[32];

        if (PKCS5_PBKDF2_HMAC(
                password.c_str(),
                static_cast<int>(password.length()),
                reinterpret_cast<const unsigned char*>(salt.data()),
                static_cast<int>(salt.length()),
                100000,
                EVP_sha256(),
                sizeof(output),
                output) != 1) {
            return false;
        }

        hash = toHex(output, sizeof(output));
        return true;
    }

public:
    bool connect(const std::string& filename) {
        if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Database error: "
                      << sqlite3_errmsg(db) << '\n';
            return false;
        }

        return true;
    }

    bool createTables() {
        const char* sql =
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL UNIQUE,"
            "full_name TEXT NOT NULL,"
            "email TEXT,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "role TEXT NOT NULL CHECK(role IN "
            "('Student','Lecturer','HoD','Admin'))"
            ");";

        char* errorMessage = nullptr;

        if (sqlite3_exec(db, sql, nullptr, nullptr,
                         &errorMessage) != SQLITE_OK) {
            std::cerr << "Table error: " << errorMessage << '\n';
            sqlite3_free(errorMessage);
            return false;
        }

        return true;
    }

    bool registerStudent(const std::string& username,
                         const std::string& fullName,
                         const std::string& email,
                         const std::string& password) {
        if (username.empty() || fullName.empty() ||
            password.length() < 8) {
            return false;
        }

        unsigned char saltBytes[16];

        if (RAND_bytes(saltBytes, sizeof(saltBytes)) != 1) {
            return false;
        }

        std::string salt = toHex(saltBytes, sizeof(saltBytes));
        std::string passwordHash;

        if (!hashPassword(password, salt, passwordHash)) {
            return false;
        }

        const char* sql =
            "INSERT INTO users "
            "(username, full_name, email, password_hash, salt, role) "
            "VALUES (?, ?, ?, ?, ?, 'Student');";

        sqlite3_stmt* statement = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1,
                               &statement, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(statement, 1, username.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 2, fullName.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 3, email.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 4, passwordHash.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 5, salt.c_str(), -1,
                          SQLITE_TRANSIENT);

        bool success = sqlite3_step(statement) == SQLITE_DONE;

        sqlite3_finalize(statement);
        return success;
    }

    bool login(const std::string& username,
               const std::string& password,
               User& loggedInUser) {
        const char* sql =
            "SELECT id, username, full_name, role, "
            "password_hash, salt "
            "FROM users WHERE username = ?;";

        sqlite3_stmt* statement = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1,
                               &statement, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(statement, 1, username.c_str(), -1,
                          SQLITE_TRANSIENT);

        if (sqlite3_step(statement) != SQLITE_ROW) {
            sqlite3_finalize(statement);
            return false;
        }

        int id = sqlite3_column_int(statement, 0);

        std::string savedUsername =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 1));

        std::string fullName =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2));

        std::string role =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 3));

        std::string savedHash =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 4));

        std::string salt =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 5));

        sqlite3_finalize(statement);

        std::string enteredHash;

        if (!hashPassword(password, salt, enteredHash)) {
            return false;
        }

        if (enteredHash != savedHash) {
            return false;
        }

        loggedInUser.id = id;
        loggedInUser.username = savedUsername;
        loggedInUser.fullName = fullName;
        loggedInUser.role = role;

        return true;
    }

    void close() {
        if (db != nullptr) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    ~Database() {
        close();
    }
};

#endif
