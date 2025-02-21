#include "server.h"

vector<triplet> poll(const char* situation)
{

    {//iterate through the_one queue and return a vector with the queued jobs
        vector<triplet> result;
        queue<triplet> q;
        while (!the_one.empty())  //pop from the_one queue into the helper q and store into the result vector
        {
            result.push_back(the_one.front());

            q.push(the_one.front());            
            the_one.pop();
        }
        while(q.size()!=0) //refill the_one queue
        {
            the_one.push(q.front());
            q.pop();
        }

        return result;
    }
 
}