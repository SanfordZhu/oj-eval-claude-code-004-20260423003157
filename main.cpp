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
        // Username may come with quotes, check the actual content
        string checkName = name;
        if (checkName.size() >= 2 && checkName.front() == '"' && checkName.back() == '"') {
            checkName = checkName.substr(1, checkName.size() - 2);
        }

        if (checkName.empty() || checkName.length() > 30) return false;
        for (char c : checkName) {
            if (c < 32 || c > 126) return false; // ASCII visible characters only (32-126)
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

    string encodeString(const string& s) {
        string result;
        for (char c : s) {
            if (c == '\\') {
                result += "\\\\";
            } else if (c == ' ') {
                result += "\\s";
            } else if (c == '\n') {
                result += "\\n";
            } else {
                result += c;
            }
        }
        return result;
    }

    string decodeString(const string& s) {
        string result;
        for (size_t i = 0; i < s.length(); i++) {
            if (s[i] == '\\' && i + 1 < s.length()) {
                if (s[i+1] == '\\') {
                    result += '\\';
                    i++;
                } else if (s[i+1] == 's') {
                    result += ' ';
                    i++;
                } else if (s[i+1] == 'n') {
                    result += '\n';
                    i++;
                } else {
                    result += s[i];
                }
            } else {
                result += s[i];
            }
        }
        return result;
    }

    void loadData() {
        // Load users
        ifstream userFile(USERS_FILE);
        if (userFile) {
            users.clear();
            string line;
            while (getline(userFile, line)) {
                istringstream iss(line);
                string userID, password, username;
                int privilege;
                if (iss >> userID >> password >> username >> privilege) {
                    User user;
                    user.userID = decodeString(userID);
                    user.password = decodeString(password);
                    user.username = decodeString(username);
                    user.privilege = privilege;
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
                string ISBN, name, author, keyword;
                double price;
                int quantity;
                if (iss >> ISBN >> name >> author >> keyword >> price >> quantity) {
                    Book book;
                    book.ISBN = decodeString(ISBN);
                    book.name = decodeString(name);
                    book.author = decodeString(author);
                    book.keyword = decodeString(keyword);
                    book.price = price;
                    book.quantity = quantity;
                    books.push_back(book);
                }
            }
        }
    }

    void saveData() {
        // Save users
        ofstream userFile(USERS_FILE);
        for (const auto& user : users) {
            userFile << encodeString(user.userID) << " "
                     << encodeString(user.password) << " "
                     << encodeString(user.username) << " "
                     << user.privilege << "\n";
        }

        // Save books
        ofstream bookFile(BOOKS_FILE);
        for (const auto& book : books) {
            bookFile << encodeString(book.ISBN) << " "
                     << encodeString(book.name) << " "
                     << encodeString(book.author) << " "
                     << encodeString(book.keyword) << " "
                     << fixed << setprecision(2) << book.price << " "
                     << book.quantity << "\n";
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

    vector<string> tokenize(const string& cmd) {
        vector<string> tokens;
        istringstream iss(cmd);
        string token;
        bool inQuotes = false;
        string currentToken;

        for (size_t i = 0; i < cmd.length(); i++) {
            char c = cmd[i];
            if (c == '"') {
                inQuotes = !inQuotes;
                currentToken += c;
            } else if (c == ' ' && !inQuotes) {
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
            } else {
                currentToken += c;
            }
        }

        if (!currentToken.empty()) {
            tokens.push_back(currentToken);
        }

        return tokens;
    }

    void processCommand(const string& cmd) {
        // Skip empty lines or lines with only spaces
        if (cmd.find_first_not_of(' ') == string::npos) {
            return;
        }

        vector<string> tokens = tokenize(cmd);
        if (tokens.empty()) return;

        string command = tokens[0];

        if (command == "quit" || command == "exit") {
            exit(0);
        }
        else if (command == "su") {
            if (tokens.size() == 2) {
                su(tokens[1], "");
            } else if (tokens.size() == 3) {
                su(tokens[1], tokens[2]);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "logout") {
            logout();
        }
        else if (command == "register") {
            if (tokens.size() == 4) {
                // Remove quotes from username if present
                string username = tokens[3];
                if (username.size() >= 2 && username.front() == '"' && username.back() == '"') {
                    username = username.substr(1, username.size() - 2);
                }
                registerUser(tokens[1], tokens[2], username);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "passwd") {
            if (tokens.size() == 3) {
                // userID newPassword (root privilege)
                passwd(tokens[1], "", tokens[2]);
            } else if (tokens.size() == 4) {
                // userID currentPassword newPassword
                passwd(tokens[1], tokens[2], tokens[3]);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "useradd") {
            if (tokens.size() == 5) {
                string username = tokens[4];
                if (username.size() >= 2 && username.front() == '"' && username.back() == '"') {
                    username = username.substr(1, username.size() - 2);
                }
                try {
                    int privilege = stoi(tokens[3]);
                    useradd(tokens[1], tokens[2], privilege, username);
                } catch (...) {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "delete") {
            if (tokens.size() == 2) {
                deleteUser(tokens[1]);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "show") {
            if (tokens.size() == 1) {
                // show (no arguments) - show all books
                showBooks("", "");
            } else if (tokens.size() == 2) {
                if (tokens[1] == "finance") {
                    showFinance(-1); // show all finance
                } else {
                    // show with filter
                    string option = tokens[1];
                    if (option.find("-ISBN=") == 0) {
                        string ISBN = option.substr(6);
                        showBooks("ISBN", ISBN);
                    } else if (option.find("-name=") == 0) {
                        string name = option.substr(6);
                        if (name.size() >= 2 && name.front() == '"' && name.back() == '"') {
                            name = name.substr(1, name.size() - 2);
                        }
                        showBooks("name", name);
                    } else if (option.find("-author=") == 0) {
                        string author = option.substr(8);
                        if (author.size() >= 2 && author.front() == '"' && author.back() == '"') {
                            author = author.substr(1, author.size() - 2);
                        }
                        showBooks("author", author);
                    } else if (option.find("-keyword=") == 0) {
                        string keyword = option.substr(9);
                        if (keyword.size() >= 2 && keyword.front() == '"' && keyword.back() == '"') {
                            keyword = keyword.substr(1, keyword.size() - 2);
                        }
                        showBooks("keyword", keyword);
                    } else {
                        cout << "Invalid\n";
                    }
                }
            } else if (tokens.size() == 3 && tokens[1] == "finance") {
                try {
                    int count = stoi(tokens[2]);
                    showFinance(count);
                } catch (...) {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "buy") {
            if (tokens.size() == 3) {
                try {
                    int quantity = stoi(tokens[2]);
                    buyBook(tokens[1], quantity);
                } catch (...) {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "select") {
            if (tokens.size() == 2) {
                selectBook(tokens[1]);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "modify") {
            if (tokens.size() < 2) {
                cout << "Invalid\n";
                return;
            }
            vector<pair<string, string>> modifications;
            for (size_t i = 1; i < tokens.size(); i++) {
                string option = tokens[i];
                if (option.find("-ISBN=") == 0) {
                    string ISBN = option.substr(6);
                    modifications.push_back({"ISBN", ISBN});
                } else if (option.find("-name=") == 0) {
                    string name = option.substr(6);
                    if (name.size() >= 2 && name.front() == '"' && name.back() == '"') {
                        name = name.substr(1, name.size() - 2);
                    }
                    modifications.push_back({"name", name});
                } else if (option.find("-author=") == 0) {
                    string author = option.substr(8);
                    if (author.size() >= 2 && author.front() == '"' && author.back() == '"') {
                        author = author.substr(1, author.size() - 2);
                    }
                    modifications.push_back({"author", author});
                } else if (option.find("-keyword=") == 0) {
                    string keyword = option.substr(9);
                    if (keyword.size() >= 2 && keyword.front() == '"' && keyword.back() == '"') {
                        keyword = keyword.substr(1, keyword.size() - 2);
                    }
                    modifications.push_back({"keyword", keyword});
                } else if (option.find("-price=") == 0) {
                    string priceStr = option.substr(7);
                    modifications.push_back({"price", priceStr});
                }
            }
            modifyBook(modifications);
        }
        else if (command == "import") {
            if (tokens.size() == 3) {
                try {
                    int quantity = stoi(tokens[1]);
                    double totalCost = stod(tokens[2]);
                    importBooks(quantity, totalCost);
                } catch (...) {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        }
        else if (command == "log") {
            showLog();
        }
        else if (command == "report") {
            if (tokens.size() == 2) {
                if (tokens[1] == "finance") {
                    reportFinance();
                } else if (tokens[1] == "employee") {
                    reportEmployee();
                } else {
                    cout << "Invalid\n";
                }
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
            if (!isValidPassword(password)) {
                cout << "Invalid\n";
                return;
            }
            if (user->password != password) {
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
        // register command has privilege {0}, so anyone can register

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