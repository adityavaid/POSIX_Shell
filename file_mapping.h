#include<fstream>
#include<iostream>
#include<unistd.h>
#include<unordered_map>
#include<utility>
#include<vector>
#include<string>
#include<sstream>

using namespace std;


unordered_map<string, pair<string, string>> file_map;
string FENV;

void populate_file_map() {
    fstream conf(FENV);
    string line;
    while (getline(conf, line)) {
        if (line == "**********Extension Map**********") {
            break;
        }
    }

    while (getline(conf, line)) {
        if (line == "********Extension Map End********")
            break;

        string s1, s2, s3;
        int i = 0;
        while (line[i] != '\'') {
            i++;
        }
        i++;
        while (line[i] != '\'') {
            s1 = s1 + line[i];
            i++;
        }
        i++;
        while (line[i] != '\'') {
            i++;
        }
        i++;
        while (line[i] != '\'') {
            s2 = s2 + line[i];
            i++;
        }
        i++;
        while (line[i] != '\'') {
            i++;
        }
        i++;
        while (line[i] != '\'') {
            s3 = s3 + line[i];
            i++;
        }

        file_map[s3] = make_pair(s1, s2);
    }
    conf.close();
}

void open_file(string file_path) {
    string line;
    stringstream ss(file_path);
    vector<string> v;
    while (getline(ss, line, '/')) {
        v.push_back(line);
    }

    string ext = v.back();
    stringstream exts(ext);
    string ex;
    while (getline(exts, line, '.')) {
        ex = line;
    }
    ex = "." + ex;
    int flag = 0;
    for (auto x : file_map) {
        if (x.first == ex) {
            flag = 1;
            execl(x.second.second.c_str(), x.second.first.c_str(), file_path.c_str(), NULL);
            break;
        }
    }
    if (flag != 1)
        execlp("xdg-open", "xdg-open", file_path.c_str(), NULL);
}

void display_file_mapping() {
    for (auto x : file_map) {
        cout << "Extension: " << x.first << endl;
        cout << "Command: " << x.second.first << endl;
        cout << "Command Path: " << x.second.second << endl;
    }
}
/*int main() {
    populate_file_map();

    open_file("/home/kishanfaldu/Desktop/POSIX/sampletxt.txt");
    return 0;
}*/
