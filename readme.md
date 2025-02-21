Job Executor System

This project implements a multi-threaded job execution system for Unix, designed as part of a Systems Programming assignment. The system consists of two main components:

    jobCommander: A client that interacts with the job execution server, sending commands and receiving responses.
    jobExecutorServer: A server that manages job execution, handling concurrency, job queuing, and worker threads.

Features

    Concurrency Control: Limits the number of active jobs via setConcurrency.
    Job Management: Supports adding, polling, stopping, and executing jobs asynchronously.
    Interprocess Communication (IPC): Utilizes pipes and signals for communication.
    Process Management: Uses fork(), execvp(), and waitpid() for process execution.
    Signal Handling: Implements SIGUSR1 for client-server signaling and SIGCHLD for child process cleanup.

Commands

The system supports the following commands:

    issueJob <command> – Submits a job for execution.
    setConcurrency <N> – Sets the maximum number of concurrent jobs.
    stop <jobID> – Cancels a queued or running job.
    poll – Lists queued jobs.
    exit – Shuts down the server.

Installation & Usage

Build the project:

    make

Run the server:

    ./bin/jobExecutorServer

Run a client command:

    ./bin/jobCommander issueJob "ls -l"

