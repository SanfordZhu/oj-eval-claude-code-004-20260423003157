#include <iostream>
#include <string>
#include <sstream>

using namespace std;

bool isValidUsername(const string& name) {
    cout << "Checking username: '" << name << "' length: " << name.length() << endl;
    if (name.empty() || name.length() > 30) {
        cout << "Failed: empty or too long" << endl;
        return false;
    }
    for (char c : name) {
        cout << "Char: '" << c << "' int: " << (int)c << endl;
        if (c < 32 || c > 126) {
            cout << "Failed: char out of range" << endl;
            return false;
        }
    }
    return true;
}

int main() {
    string line = "register user1 password1 \"Test User\"";
    istringstream iss(line);
    string command, userID, password, username;
    iss >> command >> userID >> password;

    // Get the rest as username
    getline(iss, username);
    size_t start = username.find_first_not_of(' ');
    size_t end = username.find_last_not_of(' ');
    if (start != string::npos && end != string::npos) {
        username = username.substr(start, end - start + 1);
    } else {
        username = "";
    }

    cout << "Command: " << command << endl;
    cout << "UserID: " << userID << endl;
    cout << "Password: " << password << endl;
    cout << "Username: '" << username << "'" << endl;

    bool valid = isValidUsername(username);
    cout << "Username valid: " << (valid ? "true" : "false") << endl;

    return 0;
}