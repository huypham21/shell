#include <string.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <map>

#include "shell.h"


using namespace std;

char* tokenList[30];

void handle_sigchld(int sig) {
  int saved_errno = errno;
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
  errno = saved_errno;
}

int redirect(char * arg){// argument after cd
	if (arg == NULL){// if theres nothing after cd
		chdir(getenv("HOME"));
	}
	else{
		const char* c= "-";// "-" after cd
		if(strcmp(c,arg) == 0){
			printf("Went up by one level directory\n");
			chdir("..");// This function goes up by level dir
			return 0;
			}
		else if(chdir(arg) == -1){
			printf("Error: Wrong directory path\n");
			return -1;
			}
		}
	return 0;
}


int main() {
    Controller proc;
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror(0);
        exit(1);
    }

 while (true){
    printf("$");
    string command,curr; 
    size_t currPos,begin=0;
    vector<string> token;
    string input;
    getline(cin, command,'\n');
    while (command == "\0"){//get back to promt if nothing was entered
			printf("$");
      getline(cin, command,'\n');
		}
        cin.clear();
            currPos = command.find("'");
        if ( currPos!= std::string::npos){
            command.replace(currPos,1,"");
            size_t found = command.find("'");
            curr = command.substr(currPos,found-currPos);//-currPos-1
            command.replace(currPos,found-currPos+1,"");//-currPos+1
        }


      stringstream ss(command);

    while(getline(ss, input,' ')){
      size_t prev = 0, pos;
      while ((pos = input.find_first_of("|<>", prev)) != string::npos)
      {
          
          if (pos==0)token.push_back(input.substr(pos,1));
          if (pos > prev ){
              token.push_back(input.substr(prev, pos-prev));
              token.push_back(input.substr(pos,1));
          }    
          prev = pos+1;
      }
      if (prev < input.length()) token.push_back(input.substr(prev, string::npos));
      if(input=="awk"||input=="echo") {token.push_back(curr);} 
    }
    bool cdFlag=1;
    for(int i = 0;i<token.size();i++) {
      tokenList[i]=(char*)token[i].data();
      if(strcmp(tokenList[i], "cd")==0 ){   //&& i<token.size()-1
			char * str = (char*)token[i+1].data();
			redirect(str);
            cdFlag=0;
		}
    }
        if (cdFlag){
                proc.parse(token);
                proc.run();
        }
        cin.clear();token.clear();command="\0";
  }           
return 0;
}
