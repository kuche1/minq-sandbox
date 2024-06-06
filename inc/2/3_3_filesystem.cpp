
//////////////
////////////////
/////////////////// paths
////////////////
/////////////

std::filesystem::path get_cwd_as_path(){
    try{
        return std::filesystem::current_path();
    }catch(const std::filesystem::filesystem_error& e){
        cerr << "Could not get CWD: " << e.what() << endl;
        exit(1);
    }
}

string get_cwd(){
    std::filesystem::path cwd_path_as_path = get_cwd_as_path();
    return cwd_path_as_path.string();
}

// TODO this can be fucked if with if the attacker has a symlink created b4hand
//
// just keep in mind the fact that this fnc is use everywhere, example: what if someone runs a syscall that creates a new file? if we are to simply use canonical paths if would fail; what if someone is to delete a symlink? if we are to use regular+canonical_when_possible paths this would fail; what is we don't use canonical paths at all? you are able to escape the sandbox if a sandbox already exists that symlinks to another location
// perhaps the solution is to have 2 versions - one for canonical paths, and one for non-canonical

string resolve_path_at_cwd(const string& path){

    std::filesystem::path path_as_path(path);

    if(!path.starts_with("/")){
        
        std::filesystem::path cwd_as_path = get_cwd_as_path();

        path_as_path = std::filesystem::path(path);

        path_as_path = cwd_as_path / path_as_path;

    }

    std::filesystem::path normalized_path_as_path = path_as_path.lexically_normal();

    return normalized_path_as_path.string();
}

string resolve_path(pid_t process_pid, int relative_to_fd, const string& path){

    std::filesystem::path path_as_path(path);

    if(!path.starts_with("/")){

        string relative_to = [process_pid, relative_to_fd]{
            if(relative_to_fd == AT_FDCWD){
                return process_get_cwd(process_pid);
            }else{
                return process_get_fd_path(process_pid, relative_to_fd);
            }
        }();

        path_as_path = relative_to / path_as_path;

    }

    std::filesystem::path normalized_path_as_path = path_as_path.lexically_normal();

    return normalized_path_as_path.string();
}

// tuple<bool, string> resolve_path_at_cwd(const string& path){

//     boost::filesystem::path unresolved_path(path);
//     boost::filesystem::path canonical_path;

//     try{
//         canonical_path = boost::filesystem::canonical(unresolved_path);
//     }catch(const boost::filesystem::filesystem_error& ex){
//         return make_pair(true, ex.what());
//     }

//     string canonical_path_as_str = canonical_path.string();

//     return make_pair(false, canonical_path_as_str);
// }


// // `relative_to`'s default value is ""
// pair<bool, string> resolve_path(pid_t process_pid, const string& path, string relative_to){

//     if(relative_to == ""){
//         ostringstream oss_process_pwd_file;
//         oss_process_pwd_file << "/proc/" << process_pid << "/cwd";
//         string process_pwd_file = oss_process_pwd_file.str();
//         auto [process_cwd_failure, process_cwd] = resolve_path_at_cwd(process_pwd_file);
//         if(process_cwd_failure){
//             return make_pair(true, "could not resolve process cwd file: " + process_pwd_file);
//         }
//         relative_to = process_cwd;
//     }

//     boost::filesystem::path pcwdp(relative_to);
//     boost::filesystem::path unresolved_path(path);
//     boost::filesystem::path canonical_path;

//     try{
//         canonical_path = boost::filesystem::canonical(unresolved_path, pcwdp);
//     }catch(const boost::filesystem::filesystem_error& ex){
//         return make_pair(true, ex.what());
//     }

//     string canonical_path_as_str = canonical_path.string();

//     return make_pair(false, canonical_path_as_str);
// }

// bool is_folder(string& path){
//     const std::filesystem::path as_path_obj(path);

//     if(std::filesystem::is_directory(as_path_obj)){
//         return true;
//     }

//     return false;
// }
