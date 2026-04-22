#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace std;

// Constants
const string ROOT_USERNAME = "root";
const string ROOT_PASSWORD = "sjtu";
const int ROOT_PRIVILEGE = 7;

// Data structures
struct User {
    string userID;
    string password;
    string username;
    int privilege;

    User() : privilege(0) {}
    User(string id, string pwd, string name, int priv)
        : userID(id), password(pwd), username(name), privilege(priv) {}
};

struct Book {
    string ISBN;
    string name;
    string author;
    string keyword;
    double price;
    int quantity;

    Book() : price(0.0), quantity(0) {}
    Book(string isbn) : ISBN(isbn), price(0.0), quantity(0) {}
};

class BookstoreSystem {
private:
    vector<User> users;
    vector<Book> books;
    vector<string> loginStack; // Stores userIDs of logged-in users
    map<string, string> selectedBooks; // userID -> selected ISBN

    // File paths for persistence
    const string USERS_FILE = "users.dat";
    const string BOOKS_FILE = "books.dat";

    // Helper functions
    User* findUser(const string& userID) {
        for (auto& user : users) {
            if (user.userID == userID) {
                return &user;
            }
        }
        return nullptr;
    }

    Book* findBook(const string& ISBN) {
        for (auto& book : books) {
            if (book.ISBN == ISBN) {
                return &book;
            }
        }
        return nullptr;
    }

    bool isValidUserID(const string& id) {
        if (id.empty() || id.length() > 30) return false;
        for (char c : id) {
            if (!isalnum(c) && c != '_') return false;
        }
        return true;
    }

    bool isValidPassword(const string& pwd) {
        if (pwd.empty() || pwd.length() > 30) return false;
        for (char c : pwd) {
            if (!isalnum(c) && c != '_') return false;
        }
        return true;
    }

    bool isValidUsername(const string& name) {
        if (name.empty() || name.length() > 30) return false;
        for (char c : name) {
            if (c < 32 || c > 126) return false; // ASCII visible characters only
        }
        return true;
    }

    bool isValidISBN(const string& isbn) {
        if (isbn.empty() || isbn.length() > 20) return false;
        for (char c : isbn) {
            if (c < 32 || c > 126) return false; // ASCII non-invisible characters
        }
        return true;
    }

    bool isValidBookName(const string& name) {
        if (name.empty() || name.length() > 60) return false;
        for (char c : name) {
            if (c < 32 || c > 126 || c == '\"') return false;
        }
        return true;
    }

    bool isValidAuthor(const string& author) {
        return isValidBookName(author); // Same constraints as book name
    }

    bool isValidKeyword(const string& keyword) {
        if (keyword.empty() || keyword.length() > 60) return false;
        for (char c : keyword) {
            if (c < 32 || c > 126 || c == '\"') return false;
        }
        return true;
    }

    int getCurrentPrivilege() {
        if (loginStack.empty()) return 0;
        User* user = findUser(loginStack.back());
        return user ? user->privilege : 0;
    }

    string getCurrentUserID() {
        return loginStack.empty() ? "" : loginStack.back();
    }

    void loadData() {
        // Load users
        ifstream userFile(USERS_FILE);
        if (userFile) {
            users.clear();
            string line;
            while (getline(userFile, line)) {
                istringstream iss(line);
                User user;
                if (iss >> user.userID >> user.password >> user.username >> user.privilege) {
                    users.push_back(user);
                }
            }
        }

        // Load books
        ifstream bookFile(BOOKS_FILE);
        if (bookFile) {
            books.clear();
            string line;
            while (getline(bookFile, line)) {
                istringstream iss(line);
                Book book;
                if (iss >> book.ISBN >> book.name >> book.author >> book.keyword >> book.price >> book.quantity) {
                    books.push_back(book);
                }
            }
        }
    }

    void saveData() {
        // Save users
        ofstream userFile(USERS_FILE);
        for (const auto& user : users) {
            userFile << user.userID << " " << user.password << " "
                    << user.username << " " << user.privilege << "\n";
        }

        // Save books
        ofstream bookFile(BOOKS_FILE);
        for (const auto& book : books) {
            bookFile << book.ISBN << " " << book.name << " " << book.author << " "
                    << book.keyword << " " << fixed << setprecision(2) << book.price
                    << " " << book.quantity << "\n";
        }
    }

public:
    BookstoreSystem() {
        // Initialize with root user if no users exist
        loadData();
        if (users.empty()) {
            users.push_back(User(ROOT_USERNAME, ROOT_PASSWORD, "superadmin", ROOT_PRIVILEGE));
            saveData();
        }
    }

