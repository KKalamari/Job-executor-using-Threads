#include "server.h"

int threadPoolSize;//the variables wanted from the HW2 presentation
int bufferSize;

queue<triplet> the_one;//queue where i keep the jobs 
vector<triplet> running; // vector that keeps all the running processes
int jobs = 0;            // how many jobs were given in total
int concurrency = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t worker_signal = PTHREAD_COND_INITIALIZER;//signal for the worker threads
pthread_cond_t buffer_full = PTHREAD_COND_INITIALIZER;//signal if the buffer is full
bool the_end_has_come = false; // flag to make the while loop in the main thread break

void resize() // fuction that is called after the execution of a job that corrects the queue positin for the jobs in the quweue
{
    for (size_t i = 0; i < the_one.size(); ++i)
    {
        triplet t = the_one.front();
        the_one.pop();

        t.queuePosition--; // it just decreasess by 1 the queueposition
        the_one.push(t);
    }
}

void sigchld_handler(int signum) // handler for the sigchd that removes the process from the running vector and changes the flag2
{
    pid_t pid;

    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) // waitpid return the pid of the process that ended
    {
        for (unsigned int i = 0; i < running.size(); i++)
        {
            if (running[i].pid == pid) // find the pid into the running vector and erase it
            {

                running.erase(running.begin() + i);
            }
        }
    }
    pthread_cond_signal(&worker_signal);//send a signal to the worker that a job has finished
}

void *controller_thread(void *arg)
{
    int newsock = *(int *)arg;

    string message;//server's response for the commander
    char buffer[1024] = {0};//where i save the pipe data
    char *args[1024] = {0};// where i save the buffer after tokenization with spaces
    size_t num_tokens = 0;//to count how many tokens the buffer has

    ssize_t bytes_read;

    bytes_read = read(newsock, buffer, sizeof(buffer) - 1);//read what is writen but leave space for a NULL last char
    if (bytes_read <= 0)
    {
        cout << "Error reading from socket." << endl;
        close(newsock);
        return nullptr;
    }
    buffer[bytes_read] = '\0';//null terminator sor the strtok

    char *token = strtok(buffer, " ");
    while (token != NULL)//loop that takes the tokens from the buffer. the idea was taken from a commnt in https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
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
                pthread_mutex_lock(&mutex);
                if (bufferSize < (int)the_one.size())
                {
                    pthread_cond_wait(&buffer_full, &mutex);
                }
                triplet job = issueJob(args);
                message = tripletconverte(job);
                if (write(newsock, message.c_str(), message.size()) == -1)
                {
                    cout << "something went wrong\n"
                        << endl;
                }
                pthread_cond_signal(&worker_signal);
                pthread_mutex_unlock(&mutex);
                break;
            }
        
        case 's':
            if (strcmp(args[0], "stop") == 0)///if its stop call the stop function
            {
                string jobid = args[1];
                string result = jobid + stop(jobid);
                message = result ;
                break;
            }
            else if (strcmp(args[0], "setConcurrency") == 0)//if its setconcurrency cal;l the setconcurrency function
            {
                int new_con = atoi(args[1]);
                if (new_con < 1)//if the wanted concurrency is < 1 return invalid message
                {
                    message = "invalid concurrency";
                }
                else
                {
                    setConcurrency(new_con);
                    message = "concurrency set " + string(args[1]);
                }
                break;
            }
            
        case 'p':
            if (strcmp(args[0], "poll") == 0)//if its poll call the poll function with running/queued, and the tripletconverter
            {
                vector<triplet> result = poll(args[1]);
                if (!result.empty())
                {
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        message += tripletconverte(result[i]);
                    }
                }
                else
                {
                    message = "no job queued";
                }
                break;
            }
            
        case 'e':
            if (strcmp(args[0], "exit") == 0) // if it's the exit exit
            {
                message = "jobExecutorServer terminated.";
                if (write(newsock, message.c_str(), message.size()) == -1) //write the message for the commander
                {       cout << "something went wrong\n"
                        << endl;
                }
                for (unsigned int i = 0; i < num_tokens; i++)//free the memory for each args token because i used strdup
                {
                    free(args[i]);
                }
                while (!the_one.empty())//empty the queue so that workers wont take new jobs
                {
                    the_one.pop();
                }
                the_end_has_come = true; //flag so that the main threads's loop breaks
                close(newsock);
                return nullptr;
            }
            
        default:
            message = "unknown command";
            break;
    }
    if( write(newsock , message.c_str() , message.size() )==-1)//send the results to the commander
        cout<<"something went wrong\n"<<endl; 
    message.clear();
    close(newsock);
    return nullptr;
}

