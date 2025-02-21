#include "server.h"
void perror_exit(const char *message);

int main(int argc, char *argv[])
{
    int port, sock;
    char buf[512];

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port> <message>" << std::endl;
        exit(EXIT_FAILURE);
    }

    //create socket
    if ((sock = socket(SO_REUSEADDR, SOCK_STREAM, 0)) < 0)
    {
        perror_exit("socket");
    }

    //find server address
    if ((rem = gethostbyname(argv[1])) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]);//convert port number to integer
    server.sin_family = AF_INET;// internet domain
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port); // server port

    //initiate connection
    if (connect(sock, serverptr, sizeof(server)) < 0)
    {
        perror_exit("connect");
    }

    cout << "Connecting to " << argv[1] << " port " << port << endl;

    //build the command string from the remaining arguments
    string command = argv[3];
    for (int i = 4; i < argc; ++i)
    { //start from argv[4] to the end
        command += " ";
        command += argv[i];
    }

    //send the command string
    if (write(sock, command.c_str(), command.size()) < 0)
    {
        perror_exit("write");
    }

    //reeceive the servers answers
    memset(buf, 0, sizeof(buf));
    if (read(sock, buf, sizeof(buf) - 1) < 0)
    {
        perror_exit("read");
    }

    cout << buf << endl;

    close(sock);//close socket and exit

    return 0;
}

void perror_exit(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
