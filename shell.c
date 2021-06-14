#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_CHAR 100//Maximum characters of a single command line

//
int processLine(char** args,int size)
{
    int ret=0;//return value
    //dynamically allocate the arguments
    char *myArgs[size];
    int i;
    for(i=0;i<size;i++)
    {
        myArgs[i]=strdup(args[i]);
    }
    //Add NULL to the end of the line
    myArgs[size]=NULL;
    //Start processing
    int rc=fork();
    if(rc<0)
    {
        printf("Fork Failed!");
        ret=1;
    }
    //Child process
    else if(rc==0)
    {
        //transform the process
        execvp(myArgs[0],myArgs);
        //This lines of code should not be executed in exec() is successful
        printf("\nexecvp() failed\n");
        ret=1;
        
    }
    //Parent process
    else
    {
        wait(NULL);
    }
    return ret;
}

//This function parses the command line string and passes the commands to be processed
void parseLine(char* line)
{
    char* args[MAX_CHAR];//The command line argument split by whitespace
    char* token;
    char* temp;
    int fd;
    int count=0;
    //Variables that keep track if redirection or pipes are in the command line arguments
    int pipe=0;
    //Split by whitespace
    token=strtok(line," ");
    while(token!=NULL)
    {
        //If the command is an input redirection
        if(strcmp(token,"<")==0)
        {
            temp=strtok(NULL," ");
            //If the last character of the word is \n
            if(temp[strlen(temp)-1]=='\n')
            {
                temp[strlen(temp)-1]='\0';
            }
            fd=open(temp,O_RDONLY|O_CLOEXEC);
            dup2(fd,STDIN_FILENO);
        }
        //If the command is an output redirection
        else if(strcmp(token,">")==0)
        {
            temp=strtok(NULL," ");
            //If the last character of the word is \n
            if(temp[strlen(temp)-1]=='\n')
            {
                temp[strlen(temp)-1]='\0';
            }
            fd=open(temp,O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC);
            dup2(fd,STDOUT_FILENO);
        }
        //If the command is a pipe
        else if(strcmp(token,"|")==0)
        {
            //TBC//////////
            pipe++;
        }
        else
        {
            args[count]=token;
            count++;
        }
        token=strtok(NULL," ");
    }
    //Trim the last word of the argument
    if(args[count-1][strlen(args[count-1])-1]=='\n')
    {
        args[count-1][strlen(args[count-1])-1]='\0';
    }
    //Process the line, if it fails give an error message
    if(processLine(args,count)==1)
    {
        printf("\nProcessing line %s ... %s Failed\n",args[0],args[count-1]);
    }
}

int main(int argc, char* argv[])
{
    assert(argc>0);
    FILE *fp=fopen(argv[1],"r");//open the file
    char line[MAX_CHAR];
    //Check if the file exists
    if(fp==NULL)
    {
        printf("\nFile %s does not exist\n",argv[1]);
        exit(EXIT_FAILURE);
    }
    while(fgets(line,MAX_CHAR,fp)!=NULL)
    {
        parseLine(line);
    }
    fclose(fp);//close the file
    
    
    
    return EXIT_SUCCESS;
}
