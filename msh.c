
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"  // We want to split our command line up into tokens
                            // so we need to define what delimits our tokens.
                            // In this case  white space
                            // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports 11 arguments

  int pidlist[15];
  int pid_index = 0;


  static void handle_signal (int sig )
  {

  }

  int main()
  {

    char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

    char history[50][10];
    int histindex = 0;
    int idx;
    while( 1 )
    {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;
    int token_check;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      token_check = token_count;

      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
      }

      struct sigaction act;
  /*
    Zero out the sigaction struct
  */
  memset (&act, '\0', sizeof(act));

  /*
    Set the handler to use the function handle_signal()
  */
  act.sa_handler = &handle_signal;

  /*
    Install the handler for SIGINT and SIGTSTP and check the
    return value.
  */
  if (sigaction(SIGINT , &act, NULL) < 0)
  {
    perror ("sigaction: ");
    return 1;
    }

    if (sigaction(SIGTSTP , &act, NULL) < 0)
    {
      perror ("sigaction: ");
      return 1;
    }

   //replacing left over spaces in token to NULL to be passed
   //in exec later

    while(token_count <= MAX_NUM_ARGUMENTS)
    {
      token[token_count] = NULL;
      token_count ++ ;
    }


    //diffrent ifs to check each commands and how they wil behave
    if(token[0] == NULL)
    {
      continue;
    }

    int command_stat = 0;

    int i;

    //save every command from user to history to be later
    //used if user ever wants the history command
    strcpy(history[histindex],token[0]);
    histindex++;

    if(strcmp(token[0],"exit")==0)
    {
      printf("%s\n",token[0]);
     exit(0);
    }

    if(strcmp(token[0],"quit")==0)
    {
      printf("%s\n",token[0]);
     exit(0);
    }

    if(strcmp(token[0],"history")==0)
    {
      for( i =0;i<histindex;i++)
      {
        printf("%d: %s\n",i,history[i]);
        command_stat = 1;
      }

    }

    //temporarily save something in a string and repalce
    //it with number from history command so when user wants
    //to run old commands it is easier to convert a string to a number
    int numberofprocess;
    char stringchar [3] = "te";

    if(token[0][0]=='!'){
     stringchar[0] = token[0][1];
     stringchar[1] = token[0][2];
     numberofprocess = atoi(stringchar);

     strcpy(token[0],history[numberofprocess]);
     command_stat = 1;

    }

    if(strcmp(token[0],"listpids")==0)
    {
      for( i=0;i<pid_index;i++)
      {
        printf("%d: %d\n",i,pidlist[i]);
      }
      command_stat = 1;
    }

    if(strcmp(token[0],"bg")==0)
    {
    kill(pidlist[pid_index],SIGINT);
    command_stat = 1;
    }

    pid_t child_pid = fork();

    pidlist[pid_index] = child_pid;
    pid_index++;

    int cd_status = 0;


    if(strcmp(token[0],"cd")==0)
    {
     chdir(token[1]);
     cd_status = 1;
     //continue;
    }

    int status;
    int exe = 0;

    //search four diffrent paths for a command if not found
    //skip down to the bottom if

    if(child_pid == 0)
    {
      char propath [50] = "/";
      strcat(propath,token[0]);

      if((execl(propath, token[0] , token[1], token[2], token[3],token[4]
        ,token[5],token[6],token[7],token[8],token[9], token[10], NULL)) == -1)
          {
            exe++;
          }
      char propath2 [50] = "usr/local/bin/";
      strcat(propath2,token[0]);

      if((execl(propath2, token[0] , token[1], token[2], token[3],token[4]
         ,token[5],token[6],token[7],token[8],token[9], token[10], NULL)) == -1)
       {
            exe++;
          }

      char propath3 [50] = "/usr/bin/";
      strcat(propath3,token[0]);

      if((execl(propath3, token[0] , token[1], token[2], token[3],token[4]
        ,token[5],token[6], token[7],token[8] ,token[9],token[10],NULL)) == -1)
          {
            exe++;
          }

      char propath4 [50] = "/bin/";
      strcat(propath4,token[0]);

      if((execl(propath4, token[0] , token[1], token[2], token[3],token[4]
        ,token[5],token[6], token[7],token[8] ,token[9], token[10],NULL)) == -1)
          {
            exe++;
          }

      //if command is in none of the paths or cd hasnt beeen run and any other
      //commands werent called print command not found otherwise skip this if
      if(exe==4 && cd_status==0 && command_stat == 0)
      {
      printf("%s: Command not found.\n",token[0]);
      }

    }

    waitpid(child_pid, &status, 0);

    free( working_root );
    strcpy(history[idx],token[0]);
    idx++;
  }

  return 0;

}
