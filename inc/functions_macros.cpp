
// generic

#define PATH_MAXLEN (4096+1) /* +1 for the ending \0 */

#define ERR_FAILED_CALL(to_what) { \
    cout << "Failed call to `" << to_what << "` in file `" << __FILE__ << "` at line " << __LINE__ << '\n'; \
    exit(1); \
}

// ptrace

#define PTRACE(...) { \
    if(ptrace(__VA_ARGS__)){ \
        ERR_FAILED_CALL("ptrace"); \
    } \
}

// `RW` means that we can both read and write to the register
// `R` means that the register is read-only
#if __WORDSIZE == 64
    #define CPU_REG_RW_SYSCALL_ID(regs) regs.orig_rax
    #define CPU_REG_R_SYSCALL_ARG0(regs) regs.rdi
    #define CPU_REG_R_SYSCALL_ARG1(regs) regs.rsi
    #define CPU_REG_R_SYSCALL_ARG2(regs) regs.rdx
    #define CPU_REG_R_SYSCALL_ARG3(regs) regs.r10
    #define CPU_REG_RW_SYSCALL_RET(regs) regs.rax // the return code of the syscall
#else
    #error Only 64bit is supported for now
#endif

// seccomp

#define SECCOMP_ATTR_SET(...) { \
    if(seccomp_attr_set(__VA_ARGS__)){ \
        ERR_FAILED_CALL("seccomp_attr_set"); \
    } \
}

// `-EACCES` means that the rule conflicts with the filter (for example, the rule action equals the default action of the filter)
#define SECCOMP_RULE_ADD(...) { \
    int ret = seccomp_rule_add(__VA_ARGS__); \
    if((ret != 0) && (ret != -EACCES)){ \
        ERR_FAILED_CALL("seccomp_rule_add"); \
    } \
}

#define SECCOMP_LOAD(...) { \
    if(seccomp_load(__VA_ARGS__)){ \
        ERR_FAILED_CALL("seccomp_load"); \
    } \
}

// exec

#define EXECVP(...) { \
    execvp(__VA_ARGS__); \
    ERR_FAILED_CALL("execvp"); \
}

// reading memory of other processes

string process_read_cstr_as_string(pid_t pid, char* addr){

    string str;

    for(;;){

        // read chunk of data

        char chunk[sizeof(long)];
        errno = 0;

        *(long*)chunk = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);

        if( (*(long*)chunk == -1) && (errno != 0) ){
            // the process has probably exited (or perhaps the address is wrong)
            cout << "Could not read data from process with pid " << pid << '\n';
            exit(1);
        }

        addr += sizeof(long);

        // process chunk data

        for(char ch : chunk){

            if(ch == 0){
                return str;
            }

            str += ch;
        }

    }

}

// handling of syscalls

// TODO we need to add the option to "permit and save" or "deny and save"
bool handle_syscall_openat(pid_t pid, int dir_fd, char *pidmem_filename, int flags, mode_t mode){

    // https://man7.org/linux/man-pages/man2/openat.2.html

    // read and sanitise parameter `path` from process memory
    // this can, in fact, be a file or a folder

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    if(dir_fd == AT_FDCWD){ // relative to CWD

        char resolved_path[PATH_MAXLEN];
        errno = 0;

        if(!realpath(path.c_str(), resolved_path)){

            if(errno == ENOMEM){
                cout << "Could not resolve path because of lack of buffer memory, please contact the developer\n";
                exit(1);
            }

            // probably file doesn't exist
            // deny syscall just in case

            return false;

        }

        path = string(resolved_path);
    
    }else{

        // TOD0
        cerr << "Not implemented yet, please contact the developer\n";
        exit(1);

    }

    // parse parameter `flags`

    // `flags` must include one of these: O_RDONLY (read only), O_WRONLY (write only), O_RDWR (read and write)
    // other flags can be bitwise OR-ed
    bool read_only = (flags | O_RDONLY);

    // allow reading libraries

    if(read_only){
        if(path.starts_with("/usr/lib/")){
            return true;
        }
    }

    // ask user if he wants to permit/deny the syscall request

    cout << '\n';
    cout << "Syscall request: filesystem: open\n";
    cout << "   path:" << path << '\n';
    cout << "   flags:" << flags << " [read-only:" << read_only << "]\n";
    cout << "   pid:" << pid << '\n';
    cout << "   mode:" << mode << '\n';
    // cout << '\n';

    for(;;){

        cout << "(p)ermit/(d)eny: ";

        string action;
        getline(cin, action);

        if(action == "d"){
            return false;
        }else if(action == "p"){
            return true;
        }else{
            cout << "Invalid action `" << action << "`\n";
        }
    }

}
