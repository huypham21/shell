#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
using namespace std;

int main() {
  int fd;
  char* buf[] = "file content";
  fd = open("foobar.txt",O_CRAEATE|O_WRONLY);
  write(fd,buf,strlen(buf));close(fd);
  fd = open ("foobar.txt",O_RDONLY,0);
  read(fd,&c,1);
  printf("c=%c\n",c);
  close(fd);
  return 0; 
  
}