#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
  int fd; //file descriptor
  int *p; //shmem pointer
  int shmkey = ftok("control.c",22);
  int semkey = ftok("control.c",4);
  int sd = shmget(shmkey,1024,IPC_CREAT|0664);
  int semid = semget(semkey,1,0);
  p=(int *)shmat(sd,0,0);
  int lsize = *p;
  //read last line and print it
  fd = open("story",O_RDONLY,0644);
  char lastline[lsize+1];
  if(lsize==0)printf("No previous messages.\n");
  else{
    lseek(fd,lsize*-1,SEEK_END); //move to last line
    read(fd,lastline,lsize);
    lastline[lsize]=0;
    printf("Previous message: %s\n",lastline);
  }
  close(fd);
  //down the semaphore
  struct sembuf sb;
  sb.sem_op=-1;
  sb.sem_num=0;
  sb.sem_flg=SEM_UNDO;
  semop(semid,&sb,1);
  //prompt for message
  printf("Enter a message: ");
  char input[1024];
  fgets(input,sizeof(input),stdin);
  input[strlen(input)]=0;
  fd = open("story",O_WRONLY|O_APPEND,0644);
  //write the message to story
  write(fd,input,strlen(input));
  close(fd);
  *p = strlen(input); //update shared memory value
  //up the semaphore
  sb.sem_op=1;
  semop(semid,&sb,1);
  shmdt(p);

  return 0;
}
