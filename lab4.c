#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Creata a struct to store process information */
struct Process{
        pid_t pid;
        int voter_id;
};

int main(void)
{
        /* Step 1: Ask user to input number of voters and max number of rounds */
        int num_voters, num_rounds;
        printf("Enter the number of voters: ");
        scanf("%d", &num_voters);
        printf("Enter the number of rounds: ");
        scanf("%d", &num_rounds);

        /* Declare necessary variables */
        int fd[num_voters][2], i, nbytes;
        pid_t   parentPid;
        struct Process processes[num_voters];

        /* Step 2 (1st for loop): The judge process creates N pipes using pipe() system call. */
        for (i=0; i<num_voters; i++)
          pipe(fd[i]);

        /*
        Step 3 (2nd for loop):
        The judge process creates N child processes using fork() system call.  (Only the judge process can call
        fork and child processes don’t call fork.)

        NOTE:
        - At this step, each child process needs to keep the voter id (= the index of the loop = the pipe id)
        that  will  be  used  in  Step  4.  You may set the judge process’s voter id to be -1  to  differentiate  it
        from the voter processes.
        - The processes should not call exit() in this step. All processes should continue to Step 4.
        */
        parentPid = getpid();                   // get parentPid
        for (i = 0; i < num_voters; i++) {
            if (parentPid == getpid())          // parent process call fork
                processes[i].pid = fork();

            if (parentPid != getpid())          // child process gets the voter_id
                processes[i].voter_id = i;
          }

        /*Step 4 (3rd for loop): At each round,
        For the judge process (if the process’s voter id is -1):
        - (4th for loop) Collect votes from voters using read() system call
        - Output the majority voting result
        For each voter process (if the process’s voter id is between 0 and N):
        - Send  the  vote  to  the  judge  process  using  write()  system  call  (Voter Process i   uses  Pipe i to
        communicate with the judge, DO NOT use a loop here)
        */
        srand(time(NULL) - getpid());      // using srand to ensure the vote results are randonly generated
        for (i = 0; i < num_rounds; i++) {
            /* Declare necessary variables */
            int yes = 0, no = 0;      // to count the number of yes/no votes
            char* result;             // to assign with the vote string
            char arr[num_voters];     // to store all votes in each round
            char readbuffer[2];       // for parent and child to communicate

            for (int j = 0; j < num_voters; j++) {

                /* Parent process */
                if (parentPid == getpid()) {

                    close(fd[j][1]);        // close up output side of pipe
                    nbytes = read(fd[j][0], readbuffer, sizeof(readbuffer));    // read the vote from pipe
                    arr[j] = readbuffer[0];     // save the vote into arr

                    /* Accumulate the number of yes/no votes */
                    if (readbuffer[0] == 'Y') yes += 1;
                    else no += 1;

                    /* At the last voter, print out the whole thing of each round */
                    if (num_voters - 1 == j) {
                        printf("\n\n------ Round %d ------\n", i+1);

                        /* Print the vote of each voter */
                        for (int v = 0; v < num_voters; v++) {
                            if (arr[v] == 'Y') result = "YES";
                            else result = "NO";
                            printf("Round %d: Voter %d votes: %s\n", i+1, v, result);
                        }

                        /* Print the final result of each round */
                        if (yes > no) result = "YES";
                        else if (yes < no) result = "NO";
                        else result = "TIE";
                        printf("--- Round %d: The voting result is a %s\n", i+1, result);

                    }
                }

                /* Child process */
                if (parentPid != getpid()) {
                    /* Randomly generate the vote of each voter */
                    int r = rand();
                    if (r % 2 == 0)
                        readbuffer[0] = 'Y';
                    else
                        readbuffer[0] = 'N';


                    close(fd[j][0]);     // close up input side of pipe
                    write(fd[j][1], readbuffer, strlen(readbuffer));    // write the vote into pipe

               }
            }


        }


        return(0);
}

