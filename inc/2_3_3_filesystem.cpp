
//////////////
////////////////
/////////////////// paths
////////////////
/////////////

tuple<bool, string> resolve_path_at_cwd(const string& path){

    // TODO we could replace this with boost's version
    // as to avoid the memory error

    char resolved_path[PATH_MAXLEN] = {0};
    errno = 0;

    if(!realpath(path.c_str(), resolved_path)){

        if(errno == ENOMEM){
            cout << "Could not resolve path because of lack of buffer memory, please contact the developer\n";
            exit(1);
        }

        return make_tuple(true, string(""));

    }

    return make_tuple(false, string(resolved_path));
}

// relative_to's default value is ""
pair<bool, string> resolve_path(pid_t process_pid, const string& path, string relative_to){

    if(relative_to == ""){
        ostringstream oss_process_pwd_file;
        oss_process_pwd_file << "/proc/" << process_pid << "/cwd";
        string process_pwd_file = oss_process_pwd_file.str();
        auto [process_cwd_failure, process_cwd] = resolve_path_at_cwd(process_pwd_file);
        if(process_cwd_failure){
            return make_pair(true, "could not resolve process cwd file: " + process_pwd_file);
        }
        relative_to = process_cwd;
    }

    boost::filesystem::path pcwdp(relative_to);
    boost::filesystem::path unresolved_path(path);
    boost::filesystem::path canonical_path;

    try{
        canonical_path = boost::filesystem::canonical(unresolved_path, pcwdp);
    }catch(const boost::filesystem::filesystem_error& ex){
        return make_pair(true, ex.what());
    }

    string canonical_path_as_str = canonical_path.string();

    return make_pair(false, canonical_path_as_str);
}

bool is_folder(string& path){
    const std::filesystem::path as_path_obj(path);

    if(std::filesystem::is_directory(as_path_obj)){
        return true;
    }

    return false;
}
