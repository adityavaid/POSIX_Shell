//-----------------------------------------------HEADER FILES--------------------------------------------------------------//
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <fcntl.h>
#include <cstring>
#include <bits/stdc++.h>
#include <termios.h>
#include <dirent.h>
#include <time.h>
#include "histalias.h"
#include "alarm.h"
#include "file_mapping.h"
#define clearScreen() cout<<"\033c";  //printf("\033[H\033[J")
#define clearLine() printf("\33[2K\r")
#define MAX 10                        //Change for Window Size
using namespace std;

#define RESET   "\033[0m"
#define BOLDYELLOW  "\033[1m\033[33m"
#define BOLDCYAN    "\033[1m\033[36m"
#define BOLDWHITE   "\033[1m\033[37m"
//------------------------------------------------GLOBAL VARIABLES------------------------------------------------
struct termios orig_term, norm_term;
extern char** environ;
char CWD[256];  //can only use getcwd() with char array
char HOST[256];
string USER;
string ENV;

int sizecwd=256;
string HomeDir;  // store main program directory
string RootDir;

string main_command=""; //this will store main command user is typing
bool record_flag = false;
unordered_map<string, string> env_vars;
int lasterr = 0;
stack<string> prevstack, nextstack;

//------------------------------------------------HELPER FUNCTIONS-----------------------------------------------------

void print_evariable()
{
     for(char **current = environ; *current; current++) {
        puts(*current);
    }
}

void printCWD()
{
    cout<<"\033[1;44m"<<"CWD : "<<CWD<<"\033[0m";
    fflush(stdout);
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


vector<string> TokenMaker(string main_command, string del)
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


void NormalModeDisable()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
    clearScreen();
}

void NormalModeEnable()
{
    struct termios noncanon = orig_term;
    noncanon.c_lflag &= ~(ECHO|ICANON);
    norm_term=noncanon;
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &noncanon))
    {
        cout<<"ERROR: UNABLE TO SWITCH TO NORMAL MODE";
    }
}

string GetEnv( string  var )                // Function to get environment variables
{
    const char * val = getenv( var.c_str() );
    if ( val == nullptr )
    {
        return "";
    }
    else {
        return string(val);
    }
}

string getenviron(string key);
void PS1(string str)
{
    string tempcwd = string(CWD);
    replace(tempcwd, RootDir, "~");
    
    replace(str,"|user|",USER);
    replace(str,"|host|",HOST);
    replace(str,"|cwd|",tempcwd);
    
    vector<string> strarr = TokenMaker(str, ":");
    
    cout << BOLDCYAN << strarr[0];
    if(strarr.size() > 1) cout << BOLDWHITE << ":" << BOLDYELLOW << strarr[1];
    cout << RESET; 
    
}

void CmdRefresh()
{
    clearLine();

    getcwd(CWD,256);
    string tempcwd = string(CWD);

    replace(tempcwd, RootDir, "~");
    string str = getenviron("PS1");
    
    //cout << str;
    PS1(str);
    //cout<<BOLDYELLOW<<USER<<"@"<<HOST<<BOLDWHITE<<":"<<BOLDCYAN<<tempcwd<<BOLDWHITE<<"$ "<<RESET<<main_command;
    cout << main_command;
    fflush(stdout);
}

stack<string> get_history_stack(string hist_file_name) {
    stack<string> stk;
    ifstream ifs;
    ifs.open(hist_file_name, std::ios::in);

    string tmp;
    while(getline(ifs, tmp)){
        int idx = tmp.find_first_of(' ');
        if(idx == string::npos){
            continue;
        }
        tmp = tmp.substr(idx+1);
        stk.push(tmp);
    }

    return stk;
}

void load_stack()
{
    prevstack = get_history_stack("history");
}


void prevcmd()
{
    if(prevstack.empty())
    {
        return;
    }    

    main_command=prevstack.top();
    prevstack.pop();
    nextstack.push(main_command);

}

void nextcmd()
{
    if(nextstack.empty())
    {
        return;
    }

    main_command=nextstack.top();
    nextstack.pop();
    prevstack.push(main_command);

}

