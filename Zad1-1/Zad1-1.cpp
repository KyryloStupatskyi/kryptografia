#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <map>
#include <string>
#include <cmath> // do obliczeń statystycznych
#include <iomanip>

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
    for (char i : text) {
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

// Funkcja do obliczania n-gramów
map <string, int> get_multigrams(const int order, const string& text) {
    map<string, int> multigram_map;

    for (int i = 0; i < text.length() - (order - 1); i++) {
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

// Funkcja do tworzenia stringa z n-gramów
string multigram_map_stringify(const map<string, int>& mapper) {
    string result;
    for (const auto& p : mapper) {
        result.append(p.first + "   " + std::to_string(p.second) + "\n");
    }
    return result;
}

// Funkcja do odczytu referencyjnej bazy n-gramów
map<string, double> read_reference_ngrams(const string& filename) {
    map<string, double> refNgramMap;
    map<string, double> countNgramMap;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening reference file: " << filename << endl;
        exit(1);
    }

    string ngram;
    int count = 0;
    double sum = 0;

    for (string line; getline(file, line);) {
        count  = stoi(line.substr(line.find(' ') + 1, line.length()));
        countNgramMap[line.substr(0, line.find(' '))] = count;
        sum += count;
    }
    for (const auto& p : countNgramMap) {
        refNgramMap.insert(pair<string, double>(p.first, p.second/sum));
    }
    /*
    for (const auto& p : refNgramMap) {
        std::cout << std::setprecision(3) << std::fixed;
        std::cout << "Letter: " << p.first << " \nProbability: " << p.second << std::endl;
    }
    */
    file.close();
    return refNgramMap;
}

// Funkcja do odczytu referencyjnej bazy n-gramów z odrzuceniem najmniej wystepujacych
map<string, double> read_reference_ngrams_skip_infrequent(const string& filename) {
    map<string, double> refNgramMap;
    map<string, double> countNgramMap;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening reference file: " << filename << endl;
        exit(1);
    }

    string ngram;
    int count = 0;
    double sum = 0;

    for (string line; getline(file, line);) {
        count  = stoi(line.substr(line.find(' ') + 1, line.length()));
        countNgramMap[line.substr(0, line.find(' '))] = count;
        sum += count;
    }
    for (const auto& p : countNgramMap) {
        double prob = p.second/sum;
        if(prob >= 0.01)
            refNgramMap.insert(pair<string, double>(p.first, prob));
    }
    /*
    for (const auto& p : refNgramMap) {
        std::cout << std::setprecision(3) << std::fixed;
        std::cout << "Letter: " << p.first << " \nProbability: " << p.second << std::endl;
    }
    */
    file.close();
    return refNgramMap;
}

double calculate_chi_square(const map<string, int>& observed, const map<string, double>& expected, int totalNgrams) {
    double chiSquare = 0.0;
    for (const auto& pair : observed) {
        const string& ngram = pair.first;
        int observedCount = pair.second;

        if (expected.find(ngram) != expected.end()) {
            double expectedCount = expected.at(ngram) * totalNgrams;
            chiSquare += pow(observedCount - expectedCount, 2) / expectedCount;
        }
    }
    return chiSquare;
}


int main(int argc, char* argv[]) {
    string inputFile, outputFile, keyFile, engramOutFile, referenceFile;
    bool encryptMode = false, decryptMode = false, chiSquareMode = false;
    char engramMode = 0;
    char refNgramMode = 0;

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
        else if (arg == "-r1" || arg == "-r2" || arg == "-r3" || arg == "-r4") {
            refNgramMode = arg[2];
            referenceFile = argv[++i];
        }
        else if (arg == "-s") {
            chiSquareMode = true;
        }
    }

    if ((encryptMode && decryptMode) || (!encryptMode && !decryptMode && !chiSquareMode) || inputFile.empty() || (chiSquareMode && referenceFile.empty())) {
        cerr << "Usage: ./program -e|-d -k keyfile.txt -i inputfile.txt -o outputfile.txt [-rX referencefile.txt] [-s]" << endl;
        return 1;
    }

    string text = readFile(inputFile);
    text = toUpperCase(removeNonLetters(text));

    if (encryptMode || decryptMode) {
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
    }

    if (engramMode) {
        map<string, int> multigrams;
        switch (engramMode) {
        case '1':
            multigrams = get_multigrams(1, text);
            break;
        case '2':
            multigrams = get_multigrams(2, text);
            break;
        case '3':
            multigrams = get_multigrams(3, text);
            break;
        case '4':
            multigrams = get_multigrams(4, text);
            break;
        default:
            cerr << "Invalid multigram mode!" << endl;
            return 1;
        }
        writeFile(engramOutFile, multigram_map_stringify(multigrams));
    }

    if (chiSquareMode && refNgramMode) {
        int ngramOrder = refNgramMode - '0';
        map<string, int> observedNgrams = get_multigrams(ngramOrder, text);
        map<string, double> referenceNgrams = read_reference_ngrams(referenceFile);
        //map<string, double> referenceNgrams = read_reference_ngrams_skip_infrequent(referenceFile);

        int totalNgrams = text.length() - (ngramOrder - 1);
        double chiSquare = calculate_chi_square(observedNgrams, referenceNgrams, totalNgrams);

        std::cout << std::setprecision(8) << std::fixed;
        cout << "Chi-Square value: " << chiSquare << endl;
    }

    return 0;
}