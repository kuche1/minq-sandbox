
bool is_node_allowed(Sandbox_settings& settings, string path){

    // check already existing filters

    {

        // this is bad but it's good enough
        string path_fucked = path;
        if(!path_fucked.ends_with("/")){
            path_fucked += "/";
        }

        for(string allowed_path : settings.filesystem_allowed_nodes){

            // this is bad but it's good enough
            if(!allowed_path.ends_with("/")){
                allowed_path += "/";
            }

            // `path` and `allowed_path` are the same node

            if(path == allowed_path){
                return true;
            }

            // `path` is a file within `allowed_path`

            if(path.starts_with(allowed_path)){
                return true;
            }

        }

    }

    // check hardcoded filters

    {
        // allow generic stuff

        if(path == "/dev/tty"){ // using the terminal (as in printing, not executing commands)
            return true;
        }

        if(path == "/dev/null"){ // the "nothing" file
            return true;
        }

        if(path.starts_with("/usr/lib/")){ // libraries
            return true;
        }

        if(path == "/etc/ld.so.cache"){ // linker
            return true;
        }

    }

    // interactively decide

    {

        // check if interactive mode is even enabled

        if(!settings.filesystem_ask){
            return false;
        }

        // allow/deny if found in the "permanent" lists

        static vector<string> permanent_whitelist_path;
        static vector<string> permanent_blacklist_path;

        static vector<string> permanent_whitelist_path_prefix;
        static vector<string> permanent_blacklist_path_prefix;

        if(vec_contains(permanent_whitelist_path, path)){
            return true;
        }else if(vec_contains(permanent_blacklist_path, path)){
            return false;
        }

        for(string& prefix : permanent_whitelist_path_prefix){
            if(path.starts_with(prefix)){
                return true;
            }
        }

        for(string& prefix : permanent_blacklist_path_prefix){
            if(path.starts_with(prefix)){
                return false;
            }
        }

        // ask user if he wants to permit/deny the syscall request

        // TODO we could make this colorful
        cout << "Syscall request: filesystem: open `" << path << "`\n";

        for(;;){

            cout << "(a):allow / (d):deny / (pa):permanently-allow / (pd):permanently-deny / (pap):permanently-allow-prefix / (pdp):permanently-deny-prefix > ";

            string action;
            getline(cin, action);

            if(action == "a"){
                return true;

            }else if(action == "d"){
                return false;

            }else if(action == "pa"){
                permanent_whitelist_path.push_back(path);
                return true;

            }else if(action == "pd"){
                permanent_blacklist_path.push_back(path);
                return false;

            }else if(action == "pap"){
                cout << "Enter the prefix that you want to permanently allow: ";

                string prefix;
                getline(cin, prefix);

                if(!path.starts_with(prefix)){
                    cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                    continue;
                }

                permanent_whitelist_path_prefix.push_back(prefix);
                return true;

            }else if(action == "pdp"){
                cout << "Enter the prefix that you want to permanently deny: ";

                string prefix;
                getline(cin, prefix);

                if(!path.starts_with(prefix)){
                    cout << "Path `" << path << "` doesn't start with prefix `" << prefix << "`\n";
                    continue;
                }

                permanent_blacklist_path_prefix.push_back(prefix);
                return false;

            }else{
                cout << "Invalid action `" << action << "`\n";
                continue;
            }
        }
    }

}

//////////////
////////////////
/////////////////// handling of syscalls
////////////////
/////////////

pair<bool, string> handle_syscall_openat(Sandbox_settings& settings, pid_t pid, int dir_fd, char *pidmem_filename, __attribute__((unused)) int flags, __attribute__((unused)) mode_t mode){

    // https://man7.org/linux/man-pages/man2/openat.2.html

    constexpr bool ret_value_if_cant_resolve_path = true;
    // probably file doesn't exist
    // denying the syscall would be the safer choice, however I've seen apps break because of this
    // as the app crashes thinkin that the OS doesnt support the `open` syscall (as of writing this the
    // method for invalidating syscalls that we use is to ivalidate the syscall id) (example: python3)

    string txt_cant_resolve_path = "cannot resolve path: ";
    string txt_cant_get_fd = "cannot determine FD related to: ";
    string txt_allowed_path = "permitted: ";

    // read and sanitise parameter `path` from process memory
    // this can, in fact, be a file or a folder

    string path = process_read_cstr_as_string(pid, pidmem_filename);

    if(dir_fd == AT_FDCWD){ // relative to CWD

        auto [failed, resolved_path] = resolve_path_at_cwd(path);
        if(failed){
            return make_pair(ret_value_if_cant_resolve_path, txt_cant_resolve_path + path);
        }
        path = resolved_path;
    
    }else{

        if(path.starts_with("/")){

            auto [failed, resolved_path] = resolve_path_at_cwd(path);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, txt_cant_resolve_path + path);
            }
            path = resolved_path;

        }else{

            auto [failed, fd_path] = process_get_fd_path(pid, dir_fd);
            if(failed){
                return make_pair(ret_value_if_cant_resolve_path, txt_cant_get_fd + path);
            }

            // we know that `path` doesn't start with `/`, so the only thing to check is `fd_path`
            if(!fd_path.ends_with("/")){
                fd_path += "/";
            }
    
            // now combine the folder path and the filename
            path = fd_path + path;

        }

    }

    // we actually won't be parsing the flags since it's too much of a pain for the end user
    // to have to specify all the files AND the flags
    {
        // // parse parameter `flags`

        // // `flags` must include one of these: O_RDONLY (read only), O_WRONLY (write only), O_RDWR (read and write)
        // // other flags can be bitwise OR-ed
        // bool read_only = (flags | O_RDONLY);
        // bool write_only = (flags | O_WRONLY);
        // bool read_write = (flags | O_RDWR);
    }

    // return

    return make_pair(is_node_allowed(settings, path), txt_allowed_path + path);

}
