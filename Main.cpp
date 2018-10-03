#include <string>
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
using namespace std;

int main() {
  while (true){
    string command,curr; 
    // getline(cin, command);
// for file IO
    int fileIO = command.find('>'||'<');
    // if(fileIO!=0){
    //   if(command.find('|')!=0) cerr << "Invalid command";exit(EXIT_FAILURE);
    //   int fd;
    //   char[] *buf = {"file content"};
    //   fd = open("foobar.txt",O_CRAEATE|O_WRONLY);
    //   write(fd,buf,strlen(buf));close(fd);
    //   fd = open ("foobar.txt",O_RDONLY,0);
    //   read(fd,&c,1);
    //   printf("c=%c\n",c);
    //   close(fd);
    // }
    // else{

//PIPELINE
    vector<string> token;
    string input;
    // const char* pathname = _command.data();
      getline(cin, command);
      stringstream ss(command);
      // ss.ignore();

      cout << "command " << command.c_str() << endl;

      cout << "stringstream " << ss << endl;

      while(getline(ss, input,'|')){
        // input.ignore();
      cout<<"\n input: "<<input;
      token.push_back(input);
    }
    for(int i = 0;i<token.size();i++) cout <<endl<<"token"<< token.at(i)<<endl;
    // cout<<"After getline \n";
    int status;
    int fd1[2]; //int fd2[2];
    pipe(fd1);
        char *tokenList[16];
        for(int i=1;i<token.size();i++) *tokenList[i]=token[i];
        tokenList[token.size()-1]=NULL;
    pid_t pid = fork();
    cout << "\npid: "<<pid<<endl;
    // test with ls | wc-l
      if(pid==0){ //child

        dup2(fd1[1],1);
      
        cout<< "I'm child procs \n";
        cout << token.at(0)<<endl<<flush;
        char* pathname =token.at(0);
        execvp(pathname,tokenList);

      }
      else{ //parent
        close(fd1[1]);
        dup2(fd1[0],0);
        wait(NULL);
        // waitpid(pid,NULL,WNOHANG);
        // const char* pathname =token.at(1).data();
        char* pathname_next =token.at(1);
        cout<< "I'm parent procs \n";
        execvp(pathname_next,tokenList);
      }
    // }

  }
  return 0; 
  
}
