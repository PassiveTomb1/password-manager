#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <fstream>
#include "csvstream/csvstream.hpp"
#include <opencv2/opencv.hpp>
#include <pybind11/embed.h>

using namespace std;
namespace py = pybind11;

string encrypt(const string &text)
{
    string encryptedText = text;

    for (char &c : encryptedText)
    {
        if (isalpha(c))
        {
            char offset = isupper(c) ? 'A' : 'a';
            c = (c - offset + 5) % 26 + offset;
        }
    }
    return encryptedText;
}

string decrypt(const string &text)
{
    string decryptedText = text;

    for (char &c : decryptedText)
    {
        if (isalpha(c))
        {
            char offset = isupper(c) ? 'A' : 'a';
            c = (c - offset - 5 + 26) % 26 + offset;
        }
    }
    return decryptedText;
}

class Password
{
public:
    Password()
        : non_encrypted_password(""), encrypted_password(""), username("") {}

    Password(string username, string password)
        : non_encrypted_password(password), username(username)
    {
        encrypted_password = encrypt(password);
    }
    string get_username() const
    {
        return username;
    }
    string get_password() const
    {
        return decrypt(encrypted_password);
    }

private:
    string non_encrypted_password;
    string encrypted_password;
    string username;
};

class PasswordManager
{
public:
    PasswordManager();
    string get_master_password();
    Password get_password(string site);
    bool checkPassword(string site);
    void set_master_password();
    void add_password(string site, string user, string pass);
    void edit_password(string site, string user, string pass);
    void remove_password(string site);
    void printAllPasswords();
    void write_to_pass();

private:
    string masterPassword;
    map<string, Password> passwords;
};

PasswordManager::PasswordManager()
{

    VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Error opening video capture" << std::endl;
        return -1;
    }

    Mat frame;
    cap >> frame;

    if (frame.empty()) {
        std::cerr << "Error capturing frame" << std::endl;
        return -1;
    }

    std::string filename = "person.jpg"; 

    imwrite(filename, frame);

    py::scoped_interpreter guard{};

    try {
        py::module face_recognition = py::module::import("faceID");
        py::object result = face_recognition.attr("recognize_face")("person.jpg");
    } catch (const std::exception& e) {
        std::cerr << "Python Error: " << e.what() << std::endl;
    }
    
    ifstream pass("passwords_" + result.cast<string>() + ".csv");
    if (!pass.is_open())
    {
        ofstream pass("passwords_" + result.cast<string>() + ".csv");
    }
    else
    {
        string line;
        getline(pass, line);
        while (getline(pass, line))
        {
            stringstream ss(line);
            string site, username, password;
            if (getline(ss, site, ',') && getline(ss, username, ',') && getline(ss, password, ','))
            {
                Password p(username, password);
                passwords[site] = p;
            }
        }
    }
}

string PasswordManager::get_master_password()
{
    ifstream mast("master_password.txt");
    string master;

    mast >> master;
    return decrypt(master);
}

Password PasswordManager::get_password(string site)
{
    return passwords[site];
}

void PasswordManager::add_password(string site, string user, string pass)
{
    passwords[site] = Password(user, pass);
}

void PasswordManager::edit_password(string site, string user, string pass)
{
    passwords[site] = Password(user, pass);
}

void PasswordManager::remove_password(string site)
{
    passwords.erase(site);
}

void PasswordManager::write_to_pass() {
    ofstream writeToPass("passwords.csv");
    for(auto it = passwords.begin(); it != passwords.end(); it++) {
        writeToPass << it->first << "," << it->second.get_username() << "," << it->second.get_password() << endl;
    }
}

void PasswordManager::printAllPasswords()
{
    for(auto it = passwords.begin(); it != passwords.end(); it++) {
        cout << "Website: " << it->first << endl;
        cout << "   Username: " << it->second.get_username() << endl;
        cout << "   Password: " << it->second.get_password() << endl;

    }
}

bool PasswordManager::checkPassword(string site)
{
    try
    {
        Password p = passwords.at(site);
        return true;
    }
    catch (const std::out_of_range &e)
    {
        return false;
    }
}

void PasswordManager::set_master_password()
{
    ofstream ofs;
    ofs.open("master_password.txt", std::ofstream::out | std::ofstream::trunc);
    cout << "Enter new master password: ";
    string masp;
    cin >> masp;
    ofs << encrypt(masp);
    cout << "Master password changed succsessfully!" << endl;
}

int main()
{

    ifstream infile("master_password.txt");
    if (!infile.is_open())
    {
        string mp;
        cout << "Thank you for choosing this password manager" << endl;
        cout << "Let's get you started by picking a master password" << endl;
        cout << "Pick a master password that you will use to accses your passwords: ";
        cin >> mp;
        ofstream masterFile("master_password.txt");
        masterFile << encrypt(mp);
        cout << "\nPassword succsesfully saved!" << endl;
    }

    else
    {
        cout << "Enter master password: ";
        string user;
        cin >> user;
        string s;
        getline(infile, s);
        while (decrypt(user) != s)
        {
            cout << "Incorrect, please try again: ";
            cin >> user;
        }
        cout << "Welcome" << endl;
    }

    PasswordManager pm;

    int option = -1;
    while (option != 7)
    {
        cout << "[1] Edit master password" << endl;
        cout << "[2] Acsess a password" << endl;
        cout << "[3] Add a password" << endl;
        cout << "[4] Edit a password" << endl;
        cout << "[5] Remove a password" << endl;
        cout << "[6] Print all websites" << endl;
        cout << "[7] Exit the program" << endl;
        cout << "Enter selection: ";
        string temp;
        cin >> temp;
        option = stoi(temp);
        switch (option)
        {
        case 1:
        {
            pm.set_master_password();
            break;
        }
        case 2:
        {
            cout << "Enter a webiste to accses its username and password: ";
            string site;
            cin >> site;
            if (!pm.checkPassword(site))
            {
                cout << "You don't have a password stored for that website" << endl;
            }
            else
            {
                cout << "Username: " << pm.get_password(site).get_username()
                     << "\nPassword: " << decrypt(pm.get_password(site).get_password()) << endl
                     << endl;
            }
            break;
        }

        case 3:
        {
            cout << "Enter the website you want to add: ";
            string site;
            cin >> site;
            cout << "Enter the username for the website: ";
            string user;
            cin >> user;
            cout << "Enter the password for the website: ";
            string pass;
            cin >> pass;

            pm.add_password(site, user, pass);
            cout << "Password saved succsesfully" << endl;
            break;
        }
        case 4:
        {
            cout << "Which website do you want to edit: ";
            string edit;
            cin >> edit;

            if (!pm.checkPassword(edit))
            {
                cout << "You don't have a password stored for that website" << endl;
            }
            else
            {
                cout << "Enter new user name: ";
                string user;
                cin >> user;
                cout << "Enter new password: ";
                string pass;
                cin >> pass;

                pm.edit_password(edit, user, pass);
            }
            break;
        }
        case 5:
            {
                cout << "Which website do you want to remove: ";
            string site;
            cin >> site;

            if (!pm.checkPassword(site))
            {
                cout << "You don't have a password stored for that website" << endl;
            }
            else
            {
                pm.remove_password(site);
                cout << "Password remvoed!" << endl;
            }
            }
            break;

        case 6:
            pm.printAllPasswords();
            break;

        case 7:
            cout << "Thanks for using this password manager!";
            break;

        default:
            cout << "Invalid choice. Please try again." << endl;
            break;
        }
    }

    pm.write_to_pass();

}