void stack_update()
{
    while(!nextstack.empty())
    nextstack.pop();

    prevstack.push(main_command);
}
//--------------------------------------------MYRC.TXT------------------------------------------------------------------

void setenviron(string key, string val)
{
    ifstream in(ENV);
    string buffer;
    string final_buff="";

    while(getline(in,buffer))
    {

        size_t pos=0;
        pos = buffer.find("=");

        string temp = buffer.substr(0,pos);
        if(temp != key)
        {
            final_buff += buffer + "\n";
        }


    }

    in.close();

    final_buff += key+"="+val;


    ofstream out(ENV);
    out<<final_buff;

    out.close();
}


string getenviron(string key)  //return NaN when no variable available
{

    ifstream in(ENV);
    string buffer;

    while(getline(in,buffer))
    {

        size_t pos=0;
        pos = buffer.find("=");

        string temp = buffer.substr(0,pos);
        string tempans = buffer.substr(pos+1,buffer.length());
        if(temp == key)
        {

            in.close();
            return tempans;

        }


    }


    in.close();
    return "NaN";

}


//--------------------------------------------TRIE---------------------------------------------------------------------

struct TrieNode
{
    bool EOW;
    map<char,TrieNode*> m;
    //char al;

    TrieNode()
    {

        EOW=false;

    }
};

class Trie
{

public:

    TrieNode *root;

    Trie()
    {
        root=nullptr;
    }


    void insertUtil(TrieNode* ptr, string str)
    {
        if(ptr->m.find(str[0]) == ptr->m.end())
        {
            ptr->m[str[0]]= new TrieNode();
        }

        if(str.length()==1)
        {
            TrieNode *temp=ptr->m[str[0]];
            temp->EOW=true;
            return ;
        }
        else
        {
            insertUtil(ptr->m[str[0]], str.substr(1,str.length()));
        }
    }

    void insert(string str)
    {
        if(root==nullptr)
        {
            root= new TrieNode();
        }


        if(str=="")
        {
            root->EOW=true;
            return;
        }
        else
        {
            insertUtil(root,str);
        }
    }


    bool searchUtil(TrieNode *ptr, string str)
    {


        char key = str[0];


        if(ptr->m.find(key)==ptr->m.end())
        {
            return false;
        }

        TrieNode *temp=ptr->m[key];

        if(str.length()==1 && temp->EOW==true)
        {
            return true;
        }

        return searchUtil(temp,str.substr(1,str.length()));

    }


    bool search(string str)
    {
        if(root==nullptr)
        {
            return false;
        }

        if(str=="" && root->EOW==true)
        {
            return true;
        }


        return searchUtil(root,str);
    }

//------------------------------------------------------------------




    void autocompleteUtil(TrieNode *ptr, vector<string> &v, string str)
    {
        if(ptr->EOW==true)
        {
            v.push_back(str);
        }


        if(ptr->m.size()==0)
        {
            return;
        }


        map<char,TrieNode*> :: iterator it;

        for(it=ptr->m.begin(); it!=ptr->m.end(); it++)
        {
            str.push_back(it->first);
            autocompleteUtil(it->second, v, str);
            str.pop_back();
        }

    }



    vector<string> autocomplete(string str)
    {
        vector<string> ans;

        TrieNode *temp=root;

        if(root==nullptr)
        {
            return ans;
        }

        for(int i=0;i<str.length();i++)
        {
            if(temp->m.find(str[i])==temp->m.end())
            {
                return ans;
            }

            temp=temp->m[str[i]];
        }

        if(temp->EOW==true)
        {
            ans.push_back(str);
        }
        map<char,TrieNode*> :: iterator it;

        for(it=temp->m.begin();it!=temp->m.end();it++)
        {
            str.push_back(it->first);
            autocompleteUtil(it->second,ans,str);
            str.pop_back();

        }



        return ans;




    }

//-------------------------------------------------------------------------------------------------------

