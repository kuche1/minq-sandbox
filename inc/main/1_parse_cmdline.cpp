
tuple<char*, char**, bool> parse_cmdline(int argc, char**argv){

    //defaults
    bool networkig_enable = false;

    // skip our name
    argc -= 1;
    argv += 1;

    {
        int orig_argc = argc;
        char** orig_argv = argv;

        for(int arg_idx=0; arg_idx<orig_argc; ++arg_idx){
            string arg = orig_argv[arg_idx];
            // cout << "DEBUG: " << arg << endl;

            if(arg == "--networking-enable"){
                networkig_enable = true;
                // do not pass this arg to the executable
                argc -= 1;
                argv += 1;
            }else{
                break;
            }
        }
    }

    if(argc <= 0){
        cout << "You need to specify the executable that you want to run, and the argument that are to be passed to it\n";
        exit(1);
    }

    char* executable = argv[0];
    char** executable_args = argv;

    return make_tuple(executable, executable_args, networkig_enable);
}