    ~BookstoreSystem() {
        saveData();
    }

    void processCommand(const string& cmd) {
        istringstream iss(cmd);
        string command;
        iss >> command;

        if (command.empty()) {
            // Empty command, do nothing
            return;
        }

        if (command == "quit" || command == "exit") {
            exit(0);
        }
        else if (command == "su") {
            string userID, password;
            iss >> userID;
            if (iss >> password) {
                su(userID, password);
            } else {
                su(userID, "");
            }
        }
        else if (command == "logout") {
            logout();
        }
        else if (command == "register") {
            string userID, password, username;
            iss >> userID >> password >> username;
            registerUser(userID, password, username);
        }
        else if (command == "passwd") {
            string userID, currentPassword, newPassword;
            iss >> userID;
            if (iss >> currentPassword) {
                if (iss >> newPassword) {
                    passwd(userID, currentPassword, newPassword);
                } else {
                    // Only two arguments, so currentPassword is actually newPassword
                    newPassword = currentPassword;
                    passwd(userID, "", newPassword);
                }
            }
        }
        else if (command == "useradd") {
            string userID, password, privilegeStr, username;
            iss >> userID >> password >> privilegeStr >> username;
            int privilege = stoi(privilegeStr);
            useradd(userID, password, privilege, username);
        }
        else if (command == "delete") {
            string userID;
            iss >> userID;
            deleteUser(userID);
        }
        else if (command == "show") {
            string option;
            iss >> option;
            if (option.empty()) {
                showBooks("", "");
            } else if (option.find("-ISBN=") == 0) {
                string ISBN = option.substr(6);
                showBooks("ISBN", ISBN);
            } else if (option.find("-name=") == 0) {
                string name = option.substr(7, option.length() - 8); // Remove quotes
                showBooks("name", name);
            } else if (option.find("-author=") == 0) {
                string author = option.substr(9, option.length() - 10);
                showBooks("author", author);
            } else if (option.find("-keyword=") == 0) {
                string keyword = option.substr(10, option.length() - 11);
                showBooks("keyword", keyword);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "buy") {
            string ISBN;
            string quantityStr;
            iss >> ISBN >> quantityStr;
            int quantity = stoi(quantityStr);
            buyBook(ISBN, quantity);
        }
        else if (command == "select") {
            string ISBN;
            iss >> ISBN;
            selectBook(ISBN);
        }
        else if (command == "modify") {
            vector<pair<string, string>> modifications;
            string option;
            while (iss >> option) {
                if (option.find("-ISBN=") == 0) {
                    string ISBN = option.substr(6);
                    modifications.push_back({"ISBN", ISBN});
                } else if (option.find("-name=") == 0) {
                    string name = option.substr(7, option.length() - 8);
                    modifications.push_back({"name", name});
                } else if (option.find("-author=") == 0) {
                    string author = option.substr(9, option.length() - 10);
                    modifications.push_back({"author", author});
                } else if (option.find("-keyword=") == 0) {
                    string keyword = option.substr(10, option.length() - 11);
                    modifications.push_back({"keyword", keyword});
                } else if (option.find("-price=") == 0) {
                    string priceStr = option.substr(7);
                    modifications.push_back({"price", priceStr});
                }
            }
            modifyBook(modifications);
        }
        else if (command == "import") {
            string quantityStr, totalCostStr;
            iss >> quantityStr >> totalCostStr;
            int quantity = stoi(quantityStr);
            double totalCost = stod(totalCostStr);
            importBooks(quantity, totalCost);
        }
        else if (command == "show") {
            string subcmd;
            iss >> subcmd;
            if (subcmd == "finance") {
                string countStr;
                if (iss >> countStr) {
                    int count = stoi(countStr);
                    showFinance(count);
                } else {
                    showFinance(-1); // -1 means all
                }
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "log") {
            showLog();
        }
        else if (command == "report") {
            string reportType;
            iss >> reportType;
            if (reportType == "finance") {
                reportFinance();
            } else if (reportType == "employee") {
                reportEmployee();
            } else {
                cout << "Invalid\n";
            }
        }
        else {
            cout << "Invalid\n";
        }
    }

private:
    // Command implementations
    void su(const string& userID, const string& password) {
        if (!isValidUserID(userID)) {
            cout << "Invalid\n";
            return;
        }

        User* user = findUser(userID);
        if (!user) {
            cout << "Invalid\n";
            return;
        }

        int currentPrivilege = getCurrentPrivilege();
        if (password.empty()) {
            // Password can be omitted if current privilege > target privilege
            if (currentPrivilege <= user->privilege) {
                cout << "Invalid\n";
                return;
            }
        } else {
            if (!isValidPassword(password) || user->password != password) {
                cout << "Invalid\n";
                return;
            }
        }

        loginStack.push_back(userID);
        // No output on success
    }

    void logout() {
        if (loginStack.empty()) {
            cout << "Invalid\n";
            return;
        }

        string userID = loginStack.back();
        selectedBooks.erase(userID);
        loginStack.pop_back();
        // No output on success
    }

    void registerUser(const string& userID, const string& password, const string& username) {
        if (getCurrentPrivilege() < 0) { // Guest can register
            cout << "Invalid\n";
            return;
        }

        if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
            cout << "Invalid\n";
            return;
        }

        if (findUser(userID)) {
            cout << "Invalid\n";
            return;
        }

        users.push_back(User(userID, password, username, 1));
        saveData();
        // No output on success
    }

    void passwd(const string& userID, const string& currentPassword, const string& newPassword) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        if (!isValidUserID(userID) || !isValidPassword(newPassword)) {
            cout << "Invalid\n";
            return;
        }

        User* user = findUser(userID);
        if (!user) {
            cout << "Invalid\n";
            return;
        }

        int currentPrivilege = getCurrentPrivilege();
        if (currentPrivilege == ROOT_PRIVILEGE) {
            // Root can change password without current password
            user->password = newPassword;
        } else {
            if (!isValidPassword(currentPassword) || user->password != currentPassword) {
                cout << "Invalid\n";
                return;
            }
            user->password = newPassword;
        }

        saveData();
        // No output on success
    }

