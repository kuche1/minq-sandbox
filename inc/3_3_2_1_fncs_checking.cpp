
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
