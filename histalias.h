#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

string hist_file_name;// = "/home/sarthak/Documents/.history";
string hist_temp_file_name;// = "/home/sarthak/Documents/.history_temp";
string record_dir;// = "/home/sarthak/Documents/record.txt";
int HIST_SIZE;// = 10;

unordered_map<string, string> map_alias{};

/**
 * @brief Removes white spaces from the front and back of the string
 * 
 * @param str is the string to be cleaned.
 * @return string 
 */
string trim_text(string str)
{
    string temp;
    size_t s = 0;
    while(str[s] == ' ')
        s++;

    size_t e = str.length() - 1;
    while(str[e] == ' ')
        e--;

    // std::cout << s << " to " << e << std::endl;
    temp = str.substr(s,e-s+1);
    return temp;
}

/**
 * @brief tokenize the string based on the passed delimeter
 * 
 * @param main_command is the string command to be tokenize
 * @param del refers as delimeter.
 * @return vector<string> 
 */
vector<string> tokenMaker(string main_command, string del)
{
    vector<string> cmdtokens;

    main_command+=del + "dummy";
    cmdtokens.clear();
    string space_delimiter = del;

    size_t pos = 0;
    while ((pos = main_command.find(space_delimiter)) != string::npos)
    {
        cmdtokens.push_back(main_command.substr(0, pos));
        main_command.erase(0, pos + space_delimiter.length());
    }

    return cmdtokens;

}

/**
 * @brief Create a alias object in the map_alias
 * 
 * @param str refers to command beginning with "alias"
 */
void create_alias(string str)
{
    str = trim_text(str).substr(6);    //Removing "alias " from str argument

    //  alias -p
    if(trim_text(str) == "-p"){
        auto it = map_alias.begin();
        while(it != map_alias.end()) {
            cout << "alias " << it->first << "='" << it->second << "'" << endl;
            it++;
        }
        return;
    }

    //  alias xyz='command'
    int idx = str.find('=');
    if(idx == string::npos)
    {
        cout << "Invalid command" << endl;
        return;
    }

    string cmd = trim_text(str.substr(idx+1));
    string alias = trim_text(str.substr(0, idx));

    if(cmd[0] == '\'' || cmd[0] == '\"')
        cmd = cmd.substr(1);

    size_t len = cmd.size()-1;
    if(cmd[len] == '\'' || cmd[len] == '\"')
        cmd = cmd.substr(0, len);

    map_alias.insert(make_pair(alias, cmd));
    return;
}


/**
 * @brief Get the command by alias object referring map_alias
 * 
 * @param alias refers to key against which the command is saved
 * @return string aka command.
 */
string get_command_by_alias(string alias)
{
    auto it = map_alias.find(alias);
    if(it == map_alias.end())
    {
        //cout << "Invalid alias" << endl;
        return alias;
    }

    return it->second;
}

/**
 * @brief process the command input replace alias with respective commands
 * 
 * @param str refers to command with aliases
 * @return string refers to command after substitution
 */
string process_aliasing(string str){
    string resp = "";
    vector<string> vec = tokenMaker(str, "|");
    for(int i=0; i<vec.size(); i++){
        string a = trim_text(vec[i]);
        vector<string> token = tokenMaker(a, " ");

        string inner_cmd = "";
        for(int j=0; j<token.size(); j++){
            string tmp = trim_text(token[j]);
            if(j==0){
                //Command token
                tmp = get_command_by_alias(tmp);
            }
            // Handling $$ and $?
            vector<string> sub_token = tokenMaker(tmp, " ");
            tmp = "";
            for(int k=0; k<sub_token.size(); k++){
                string st = trim_text(sub_token[k]);
                if(st == "$$"){
                    pid_t pid = getpid();
                    tmp+= to_string(pid);
                }
                else if(st == "$?")
                    tmp += strerror(errno);
                else
                    tmp += st;

                if(k != sub_token.size()-1)
                    tmp += " ";
            }

            inner_cmd += tmp;
            if(j != token.size()-1){
                inner_cmd += " ";
            }
        }

        resp += inner_cmd;
        if(i != vec.size()-1){
            resp += "|";
        }
    }
    return resp;
}


/**
 * @brief Remove previously saved aliases
 * 
 * @param str refers string beginning with "unalias"
 */
