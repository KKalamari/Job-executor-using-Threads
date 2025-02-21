#include"server.h"

triplet issueJob(char * args[]) 
{
    triplet job;

    job.jobID = "job_" + to_string(++jobs); //the id is job_ and the number of the jobs that has been given yet plus 1      
    //cout<<"hereher\n" << endl;

    job.queuePosition= the_one.size();//the queueposition is the size (i use resize to fix the position when a job is either stoped or executed)

    int j;
    for (j = 1; args[j] != NULL; ++j)
    {
        job.job[j-1] = args[j];//give the job and the arguments to the job.job array from the args array, i don't need the "issujob"
    }
    job.job[j-1] = NULL;//null terminate for the execvp
    the_one.push(job);// push the jop into the queue

    return job;

}