    void useradd(const string& userID, const string& password, int privilege, const string& username) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
            cout << "Invalid\n";
            return;
        }

        if (privilege != 1 && privilege != 3 && privilege != 7) {
            cout << "Invalid\n";
            return;
        }

        if (privilege >= getCurrentPrivilege()) {
            cout << "Invalid\n";
            return;
        }

        if (findUser(userID)) {
            cout << "Invalid\n";
            return;
        }

        users.push_back(User(userID, password, username, privilege));
        saveData();
        // No output on success
    }

    void deleteUser(const string& userID) {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        if (!isValidUserID(userID)) {
            cout << "Invalid\n";
            return;
        }

        // Check if user exists
        auto it = find_if(users.begin(), users.end(), [&](const User& u) {
            return u.userID == userID;
        });

        if (it == users.end()) {
            cout << "Invalid\n";
            return;
        }

        // Check if user is logged in
        for (const auto& loggedInID : loginStack) {
            if (loggedInID == userID) {
                cout << "Invalid\n";
                return;
            }
        }

        users.erase(it);
        saveData();
        // No output on success
    }

    void showBooks(const string& filterType, const string& filterValue) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        vector<Book*> matchingBooks;
        for (auto& book : books) {
            bool match = false;
            if (filterType.empty()) {
                match = true;
            } else if (filterType == "ISBN") {
                match = (book.ISBN == filterValue);
            } else if (filterType == "name") {
                match = (book.name == filterValue);
            } else if (filterType == "author") {
                match = (book.author == filterValue);
            } else if (filterType == "keyword") {
                // Check if keyword contains the filter value
                size_t pos = book.keyword.find(filterValue);
                match = (pos != string::npos);
            }

            if (match) {
                matchingBooks.push_back(&book);
            }
        }

        // Sort by ISBN
        sort(matchingBooks.begin(), matchingBooks.end(), [](Book* a, Book* b) {
            return a->ISBN < b->ISBN;
        });

        for (auto book : matchingBooks) {
            cout << book->ISBN << "\t" << book->name << "\t" << book->author << "\t"
                 << book->keyword << "\t" << fixed << setprecision(2) << book->price
                 << "\t" << book->quantity << "\n";
        }

        if (matchingBooks.empty()) {
            cout << "\n";
        }
    }

    void buyBook(const string& ISBN, int quantity) {
        if (getCurrentPrivilege() < 1) {
            cout << "Invalid\n";
            return;
        }

        if (!isValidISBN(ISBN) || quantity <= 0) {
            cout << "Invalid\n";
            return;
        }

        Book* book = findBook(ISBN);
        if (!book || book->quantity < quantity) {
            cout << "Invalid\n";
            return;
        }

        double total = book->price * quantity;
        book->quantity -= quantity;
        saveData();

        cout << fixed << setprecision(2) << total << "\n";
    }

    void selectBook(const string& ISBN) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        if (!isValidISBN(ISBN)) {
            cout << "Invalid\n";
            return;
        }

        string currentUser = getCurrentUserID();
        if (currentUser.empty()) {
            cout << "Invalid\n";
            return;
        }

        // Create book if it doesn't exist
        Book* book = findBook(ISBN);
        if (!book) {
            books.push_back(Book(ISBN));
            saveData();
        }

        selectedBooks[currentUser] = ISBN;
        // No output on success
    }

    void modifyBook(const vector<pair<string, string>>& modifications) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        string currentUser = getCurrentUserID();
        if (currentUser.empty()) {
            cout << "Invalid\n";
            return;
        }

        auto it = selectedBooks.find(currentUser);
        if (it == selectedBooks.end()) {
            cout << "Invalid\n";
            return;
        }

        string ISBN = it->second;
        Book* book = findBook(ISBN);
        if (!book) {
            cout << "Invalid\n";
            return;
        }

        // Check for duplicate modification types
        map<string, bool> seen;
        for (const auto& mod : modifications) {
            if (seen[mod.first]) {
                cout << "Invalid\n";
                return;
            }
            seen[mod.first] = true;
        }

        // Apply modifications
        for (const auto& mod : modifications) {
            if (mod.first == "ISBN") {
                if (!isValidISBN(mod.second) || mod.second == book->ISBN || findBook(mod.second)) {
                    cout << "Invalid\n";
                    return;
                }
                book->ISBN = mod.second;
                it->second = mod.second; // Update selected book
            } else if (mod.first == "name") {
                if (!isValidBookName(mod.second)) {
                    cout << "Invalid\n";
                    return;
                }
                book->name = mod.second;
            } else if (mod.first == "author") {
                if (!isValidAuthor(mod.second)) {
                    cout << "Invalid\n";
                    return;
                }
                book->author = mod.second;
            } else if (mod.first == "keyword") {
                if (!isValidKeyword(mod.second)) {
                    cout << "Invalid\n";
                    return;
                }
                book->keyword = mod.second;
            } else if (mod.first == "price") {
                try {
                    double price = stod(mod.second);
                    if (price < 0) {
                        cout << "Invalid\n";
                        return;
                    }
                    book->price = price;
                } catch (...) {
                    cout << "Invalid\n";
                    return;
                }
            }
        }

        saveData();
        // No output on success
    }

    void importBooks(int quantity, double totalCost) {
        if (getCurrentPrivilege() < 3) {
            cout << "Invalid\n";
            return;
        }

        if (quantity <= 0 || totalCost <= 0) {
            cout << "Invalid\n";
            return;
        }

        string currentUser = getCurrentUserID();
        if (currentUser.empty()) {
            cout << "Invalid\n";
            return;
        }

        auto it = selectedBooks.find(currentUser);
        if (it == selectedBooks.end()) {
            cout << "Invalid\n";
            return;
        }

        Book* book = findBook(it->second);
        if (!book) {
            cout << "Invalid\n";
            return;
        }

        book->quantity += quantity;
        saveData();
        // No output on success
    }

    void showFinance(int count) {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        // Simplified implementation - always return 0.00 for now
        cout << "+ 0.00 - 0.00\n";
    }

    void showLog() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        // Simplified implementation
        cout << "Log system not fully implemented\n";
    }

    void reportFinance() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        // Simplified implementation
        cout << "Financial report not fully implemented\n";
    }

    void reportEmployee() {
        if (getCurrentPrivilege() < 7) {
            cout << "Invalid\n";
            return;
        }

        // Simplified implementation
        cout << "Employee report not fully implemented\n";
    }
};

int main() {
    BookstoreSystem system;
    string line;

    while (getline(cin, line)) {
        system.processCommand(line);
    }

    return 0;
}