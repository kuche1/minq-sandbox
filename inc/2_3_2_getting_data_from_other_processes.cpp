
//////////////
////////////////
/////////////////// getting data for other processes
////////////////
/////////////

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

tuple<bool, string> process_get_fd_path(pid_t pid, int fd){

    ostringstream oss_path;
    oss_path << "/proc/" << pid << "/fd/" << fd;
    string path = oss_path.str();

    auto [failed, resolved_path] = resolve_path_at_cwd(path);
    if(failed){
        return make_tuple(true, "");
    }

    return make_tuple(false, resolved_path);
}
