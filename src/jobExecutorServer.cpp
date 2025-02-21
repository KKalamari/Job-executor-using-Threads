#include "server.h"

queue<triplet> the_one;
vector<triplet> running; //vector that keeps all the running processes
int jobs = 0; //how many jobs were given in total
int concurrency =1; 

int flag = 0; //when flag is 1 then the server reads from pipe


void receivesignal(int signum) //handler for sigaction for the read signal(SIGUSR1)
{
    flag = 1;
}

void resize() //fuction that is called after the execution of a job that corrects the queue positin for the jobs in the quweue
{
    for (size_t i = 0; i< the_one.size(); ++i) 
    {
        triplet t = the_one.front();
        the_one.pop(); 

        t.queuePosition--;//it just decreasess by 1 the queueposition
        the_one.push(t);
    }
}

void sigchld_handler(int signum) // handler for the sigchd that removes the process from the running vector and changes the flag2
{
    pid_t pid;

    while ((pid= waitpid(-1, 0,WNOHANG))> 0) //waitpid return the pid of the process that ended
    {    
        for (unsigned int i= 0; i <running.size(); i++) 
        {
            if (running[i].pid_for_kill == pid) //find the pid into the running vector and erase it
            {

                running.erase(running.begin() + i);
                cout << "Process removed from the vector" << endl;
            }
        }
        
    }
}

int main() 
{
    int pipe1_fd;
    int pipe2_fd;

    mkfifo("pipe1", 0666);
    mkfifo("pipe2", 0666);

    pipe1_fd =open("pipe1", O_RDWR);//open the pipes for talking
    pipe2_fd = open("pipe2", O_RDWR);
    
    //create txt file and write into it the server pid
    FILE* file = fopen("jobExecutorServer.txt", "w");
    fprintf(file, "%d\n", getpid());
    fclose(file);

    static struct sigaction act;//sigaction for signal that changes the reading flag
    act.sa_handler = receivesignal;
    sigfillset(&(act.sa_mask));

    sigaction(SIGUSR1, &act , NULL);

    static struct sigaction for_kid;//sigaction for the sgichd signal
    for_kid.sa_handler = sigchld_handler;
    sigfillset(&(for_kid.sa_mask));

    sigaction(SIGCHLD, &for_kid , NULL);

    while(true)//ininite while loop for the server
    {       
        string message;//server's response for the commander
        char buffer[1024] ={0};//where i save the pipe data
        char* args[1024]={0}; // where i save the buffer after tokenization with spaces
        size_t num_tokens = 0;//to count how many tokens the buffer has

        if (flag == 1) //if flag ==1 then the commander has written something in the pipe
        {
            ssize_t bytes_read;

            bytes_read = read(pipe1_fd, buffer, 1024 -1);//read what is writen but leave space for a NULL last char
            if (bytes_read <= 0) 
            {
                fprintf(stderr, "error reading from pipe.\n");
                return 1;
            }
            buffer[bytes_read] = '\0';//null terminator sor the strtok
           
            char* token = strtok(buffer, " ");
            while (token != NULL) //loop that takes the tokens from the buffer. the idea was taken from a commnt in https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
            {
                args[num_tokens] = strdup(token);//i'm using strdup otherwise the token pointers will point to the buffer
                num_tokens++;
                token = strtok(NULL, " ");
            }

            args[num_tokens] = NULL;//null end the array for the execvp
            switch (args[0][0])//main switch case for the diferent function calls where i check the first letter of the first word to decide wich case i should go
            {
                case 'i':
                    if (strcmp(args[0], "issueJob") == 0)//if its issuejob call the issuejob function and the tripletconverter
                    {
                        triplet job;
                        job = issueJob(args);
                        message = tripletconverte(job);
                        break;
                    }
                case 's':
                    if (strcmp(args[0], "stop") == 0) //if its stop call the stop function
                    {
                        string jobid = args[1];
                        string result = jobid + stop(jobid);
                        message = result ;
                        break;
                    }
                    if (strcmp(args[0], "setConcurrency") == 0)//if its setconcurrency cal;l the setconcurrency function
                    {
                        int new_con = atoi(args[1]);//if the wanted concurrency is < 1 return invalid message
                        if(new_con < 1)
                        {
                            message = "invalid concurrency";
                            break;
                        }
                        setConcurrency (new_con);
                        message = "end";        
                        break;
                    }
                case 'p':
                    if (strcmp(args[0], "poll") == 0) //if its poll call the poll function with running/queued, and the tripletconverter
                    {
                        vector<triplet> result= poll(args[1]);
                        if(result.size()!=0) 
                        {
                            for(size_t i =0; i<result.size(); i++)
                            {
                                string tripletresult = tripletconverte(result[i]);
                                message += tripletresult;
                            }                                
                        } else
                        {
                            message = "no job running or queued";
                        } 
                        break;
                    }
                case 'e':
                    if (strcmp(args[0], "exit") == 0) // if it's the exit call the exit function 
                    {                            
                        remove("jobExecutorServer.txt");
                        message = "jobExecutorServer terminated.";           
                        if( write(pipe2_fd , message.c_str() , message.size() )==-1) //write the message for the commander
                            cout<<"something went wrong\n"<<endl;   

                        for (unsigned int i = 0; i < num_tokens; i++) {//free the memory for each args token because i used strdup
                            free(args[i]); 
                        }

                        close(pipe1_fd);//close the pipes
                        close(pipe2_fd);

                        return 0;
                    }
                default://if it's none of them send edn message to the commander 
                    message = "end";
                    break;
            }


                           
            if( write(pipe2_fd , message.c_str() , message.size() )==-1)//send the results to the commander
                cout<<"something went wrong\n"<<endl; 
            message.clear();
            flag =0;  //set the flag to 0 after done reading  

        }    

        while (the_one.size()!= 0) //while there are jobs in the queue
        {   
            int i = running.size();//set variable i to running size vecotr, so that i can control the concurrency correctly
            while (i < concurrency ) {//if the running jobs are less than the concurrency and the sigchld flag is 0 , you can start another job
                triplet job = the_one.front();//the job to execute
                pid_t pid = fork(); 

                if (pid == -1) 
                {
                    std::cerr << "fork failed." << std::endl;
                    return 1;
                } else if (pid == 0) 
                {
                    if (execvp(job.job[0], job.job) == -1) //execute the job
                    {
                        perror("failed to execute command");
                        exit(EXIT_FAILURE);
                    }
                } 
                job.pid_for_kill = pid; //save the pid into the job.pid_for_kill variable
                running.push_back(job);//push job into the running vectors
                i++; //increase 1 
                the_one.pop();//remove the job from the queue
                resize();//fox the queueposition of the rest of the jobs
                if (flag == 1)//if flag1==1 it's time to read again
                    break;

            }

            if (flag == 1)//if flag1==1 it's time to read again
                break;        
        }
    }
}
