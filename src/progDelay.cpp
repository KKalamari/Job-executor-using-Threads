//Αυτό το αρχείο ειναι από την εκφώνηση της εργασίας 
//απλά από c το έγραψα σε cpp

#include <iostream>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        std::cout << "Usage: " << argv[0] << " <number>" << std::endl;
        return 1;
    }
    int delay = std::atoi(argv[1]);
    if (delay <= 0) 
    {
        std::cout << "Invalid number: " << argv[1] << std::endl;
        return 1;
    }
    for (int i = 0; i < delay; i++) 
    {
        sleep(1);
        std::cout << "$" << std::flush;
    }
    std::cout << std::endl;
    return 0;
}