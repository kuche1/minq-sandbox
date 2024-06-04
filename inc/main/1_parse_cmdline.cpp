
tuple<char*, char**, bool, bool, vector<string>> parse_cmdline(int argc, char**argv){

    // flags // would be cool if all of these were constexpr
    string flag_networking_enable = "--networking-enable";
    string flag_filesystem_allow_all = "--filesystem-allow-all";
    string flag_help = "--help";
    string flag_folder_allow = "--folder_allow:";
    vector<string> flags_match = {flag_networking_enable, flag_filesystem_allow_all, flag_help};
    vector<string> flags_prefix = {flag_folder_allow};

    // defaults
    bool networkig_enable = false;
    bool filesystem_allow_all = false;
    vector<string> filesystem_allowed_folders;

    // skip our name
    argc -= 1;
    argv += 1;

    {
        int orig_argc = argc;
        char** orig_argv = argv;

        for(int arg_idx=0; arg_idx<orig_argc; ++arg_idx){
            string arg = orig_argv[arg_idx];

            // flags that exactly match

            if(arg == flag_networking_enable){
                networkig_enable = true;

            }else if(arg == flag_filesystem_allow_all){
                filesystem_allow_all = true;

            }else if(arg == flag_help){ // not the best, but good enough

                cout << "Here is a list of the flags that can be called by themselves (example: " << flag_networking_enable << "):\n";
                for(string& flag : flags_match){
                    cout << flag << endl;
                }
            
                cout << "Here is a list of the flags that require an argument (example: " << flag_folder_allow << "/home/user123/data):\n";
                for(string& flag : flags_prefix){
                    cout << flag << endl;
                }
            
                exit(0);
            
            // flags that are used as prefixes

            }else if(arg.starts_with(flag_folder_allow)){

                arg = arg.substr(flag_folder_allow.size(), arg.size() - flag_folder_allow.size());

                auto [failure, resolved] = resolve_path_at_cwd(arg);
                if(failure){
                    cerr << "Could not resolve path `" << arg << "` (probably doesn't exist)\n";
                    exit(1);
                }

                if(!is_folder(resolved)){
                    cerr << "Path is not a folder `" << resolved << "`\n";
                    exit(1);
                }

                filesystem_allowed_folders.push_back(resolved);

            // ...

            }else{
                break;
            }

            // do not pass the arg to the executable
            argc -= 1;
            argv += 1;
        }
    }

    if(argc <= 0){
        cout << "You need to specify the executable that you want to run, and the argument that are to be passed to it\n";
        exit(1);
    }

    char* executable = argv[0];
    char** executable_args = argv;

    return make_tuple(executable, executable_args, networkig_enable, filesystem_allow_all, filesystem_allowed_folders);
}
