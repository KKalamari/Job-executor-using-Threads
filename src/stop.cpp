#include "server.h"

string stop(string job_id)
{
    int flag =0;
    queue<triplet> temp;
    while (the_one.size()!=0) 
    {//i iterate through the_one queue that contains the proccesses to be executed
    //i put the rest of the proccesses in a temp queue and then refill the_one after
    //finding the wanted jobid (if it is found)
        triplet current = the_one.front(); 
        the_one.pop(); 
        
        if (current.jobID == job_id ) //if you found the element set flag to 1 and break
        {   
            flag =1;
            break;
        } else 
        {
            temp.push(current); 
        }
    }

    //if the flag is 0 then it wasn't found in the_one queue, so it is invalid id
    if(flag ==0) 
    {   
        //re-push elements from the temp queue back to the_one queue so that the_one doesn't get lost
        while (temp.size()!=0) 
        {
            the_one.push(temp.front());
            temp.pop();
        }  
      
       return " NOTFOUND";//if the id isn't found in the vector and in the queue then the id doesn't exist
    }

    while (the_one.size()!=0)//change the queue position of the rest of the jobs in the queue 
    {
        triplet current = the_one.front(); 
        the_one.pop(); 

        current.queuePosition--;
        temp.push(current);
    }

    
    while (temp.size()!=0) //refill the_one queue so it doesn't get lost
    {
        the_one.push(temp.front());
        temp.pop();
    }

    return " REMOVED";

}