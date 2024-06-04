
typedef struct{
    char* executable = nullptr;
    char** executable_args = nullptr;

    bool networking_enable = false;

    bool filesystem_allow_all = false;
    bool filesystem_ask = false;
    vector<string> filesystem_allowed_nodes = {}; // if the names match we'll allow it AND if it's a file that is contains in a folder with such name

    bool color = false;
} Sandbox_settings;

Sandbox_settings parse_cmdline(int argc, char**argv){

    // flags // would be cool if all of these were constexpr
    string flag_networking_enable = "--networking-enable";
    string flag_filesystem_allow_all = "--filesystem-allow-all";
    string flag_help = "--help";
    string flag_filesystem_ask = "--filesystem-ask";
    string flag_color = "--color";
    vector<string> flags_match = {flag_networking_enable, flag_filesystem_allow_all, flag_help, flag_filesystem_ask, flag_color};
    string flag_node_allow = "--node-allow:";
    vector<string> flags_prefix = {flag_node_allow};

    // defaults
    Sandbox_settings settings;

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
                settings.networking_enable = true;

            }else if(arg == flag_filesystem_allow_all){
                settings.filesystem_allow_all = true;

            }else if(arg == flag_help){ // not the best, but good enough

                cout << "Here is a list of the flags that can be called by themselves (example: " << flag_networking_enable << "):\n";
                for(string& flag : flags_match){
                    cout << flag << endl;
                }
            
                cout << "Here is a list of the flags that require an argument (example: " << flag_node_allow << "/home/user123/data):\n";
                for(string& flag : flags_prefix){
                    cout << flag << endl;
                }
            
                exit(0);
            
            }else if(arg == flag_filesystem_ask){

                settings.filesystem_ask = true;

            }else if(arg == flag_color){

                settings.color = true;
            
            // flags that are used as prefixes

            }else if(arg.starts_with(flag_node_allow)){

                arg = arg.substr(flag_node_allow.size(), arg.size() - flag_node_allow.size());

                auto [failure, resolved] = resolve_path_at_cwd(arg);
                if(failure){
                    cerr << "Could not resolve path `" << arg << "` (probably doesn't exist)\n";
                    exit(1);
                }

                settings.filesystem_allowed_nodes.push_back(resolved);

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

    settings.executable = argv[0];
    settings.executable_args = argv;

    return settings;
}