    void autocorrectUtil(TrieNode *ptr, vector<string> &v, string str, vector<int> dp, string temp_ans)
    {
        vector<int> newdp(dp.size());


        if(dp[dp.size()-1]<=3 && ptr->EOW==true)
        {
            v.push_back(temp_ans);
        }


        if(ptr->m.size()==0)
        {
            return;
        }



        map<char,TrieNode*> :: iterator it;

        for(it=ptr->m.begin();it!=ptr->m.end();it++)
        {
            char x=it->first;

            for(int i=0;i<dp.size();i++)
            {
                int change;

                if(i==0)
                {
                    newdp[i]=temp_ans.length()+1;
                    continue;
                }

                if(x==str[i-1] )
                {
                    newdp[i]=dp[i-1];
                }
                else
                {
                    newdp[i]=min(newdp[i-1],min(dp[i],dp[i-1]))+1;
                }






            }



            temp_ans.push_back(x);

            autocorrectUtil(it->second,v,str,newdp,temp_ans);

            temp_ans.pop_back();



        }

    }




    vector<string> autocorrect(string str)
    {
        vector<string> ans;

        TrieNode *temp=root;

        if(root==nullptr)
        {
            return ans;
        }


        vector<int> dp(str.length()+1);

        for(int i=0;i<dp.size();i++)
        {
            dp[i]=i;
        }






        autocorrectUtil(root,ans,str,dp,"");


        return ans;






    }




};



vector<string> cur_cmd(string command)
{
    string darrowdone;
    reverse(command.begin(),command.end());
    size_t pos = 0;

    if((pos = command.find(">>")) != string::npos)
    {
        darrowdone = command.substr(0, pos);

    }
    else
    {
        darrowdone = command;
    }

    string arrowdone;

    if((pos = command.find(">")) != string::npos)
    {
        arrowdone = darrowdone.substr(0, pos);

    }
    else
    {
        arrowdone = darrowdone;
    }

    string pipedone;


    if((pos = command.find("|")) != string::npos)
    {
        pipedone = arrowdone.substr(0, pos);

    }
    else
    {
        pipedone = arrowdone;
    }


    string ans1 = command;
    string ans2 = pipedone;


    reverse(ans1.begin(),ans1.end());
    ans1 = ans1.substr(0,ans1.length()-pipedone.length());
    reverse(ans2.begin(),ans2.end());

    vector<string> res;

    res.push_back(ans1);
    res.push_back(ans2);

    return res;

}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Trie commmand_trie;





void commmand_trie_maker()
{
    DIR *dr;
    struct dirent* di;

    string str = getenviron("PATH");

    vector<string> cmdtokens = TokenMaker(str,":");

    for(int i=0; i< cmdtokens.size();i++)
    {
        dr=opendir(cmdtokens[i].c_str());
        if(dr== nullptr)
        {
            cout<<"Error: No directory found";
            return;
        }

        for(di= readdir(dr);di!= nullptr;di= readdir(dr))
        {
            commmand_trie.insert(string(di->d_name));

        }
        closedir(dr);
    }
    
    commmand_trie.insert("alarm");
    commmand_trie.insert("export");
    commmand_trie.insert("fg");
    commmand_trie.insert("alarm_check");
    commmand_trie.insert("proc");
    commmand_trie.insert("cd");
    commmand_trie.insert("alias");
    commmand_trie.insert("unalias");
    commmand_trie.insert("history");
    commmand_trie.insert("record");

}

vector<string> auto_complete_dir(string dir_str, string search_word)
{
    DIR *dr;
    struct dirent* di;
    Trie dir_trie;

    dr=opendir(dir_str.c_str());
    if(dr== nullptr)
    {
        cout<<"Error: No directory found"<<endl;
        return vector<string>();
    }

    for(di= readdir(dr);di!= nullptr;di= readdir(dr))
    {
        dir_trie.insert(string(di->d_name));

    }
    closedir(dr);



    return dir_trie.autocomplete(search_word);






}





