#include"server.h"

string tripletconverte(triplet job) 
{//take the job id, the job, and the queueposition , turn it into a big string
    std::string result = "jobid: " + job.jobID + " job: ";
    int i =0;
    while(job.job[i]!=NULL) 
    {
        result += job.job[i];
        i++;
    }
    //result += " queuepos: " + std::to_string(job.queuePosition);
    result += "\n"; // add a \n at te end so that when there are multiple triplets the commander prints them with a \n inbetween
    return result;
}
