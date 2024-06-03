
tuple<char*, char**, bool, bool> parse_cmdline(int argc, char**argv){

    // flags // would be cool if all of these were constexpr
    string flag_networking_enable = "--networking-enable";
    string flag_filesystem_allow_all = "--filesystem-allow-all";
    string flag_help = "--help";
    vector<string> all_flags = {flag_networking_enable, flag_filesystem_allow_all, flag_help};

    // defaults
    bool networkig_enable = false;
    bool filesystem_allow_all = false;

    // skip our name
    argc -= 1;
    argv += 1;

    {
        int orig_argc = argc;
        char** orig_argv = argv;

        for(int arg_idx=0; arg_idx<orig_argc; ++arg_idx){
            string arg = orig_argv[arg_idx];

            if(arg == flag_networking_enable){
                networkig_enable = true;

            }else if(arg == flag_filesystem_allow_all){
                filesystem_allow_all = true;

            }else if(arg == flag_help){ // not the best, but good enough
                cout << "Here is a list of all the flags:\n";
                for(string& flag : all_flags){
                    cout << flag << endl;
                }
                exit(0);

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

    return make_tuple(executable, executable_args, networkig_enable, filesystem_allow_all);
}