void Auto_Complete()
{
    vector<string> sep_commands = cur_cmd(main_command), ansvect ;


    string cmd = sep_commands[1];


    vector<string> cmdtokens = TokenMaker(cmd," ");


    if(cmdtokens.size() ==  1)
    {
        ansvect = commmand_trie.autocomplete(cmdtokens[0]);



        if(ansvect.size() == 0)
        {
            return;
        }
        else if(ansvect.size() == 1)
        {
            main_command= sep_commands[0]+ansvect[0];
            return;
        }


    }
    else if(cmdtokens.size() == 0)
    {
        return;
    }
    else
    {
        string tempstr = cmdtokens.back();

        ansvect = auto_complete_dir(string(CWD), tempstr);




        if(ansvect.size() == 0)
        {
            return;
        }
        else if(ansvect.size() == 1)
        {
            main_command = sep_commands[0];

            for(int i=0;i<cmdtokens.size()-1;i++)
            {
                main_command+= cmdtokens[i]+" ";
            }

            main_command+=ansvect[0];
            return;
        }




    }


    cout<<endl;

    for(int i = 0; i<ansvect.size();i++)
    {
        cout<<ansvect[i]<<" ";
    }

    cout<<endl;

}

//----------------------------------------------------------------------------------------------------------------------

unordered_map<pid_t, string> process_map;

bool checkslash(string s){
	for(int i = 0; i < s.size(); i++){
		if(s[i] == '/') return true;
	}
	return false;
}

void callexec(vector<string> command, int input, int output){
	pid_t p;
	//int status;
	p = fork();
	if(p == 0){
		char** arr = new char*[command.size()+1];
		for(int i = 0; i < command.size(); i++){
			arr[i] = new char[command[i].size()];
			strcpy(arr[i], command[i].c_str());
		}
		arr[command.size()] = NULL;
		
		if(input != 0){
			dup2(input, 0);
			close(input);
		}
		if(output != 1){
			dup2(output, 1);
			close(output);
		}
		
		//check if command[0] is command or file
		//if file, run execl else execvp
		struct stat temp;
		string key = command[0];
		if(commmand_trie.search(key)){
			execvp(arr[0], arr);
		}
		else if(!stat(arr[0], &temp) && checkslash(key)){
			open_file(key);
		}
		else{
			cout << "No such command or directory" << endl;
		}
	}
	else{
		siginfo_t status;
		waitid(P_PID, p, &status, WEXITED);
		if(status.si_status != 0) exit(status.si_status);
	}
}

void executePipes(vector<vector<string>> command, string redirectFile, int redirectFlag = 0){
	int pipefd[2];
	int err;
	int input = 0;
	for(int i = 0; i < command.size()-1; i++){
		pipe(pipefd); // int[2]
		callexec(command[i], input, pipefd[1]);
		close(pipefd[1]);
		input = pipefd[0];
	}
	
	if(input != 0){
		dup2(input, 0);
		close(input);
	}
	
	//0 = stdin
	//1 = stdout
	//2 = stderr
	
	int fd = 1;
	if(redirectFlag == 1){
		fd = open(redirectFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0775);
	}
	else if(redirectFlag == 2){
		fd = open(redirectFile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0775);
	}
	
	callexec(command[command.size()-1], 0, fd);
	if(redirectFlag != 0) close(fd);
}

void executeCommand(vector<vector<string>> command, string redirectFile, int redirectFlag, bool bgflag){
	pid_t p;
	p = fork();
	if(p == 0){
		executePipes(command, redirectFile, redirectFlag);
		exit(0);
	}
	else{
		string command_s;
		for(vector<string> seg : command){
			for(string token : seg){
				command_s += token + " ";
			}
		}
		process_map[p] = command_s;
		
		if(!bgflag){
			siginfo_t status;
			waitid(P_PID, p, &status, WEXITED);
			lasterr = status.si_status;
			if(process_map.find(p) != process_map.end()) process_map.erase(p);
		}
	}
}




void processCommand(){
	bool bgflag = false;
	string main_command_local = main_command;
	for(int i = main_command.size()-1; i >= 0; i--){
		if(main_command[i] != ' '){
			if(main_command[i] == '&'){
				bgflag = true;
				main_command_local = TokenMaker(main_command_local, "&")[0];
			}
			break;
		}
	}

	vector<string> redirectParsed = TokenMaker(main_command_local, ">");
	int redirectFlag = redirectParsed.size()-1;
	
	string outfile = "";
	if(redirectParsed.size() > 1) outfile = trim_text(redirectParsed[redirectParsed.size()-1]);
	
	string basecommand = redirectParsed[0];

	vector<string> pipesegs = TokenMaker(basecommand, "|");
	
	for(int i = 0; i < pipesegs.size(); i++){
		pipesegs[i] = trim_text(pipesegs[i]);
	}
	
	
	vector<vector<string>> processedCommand;
	for(string seg : pipesegs){
		processedCommand.push_back(TokenMaker(seg, " "));
	}
	
	executeCommand(processedCommand, outfile, redirectFlag, bgflag);
}

