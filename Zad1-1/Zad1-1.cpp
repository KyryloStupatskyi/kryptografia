﻿#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <map>
#include <string>

using namespace std;

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    string line, tmp;

    while (getline(file, line)) {
        tmp += line + "\n";
    }

    file.close();

    return tmp;
}

void writeFile(const string& filename, const string& content) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    file << content;
    file.close();
}

string toUpperCase(const string& text) {
    string result = text;

    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

string removeNonLetters(const string& text) {
    string result;

    for (char c : text) {
        if (isalpha(c)) {
            result += c;
        }
    }

    return result;
}

unordered_map<char, char> readKey(const string& filename) {
    unordered_map<char, char> keyMap;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening key file: " << filename << endl;
        exit(1);
    }

    char plainChar, cipherChar;
    while (file >> plainChar >> cipherChar) {
        keyMap[toupper(plainChar)] = toupper(cipherChar);
    }
    return keyMap;
}

unordered_map<char, char> invertKey(const unordered_map<char, char>& keyMap) {
    unordered_map<char, char> invertedKeyMap;
    for (const auto& pair : keyMap) {
        invertedKeyMap[pair.second] = pair.first;
    }
    return invertedKeyMap;
}

string encrypt(const string& text, const unordered_map<char, char>& keyMap) {
    string result;
    for (char i: text) {
        result += keyMap.at(i);
    }
    return result;
}

string decrypt(const string& text, const unordered_map<char, char>& invertedKeyMap) {
    string result;
    for (char c : text) {
        result += invertedKeyMap.at(c);
    }
    return result;
}

map <string, int> get_multigrams(const int order, const string& text) {
    map<string, int> multigram_map;

    for (int i = 0; i < text.length()-(order-1); i++) {
        auto bg = multigram_map.find(text.substr(i, order));
        if (bg != multigram_map.end()) {
            bg->second++;
        }
        else {
            multigram_map.insert(pair<string, int>(text.substr(i, order), 1));
        }
    }
    return multigram_map;
}

string multigram_map_stringify(const map<string, int>& mapper) {
    string result;
    for (const auto& p: mapper) {
        result.append(p.first + "   " + std::to_string(p.second) + "\n");
    }
    return result;
}

int main(int argc, char* argv[]) {
    string inputFile, outputFile, keyFile, engramOutFile;
    bool encryptMode = false, decryptMode = false;
    char engramMode = 0;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-i") {
            inputFile = argv[++i];
        }
        else if (arg == "-o") {
            outputFile = argv[++i];
        }
        else if (arg == "-k") {
            keyFile = argv[++i];
        }
        else if (arg == "-e") {
            encryptMode = true;
        }
        else if (arg == "-d") {
            decryptMode = true;
        }
        else if (arg == "-g1" || arg == "-g2" || arg == "-g3" || arg == "-g4") {
            engramMode = arg[2];
            engramOutFile = argv[++i];
        }
    }

    if ((encryptMode && decryptMode) || (!encryptMode && !decryptMode) || inputFile.empty() || outputFile.empty() || keyFile.empty()) {
        cerr << "Usage: ./program -e|-d -k keyfile.txt -i inputfile.txt -o outputfile.txt" << endl;
        return 1;
    }

    string text = readFile(inputFile);
    text = toUpperCase(removeNonLetters(text));

    unordered_map<char, char> keyMap = readKey(keyFile);
    unordered_map<char, char> invertedKeyMap = invertKey(keyMap);

    string result;

    if (encryptMode) {
        result = encrypt(text, keyMap);
    }
    else if (decryptMode) {
        result = decrypt(text, invertedKeyMap);
    }

    writeFile(outputFile, result);

    cout << "Operation completed successfully!" << endl;

    if(engramMode) {
        //std::cout << "Engram mode: " << engramMode << std::endl;
        map<string, int> multigrams;

        switch (engramMode) {
            case '1':
                multigrams = get_multigrams(1, text);
            /*
            for (const auto& p: multigrams) {
                cout << "Monogram: " << p.first << " Val: " << p.second << endl;
            }
            */
            writeFile(engramOutFile, multigram_map_stringify(multigrams));
            break;
            case '2':
                multigrams = get_multigrams(2, text);
                writeFile(engramOutFile, multigram_map_stringify(multigrams));
            break;
            case '3':
                multigrams = get_multigrams(3, text);
                writeFile(engramOutFile, multigram_map_stringify(multigrams));
            break;
            case '4':
                multigrams = get_multigrams(4, text);
                writeFile(engramOutFile, multigram_map_stringify(multigrams));
            break;
            default:
                cerr << "Invalid multigram mode!" << endl;
        }
    }

    return 0;
}