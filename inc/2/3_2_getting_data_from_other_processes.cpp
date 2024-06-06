
//////////////
////////////////
/////////////////// getting data for other processes
////////////////
/////////////

pair<bool, string> process_read_cstr_as_string(pid_t pid, char* addr){

    string str;

    for(;;){

        // read chunk of data

        char chunk[sizeof(long)];
        errno = 0;

        *(long*)chunk = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);

        if( (*(long*)chunk == -1) && (errno != 0) ){
            // the process has probably exited (or perhaps the address is wrong)
            return {true, "could not read cstring from process"};
        }

        addr += sizeof(long);

        // process chunk data

        for(char ch : chunk){

            if(ch == 0){
                return {false, str};
            }

            str += ch;
        }

    }

}

string process_get_fd_path(pid_t pid, int fd){

    ostringstream oss_path;
    oss_path << "/proc/" << pid << "/fd/" << fd;
    string path = oss_path.str();

    return resolve_path_at_cwd(path);
}

string process_get_cwd(pid_t pid){

    ostringstream oss_process_pwd_file;
    oss_process_pwd_file << "/proc/" << pid << "/cwd";
    string process_pwd_file = oss_process_pwd_file.str();

    return resolve_path_at_cwd(process_pwd_file);

}