void my_sigchld_handler(int sig){
	pid_t pid;
	int status;
	if ((pid = waitpid(-1, &status, WNOHANG)) != -1){
		//if(pid > 0) cout << "End of process: " << pid << endl;
		if(process_map.find(pid) != process_map.end()) process_map.erase(pid);
		//cout << "Process erased: " << pid << endl;
		if(alarm_map.find(pid) != alarm_map.end()){
			cout << endl;
			alarm_map[pid].display_details();
			alarm_map.erase(pid);
		}
		
		lasterr = status;
	}
}

void alarm_call(int signum) {
	exit(0);
}

void handle_alarm(){
	pid_t proc;
	alarm_details alrm_obj;
	long long sec = alrm_obj.set_alarm_values(main_command.substr(6, main_command.size()));
	if(sec <= 0){
		cout << endl << "Cannot set alarm" << endl;
		return;
	}
	proc = fork();
	if(proc == 0){
		signal(SIGALRM, alarm_call);
		alarm(sec);
		pause();
	}
	else{
		alarm_map[proc] = alrm_obj;
	}
}

void handle_premade_alarms(){
	for(string al : alarm_vec){
		pid_t proc;
		alarm_details alrm_obj;
		long long sec = alrm_obj.set_alarm_values(al);
		if(sec <= 0){
			cout << "Cannot set alarm" << endl;
			return;
		}
		proc = fork();
		if(proc == 0){
			signal(SIGALRM, alarm_call);
			alarm(sec);
			pause();
		}
		else{
			alarm_map[proc] = alrm_obj;
		}
	}
}

void parseCommand(){
	if(main_command.size() == 0) return;
	string tempcomm = TokenMaker(main_command, "|")[0];
	vector<string> firstseg = TokenMaker(tempcomm, " ");
	if(firstseg[0] == "cd"){
		string temp = firstseg[1];
		if(temp[0] == '~'){
			temp = "/home/"+ USER + temp.substr(1);
			cout << endl << "Changed to: " << temp << endl; 
		}
		chdir(temp.c_str());
		getcwd(CWD, sizeof(CWD));
	}
	else if(firstseg[0] == "exit"){
		NormalModeDisable();
		update_alarm_file();
		exit(0);
	}
	else if(firstseg[0] == "alarm"){
		handle_alarm();
	}
	else if(firstseg[0] == "history"){
		get_history();
	}
	else if(firstseg[0] == "alias"){
		create_alias(main_command);
	}
	else if(firstseg[0] == "unalias"){
		unalias_command(main_command);
	}
	else if(firstseg[0] == "proc"){
		for(auto p : process_map){
			cout << p.first << ": " << p.second << endl;
		}
	}
	else if(firstseg[0] == "export"){
		if(firstseg.size() == 1){
			cout << "PS1=" << getenviron("PS1") << endl;
			cout << "HIST_SIZE=" << getenviron("HIST_SIZE") << endl;
			cout << "PATH=" << getenviron("PATH") << endl;
			print_evariable();
		}
		else{
			string temp = "";
			for(int i = 1; i < firstseg.size(); i++){
				temp += firstseg[i] + " ";
			}
			temp.pop_back();
			if(temp.find("=") != string::npos){
				vector<string> temparr = TokenMaker(temp, "=");
				string rest = temp.substr(temparr[0].size()+1);
				if(temparr[0] == "PS1" || temparr[0] == "HIST_SIZE" || temparr[0] == "PATH"){
					setenviron(temparr[0], rest);
				}
				else{
					setenv(temparr[0].c_str(), rest.c_str(), 1);
				}
			}
		}
	}
	else if(firstseg[0] == "record"){
		if (firstseg.size() > 1){
			if(firstseg[1] == "start"){
				clear_record();
				record_flag = true;
			}
			else if(firstseg[1] == "stop"){
				record_flag = false;
			}
			else{
				cout << "Invalid argument" << endl;
			}
		}
		else{
			cout << "Invalid argument" << endl;
		}
	}
	else if(firstseg[0] == "alarm_check"){
		int i = 1;
		cout << "Current alarms: " << endl;
		if(alarm_map.size() == 0){
			cout << "None" << endl;
		}
		else{
			for(auto p : alarm_map){
				cout << "Alarm " << i << ":" << endl;
				p.second.display_details();
				i++;
			}
		}
	}
	else if(firstseg[0] == "fg"){
		if(firstseg.size() > 1){
			try{
				pid_t p = stoi(firstseg[1]);
				siginfo_t status;
				if(process_map.find(p) == process_map.end()) return;
				waitid(P_PID, p, &status, WEXITED);
				lasterr = status.si_status;
			}
			catch(...){
				cout << "Invalid process id" << endl;
			}
		}
	}
	else{
		processCommand();
	}
}

