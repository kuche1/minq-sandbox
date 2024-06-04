
//////////////
////////////////
/////////////////// paths
////////////////
/////////////

tuple<bool, string> resolve_path_at_cwd(string& path){

    char resolved_path[PATH_MAXLEN];
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

bool is_folder(string& path){
    const std::filesystem::path as_path_obj(path);

    if(std::filesystem::is_directory(as_path_obj)){
        return true;
    }

    return false;
}