void unalias_command(string str){
    //  unalias names
    str = trim_text(str);
    str = str.substr(8);

    // unalias -a
    if(trim_text(str) == "-a"){
        map_alias.clear();
        return;
    }

    // unalias name
    auto it = map_alias.find(trim_text(str));
    if(it != map_alias.end()){
        map_alias.erase(it);
    }
}

/**
 * @brief Set the history object in .history file and 
 * also maintain the max length of the file based on HIST_SIZE,
 * 
 * @param str is command to be maintained in history
 */
void set_history(string str){
    static int hist_count = -1;
    static int hist_start_idx = -1;

    
    if(trim_text(str).length() == 0){
        return;
    }

    if(hist_count == -1){
        std::ifstream ifs;
        ifs.open(hist_file_name, std::ios::in);

        std::string tmp{};
        while(std::getline(ifs, tmp)){
            if(hist_start_idx == -1){
                int idx = tmp.find_first_of(' ');
                hist_start_idx = stoi(tmp.substr(0,idx));
            }
            //cout << "hist: " << tmp << endl;
        }
        ifs.close();

        if(tmp.length() > 0){
            //cout << tmp << endl;
            int idx = tmp.find_first_of(' ');
            hist_count = stoi(tmp.substr(0,idx));
            //cout << hist_count << endl;
        }
        else {
            hist_count = 0;
        }
    }
    std::ofstream ofs;
    ofs.open(hist_file_name, std::ios::app);
    if(hist_count!=0)
        ofs << endl;
    ofs << ++hist_count << " " << str;
    ofs.close();

    if(hist_count > HIST_SIZE){
        std::ifstream ifs;
        std::string tmp;
        ifs.open(hist_file_name, std::ios::in);
        int extras = hist_count - hist_start_idx - HIST_SIZE + 1;
        while(extras>0){
            std::getline(ifs, tmp);
            extras--;
        }

        std::ofstream ofs;
        ofs.open(hist_temp_file_name, std::ios::out);
        bool is_first_line = true;
        while(std::getline(ifs, tmp)){
            if(is_first_line){
                int idx = tmp.find_first_of(' ');
                hist_start_idx = stoi(tmp.substr(0,idx));
                is_first_line = false;
            }
            else
                ofs << endl;

            ofs << tmp;
        }

        rename(hist_temp_file_name.c_str(), hist_file_name.c_str());
    }
}

/**
 * @brief Get the history object on to the screen
 * 
 */
void get_history() {
    std::ifstream ifs;
    ifs.open(hist_file_name, std::ios::in);

    std::string tmp{};
    while(std::getline(ifs, tmp)){
        cout << tmp << endl;
    }
    ifs.close();
}

void record_func(string str)
{
    if(str == "record stop") return;
    ofstream out(record_dir, std::ios_base::app | std::ios_base::out);

    out<<str+"\n";

    out.close();

}

void clear_record()
{
    ofstream out(record_dir,std::ios_base::trunc);
    out.close();
}

/**
 * @brief Main Function
 * 
 * @return 0
 */
/*int main()
{
    while(1)
    {
        int optn;
        cout << "Enter optn: ";
        cin >> optn;
        switch(optn)
        {
            case 1:
            {
                string cmd;
                cout << "enter command: ";
                cin.ignore();
                getline(cin, cmd);
                create_alias(cmd);
            }
            break;
            case 2:
            {
                string al;
                cout << "Enter alias: ";
                cin.ignore();
                getline(cin, al);
                cout << "The command is = " << get_command_by_alias(al) << endl;
            }
            break;
            case 3:
            {
                string cmd;
                cout << "enter command: ";
                cin.ignore();
                getline(cin, cmd);
                set_history(cmd);
            }
            break;
            case 4:
            {
                get_history();
            }
            break;
            case 5:
            {
                string cmd;
                cout << "enter command: ";
                cin.ignore();
                getline(cin, cmd);
                unalias_command(cmd);
            }
            break;
            case 6:
            {
                string cmd;
                cout << "enter command: ";
                cin.ignore();
                getline(cin, cmd);
                cout << "After process: " << process_aliasing(cmd) << endl;
            }
            break;
            default:
                return 0;
        }
    }
    return 0;
}*/