void subpid(){
	pid_t pid = getpid();
	vector<string> temp = TokenMaker(main_command, "$$");
	string tempcomm;
	if(temp.size() <= 1) return;
	
	for(int i = 0; i < temp.size()-1; i++){
		tempcomm = temp[i] + to_string(pid);
	}
	tempcomm = tempcomm + temp[temp.size()-1];
	main_command = tempcomm;
}

void suberr(){
	int err = lasterr;
	vector<string> temp = TokenMaker(main_command, "$?");
	string tempcomm;
	if(temp.size() <= 1) return;
	
	for(int i = 0; i < temp.size()-1; i++){
		tempcomm = temp[i] + to_string(err);
	}
	tempcomm = tempcomm + temp[temp.size()-1];
	main_command = tempcomm;
}


void EnterCommand()
{
    CmdRefresh();

    char input=' ';
    handle_premade_alarms();
    while(true)
    {
        read(STDIN_FILENO, &input, 1);
        switch(input)
        {
            case 27 :{
		char seq[3];
		
		if(read(STDIN_FILENO, &seq[0], 1) != 1) return;
		if(read(STDIN_FILENO, &seq[1], 1) != 1) return;
		
		if(seq[0] == '['){
			switch(seq[1]){
				case 'A':{
					prevcmd();
					break;
				}
				case 'B':{ 
					nextcmd();
					break;
				}
			}
		}
		
		break;
            }
            case 10 : {
                set_history(main_command);
                stack_update();
                if(record_flag) record_func(main_command);
                subpid();
                suberr();
                main_command = process_aliasing(main_command);
                cout << endl;
                parseCommand();
                main_command ="";
                break;
            }

            case 127 : {
                if(main_command!="")
                    main_command.pop_back();
                break;
            }
            case 9 :{
                Auto_Complete();
                break;
            }

            default :{
                main_command.push_back(input);
            }

        }

        CmdRefresh();

    }
}

//---------------------------------------------------------------------------------------------------------------------

void init()
{
    clearScreen();
    struct passwd *pw = getpwuid(getuid());

    const char *hmdir = pw->pw_dir;
    RootDir=string(hmdir);
    getcwd(CWD,256);
    gethostname(HOST, 256);
    USER = getlogin();
    HomeDir=string(CWD);
    ENV = "/home/"+USER+"/Documents/.myrc";
    FENV = "/home/"+USER+"/Documents/.filemaprc";
    HIST_SIZE=stoi(string(getenviron("HIST_SIZE")));
    hist_file_name = "/home/"+USER+"/Documents/.history";
    hist_temp_file_name = "/home/"+USER+"/Documents/.history_temp";
    record_dir = "/home/"+USER+"/Documents/record.txt";
    afname = "/home/"+USER+"/Documents/alarms.txt";

    commmand_trie_maker();

    tcgetattr(STDIN_FILENO,&orig_term); //current termios status saved
    NormalModeEnable();
    
    populate_alarm_map();
    populate_file_map();
    load_stack();
    
    signal(SIGCHLD, my_sigchld_handler);
}


int main(){
    init();
    EnterCommand();

    atexit(NormalModeDisable);
    update_alarm_file();
    return 0;
}
