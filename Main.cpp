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

// #include <token.h>

using namespace std;

char* tokenList[30];

vector<string> special = {"|", ";",    // Pipe and sequence
                     ">>", ">",   // - stdout append and overwrite to file
                     "<"};        // - read from file
//tokenize command

// char * Tokenizer ( char *c ){//read the command line
// 	char *temp = c;
//     while ((*temp = getchar()) != '\n'){//temp goes through cmd until \n
//     	if(find(special.begin(), vector.end(), input)!=vector.end()) {
//         //call special handle
//         spec.push_back(input);
//       }
      
//       temp++;
//     }
//     *temp=0;
//     return c;
// }


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
int exec(char* cmd, char** cmd_array, int flag, char* inf, char* outf){
	pid_t pid = fork();
	if(pid == 0){//child
		if (flag==1){ //input from file
			    if(inf == NULL) printf("Error: Couldn't read from file!\n");
		  int inF ; //in fd is 0
				inF = open(inf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				dup2(inF , 0); //std in fd is 0
				close(inF);
			}
		else if (flag==2){ //output to file
    		int outF ; // out fd is 1
				if(outf == NULL) printf("Error: Couldn't write to file!\n");
				printf("\nWrote to file. \n");
				outF = open(outf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				
        dup2(outF , 1);
				close(outF);
			}
				execvp(cmd, cmd_array);
			}

	else if (pid>0) { //parent
		if(flag != 3) wait(NULL);// wait for the child then exit the loop to avoid zombie processes			
		}
	else if (pid == -1)printf("Child couldn'y be created.\n"); //error
	return 0;
}
  




void control(char **tokenList, int size){//char *command
	// parse(command, cmd_array);
	char * inFS = NULL;
	char * outFS = NULL;
	int flg = 0;
	int l;// length of cmd_array
	// for (l = 0 ; cmd_array[l] ; l++);
	bool exec_now = true;
	int i;
    char *command =tokenList[0];
    // cout<< "Command!: "<<command<<endl<<flush;
    for (i = 0 ; i<size ; i++){
      
		if(strcmp(tokenList[i], "cd")==0){    //strcmp) == 0
    
			char * str = tokenList[i+1];
			exec_now = false;
      //  cout<<"\nstr: "<<*str<<"\ntokenList[i + 1] "<<*tokenList[i + 1];
			redirect(str);
		}
	    if (strcmp(tokenList[i], "<")==0){ //go to input file tokenList[i] == '<'
	          inFS = tokenList[i + 1];// get filename if (i + 1 < l)  .data()
	          //  cout<<"\ntokenList[i + 1] "<<*tokenList[i + 1];
            flg = 1;
            command = tokenList[i-1];
            // cout<<"\ncommand in <: "<<*command <<"\n InFS: "<<*inFS;
	          tokenList[i] = NULL;
	          i++; //increment becase theres filename
	      }
      if (strcmp(tokenList[i], ">")==0) {//output to file

          outFS = tokenList[i + 1]; //get filename if (i + 1 < l) 
	        // cout<<"\ntokenList[i + 1]: "<<outFS<<endl;
          flg = 2;
          tokenList[i] = NULL;
          command = tokenList[i-1];
          i++; //increment because there's filename
      }
			// if(cmd_array[i][0] == '&'){//background process
			// 	flg = 3;
			// 	cmd_array[i] = NULL;
			// 	i++;
			// 	l--;
			// }
			// if(cmd_array[i][0] == '|'){//pipelining
			// 	exec_now = false;
			// 	pipelining(cmd_array);
			// 	return;
			// }
		}
		if(exec_now==true) {
      // cout<<"\nTRUE! \n\n";
			exec(command, tokenList, flg, inFS, outFS);
    }
}

int main() {
  while (true){
    printf("$");
    int tokenList_size = 0;
    string command,curr; 
    vector<string> token, spec;
    string input;
      getline(cin, command,'\n');
        // cin.ignore();   .ignore() 
    while (command == "\0"){//get back to promt if nothing was entered
			printf("$");
      getline(cin, command,'\n');
		}
      stringstream ss(command);
      // ss.ignore();

      // cout << "command " << command.c_str() << endl<<flush;
    while(getline(ss, input,' ')){
      // while(getline(input,command,"|")){
      //   cout<<"\n input: "<<input;
      //   token.push_back(input);
      // }
      size_t prev = 0, pos;
      while ((pos = input.find_first_of("|<>", prev)) != string::npos)
      {
        // if( pos==0 ) token.push_back(input.substr(pos,1));//&& input.substr(pos,1)=="|"
          if (pos==0)token.push_back(input.substr(pos,1));
          if (pos > prev ){
              token.push_back(input.substr(prev, pos-prev));
              // cout<< "\nspecial tokens: " <<input.substr(pos,1);
              token.push_back(input.substr(pos,1));
          }    
          prev = pos+1;
      }
      if (prev < input.length())
          token.push_back(input.substr(prev, string::npos));
    }
    for(int i = 0;i<token.size();i++) {
      // cout <<endl<<"token "<< token.at(i)<<endl;
      tokenList[i]=(char*)token[i].data();
    }
//////////////////////////////////////////////////
    for(int i=0;i<token.size();i++) tokenList[i]=(char*)token[i].data();//cout<<tokenList[i]<<endl;}//tokenList[i]=
    tokenList[token.size()]=NULL; tokenList_size = token.size(); 
    cin.clear();
    control(tokenList,tokenList_size);

		if(token[0]== "exit") {
			exit(0); // if "exit" is typed, exit shell
			printf("Shell terminated.\n");
		}
		if(token[0]== "clear") system("clear"); // clear 

      cin.clear();token.clear();command="\0";
  }
  return 0; 
  
}












      // while(command.find(specialToken)>0)


    // const char* pathname = _command.data();

    // int fileIO = command.find('>'||'<');

      // if(find(special.begin(), vector.end(), input)!=vector.end()) {
      //   //call special handle
      //   spec.push_back(input);
      // }
      // else{
      // }



// for file IO


//PIPELINE


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


    // int status;
    // int fd1[2]; //int fd2[2];
    // pipe(fd1);
    //     for(int i=1;i<token.size();i++) tokenList[i]=(char*)token[i].data();//tokenList[i]=cout<<<<endl
    //     tokenList[token.size()-1]=NULL;
    // pid_t pid = fork();
    // cout << "\npid: "<<pid<<endl;
    // // test with ls | wc-l
    //     cout<< "I'm child procs \n";
    //     const char* pathname =token.at(0).data();
    //     cout << pathname<<endl<<flush;
    //     // cout
    //     execvp(pathname,tokenList);
    //   if(pid==0){ //child

    //     dup2(fd1[1],1);
      

    //   }
    //   else{ //parent
    //     close(fd1[1]);
    //     dup2(fd1[0],0);
    //     wait(NULL);
    //     // waitpid(pid,NULL,WNOHANG);
    //     // const char* pathname =token.at(1).data();
    //     const char* pathname_next =token.at(1).data();
    //     cout<< "I'm parent procs \n";
    //     execvp(pathname_next,tokenList);
    //   }
    // }


// int exec(char* cmd, char** cmd_array, int flag, char* inf, char* outf){
// 	pid_t pid = fork();
// 	if(pid == 0){//child
// 		int inF = 0; //in fd is 0
// 		int outF = 1; // out fd is 1
// 		if (flag==1){ //input from file
// 			    if(inf == NULL) cout<<"Error: Couldn't read from file!\n";
// 				inF = open(inf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
// 				dup2(inF , 0); //std in fd is 0
// 				close(inF);
// 			}
// 		else if (flag==2){ //output to file
// 				if(outf == NULL) cout<<"Error: Couldn't write to file!\n";
// 				cout<<"Wrote to file.\n";
// 				outF = open(outf, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
// 				dup2(outF , 1);
// 				close(outF);
// 			}
// 				execvp(cmd, cmd_array);
// 			}

// 	else if (pid>0) { //parent
// 		if(flag != 3) wait(NULL);// wait for the child then exit the loop to avoid zombie processes			
// 		}
// 	else if (pid == -1)cout<<"Child couldn'y be created.\n"; //error
// 	return 0;
// }