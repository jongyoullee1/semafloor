#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
void view();
void create();
void reremove();
/*union semun{
    int val;
    struct semid_ds *buff;
    unsigned long *buffer;
    struct seminfo *_buf;
}; */

void create(int shmkey,int semkey){
    //create shared memory
    int sd;
    sd = shmget(shmkey,1024,IPC_CREAT|IPC_EXCL|0664);
    if(sd==-1){
      printf("Resources already created, use -r or -v\n");
      exit(0);
    }
    int *p;
    //set shared mem initial value to 0
    p=(int *)shmat(sd,0,0);
    *p=0;
    shmdt(p);
    //create semaphore, set value to 1
    int semid;
    semid = semget(semkey, 1, IPC_CREAT|IPC_EXCL|0644);
    printf("Shared memory created.\n");
    union semun su;
    su.val = 1;
    semctl(semid, 0, SETVAL, su);
    printf("Semaphore created.\n");
    //create the file
    int fd = open("story",O_CREAT|O_TRUNC,0644);
    printf("Story file created.\n");
    close(fd);
}

void reremove(int shmkey,int semkey){
    //print the whole story and remove it
    printf("Contents of the story: \n");
    view();
    remove("story");
    printf("Story removed.\n");
    //remove shared mem
    int sd;
    sd = shmget(shmkey,1024,0);
    struct shmid_ds d;
    shmctl(sd,IPC_RMID, &d);
    printf("Shared memory removed.\n");
    //remove semaphore
    int semid;
    semid = semget(semkey,1,0);
    union semun su;
    semctl(semid,0,IPC_RMID,su);
    printf("Semaphore removed.\n");
}

void view(){
    //output contents of file
    int fd = open("story",O_RDONLY);
    if (fd ==-1){
      printf("Resources already removed. Create new ones.\n");
      exit(0);
    }
    int filesize = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    char s[filesize+1];
    read(fd,s,filesize);
    s[filesize]=0;
    close(fd);
    printf("%s\n",s);
}

int main(int argc, char *argv[]){
    int shmkey = ftok("control.c",22);
    int semkey = ftok("control.c",4);
    if(argc==2){
      if (strcmp(argv[1],"-c")==0) create(shmkey,semkey);
      else if (strcmp(argv[1],"-r")==0) reremove(shmkey,semkey);
      else if(strcmp(argv[1],"-v")==0) view();
      else printf("Error: Argument must be -r, -c, or -v\n");
    }
    else printf("Please include -c,-v or -r flag\n");
    return 0;
}
