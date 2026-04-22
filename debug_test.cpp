#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main() {
    string line = "su root sjtu";
    istringstream iss(line);
    string command, userID, password;
    iss >> command >> userID >> password;

    cout << "Command: " << command << endl;
    cout << "UserID: " << userID << endl;
    cout << "Password: " << password << endl;

    // Test another command
    line = "register user1 pass1 \"User One\"";
    istringstream iss2(line);
    string cmd, uid, pwd, uname;
    iss2 >> cmd >> uid >> pwd >> uname;

    cout << "\nCommand: " << cmd << endl;
    cout << "UserID: " << uid << endl;
    cout << "Password: " << pwd << endl;
    cout << "Username: " << uname << endl;

    return 0;
}