void *worker_thread(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&mutex);

        while (the_one.empty() == true)//if the job queue is empty wait for the commander to signal it gave a new job
        {
            pthread_cond_wait(&worker_signal, &mutex);
        }
        while ((int)running.size() < concurrency)
        {
            triplet job = the_one.front();//the job to be executed
            
            pthread_cond_signal(&buffer_full);
            pid_t pid = fork();
            if (pid == -1)
            {
                cout << "fork failed." << endl;
                continue;
            }
            else if (pid == 0)
            {
                if (execvp(job.job[0], job.job) == -1)//execute the job
                {
                    perror("failed to execute command");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                job.pid = pid;//save the pid 
                running.push_back(job);//insert the job into the running vector
                resize(); //fix the queue position for the rest of the jobs
                the_one.pop();//remove the job from the queue
            }
        }

        pthread_mutex_unlock(&mutex);//unlock the mutex when you done
    }
    return nullptr;
}

void *main_thread(void *arg)
{
    int portnum = *(int *)arg;
    int sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof(client);

    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "socket" << endl;
        return nullptr;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portnum);

    if (bind(sock, serverptr, sizeof(server)) < 0)
    {
        cout << "bind" << endl;
        close(sock);
        return nullptr;
    }

    if (listen(sock, 5) < 0)
    {
        cout << "listen" << endl;
        close(sock);
        return nullptr;
    }

    cout << "Listening for connections to port " << portnum << endl;

    while (true)
    {
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
        {
            cout << "accept" << endl;
            continue;
        }
        if (the_end_has_come)
        {
            close(newsock);
            break;
        }

        pthread_t controller, worker;
        int *newsock_ptr = (int *)malloc(sizeof(int));
        *newsock_ptr = newsock;
        if (pthread_create(&controller, nullptr, controller_thread, newsock_ptr) != 0)
        {
            cout << "pthread_create" << endl;
            free(newsock_ptr);
            close(newsock);
            continue;
        }

        for (int i = 0; i < threadPoolSize; ++i)
        {
            if (pthread_create(&worker, nullptr, worker_thread, nullptr) != 0)
            {
                cout << "pthread_create" << endl;
            }
        }

        if (the_end_has_come)
        {
            //pthread_detach(controller);
            break;
        }

        pthread_detach(controller);
        free(newsock_ptr);
    }

    close(sock);
    return nullptr;
}

int main(int argc, char *argv[])
{
    if (argc != 4)//4 arguments as the ekfonisi says
    {
        cout << "give exactly 4 arguments please" << endl;
        exit(EXIT_FAILURE);

    }

    //the wanted arguments
    int portnum = atoi(argv[1]);
    bufferSize = atoi(argv[2]);
    threadPoolSize = atoi(argv[3]);

    signal(SIGCHLD, sigchld_handler);//signal for when a thread is done with a job

    pthread_t for_main_thread;
    //create the main thread
    if (pthread_create(&for_main_thread, nullptr, main_thread, &portnum) != 0)
    {
        cout << "pthread_create" << endl;

    }
    //join the main thread
    pthread_join(for_main_thread, nullptr);

    return 0;
}
