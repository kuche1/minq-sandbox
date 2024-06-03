
pid_t spawn_executable(char* executable, char** executable_args, bool networking_enable, bool filesystem_allow_all){

    pid_t child = fork();

    if(child < 0){

        ERR_FAILED_CALL("fork");

    }else if(child == 0){

        PTRACE(PTRACE_TRACEME, 0, NULL, NULL);

        raise(SIGSTOP);
        // pause execution since TRACEME won't do that by itself

        #include "spawn_executable/set_static_rules.cpp"

        EXECVP(executable, executable_args);
        // everything below this point should be unreachable

    }else{

        // wait for the SIGSTOP
        int status;
        waitpid(child, &status, 0);

        if(!WIFSTOPPED(status)){ // was child stopped by a delivery of a signal
            cerr << "Child was not stopped by a delivery of a signal\n";
            exit(1);
        }

        if(WSTOPSIG(status) != SIGSTOP){ // which was the signal that caused the child to stop
            cerr << "Child was stopped, but not due to SIGSTOP\n";
            exit(1);
        }

        // set some more restrictions

        PTRACE(
            PTRACE_SETOPTIONS,
            child,
            0,
            PTRACE_O_EXITKILL | // make sure to kill the child if the parent exits
            PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | // trace any new processes created by the child
            PTRACE_O_TRACEEXIT | // get notified when a process exits
            PTRACE_O_TRACESECCOMP // trace syscalls based on seccomp rules
        );

        PTRACE(PTRACE_CONT, child, NULL, NULL);

        return child;

    }

}
