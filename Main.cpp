#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
using namespace std;

int main() {
  while (true){
    string command,curr;
    getline(cin, command);
// for file IO
    int fileIO = command.find('>'||'<');
    if(fileIO!=0){
      if(command.find('|')!=0) cerr << "Invalid command";exit();
      int fd;
      char* buf[] = "file content";
      fd = open("foobar.txt",O_CRAEATE|O_WRONLY);
      write(fd,buf,strlen(buf));close(fd);
      fd = open ("foobar.txt",O_RDONLY,0);
      read(fd,&c,1);
      printf("c=%c\n",c);
      close(fd);
    }
    else{

//PIPELINE
    vector<string> token;
    stringstream ss(command);
    while(getline(curr, command,'|')){
      token.push_back(curr);
    }
    int fd1[2]; //int fd2[2];
    pipe(pd1);
    int pid = fork():
    // test with ls | wc-l
      if(pid==0){ //child
        dup2(fd1[1],1)
        execlp(token.at(0),token.at(0),NULL);

      }
      else{ //parent
        close(fd1[1]);
        dup2(fd1[0],0);
        waitpid(pid);
        execlp(token.at(1),token.at(1),token.at(2),NULL);
      }
    }

  }
  return 0; 
  
}
