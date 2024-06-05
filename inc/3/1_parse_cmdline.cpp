
typedef struct{

    // executable

    char* executable = nullptr;
    char** executable_args = nullptr;

    // networking

    bool networking_enable = false;

    // filesystem

    bool filesystem_allow_all = false;
    bool filesystem_ask = false;
    vector<string> filesystem_allowed_nodes = {}; // if the names match we'll allow it AND if it's a file that is contains in a folder with such name

} Sandbox_settings;

Sandbox_settings parse_cmdline(int argc, char**argv){

    // flags // would be cool if all of these were constexpr
    string flag_networking_enable = "--networking-enable";
    string flag_filesystem_allow_all = "--filesystem-allow-all";
    string flag_help = "--help";
    string flag_filesystem_ask = "--filesystem-ask";
    string flag_common_allow = "--common-allow";
    vector<string> flags_match = {flag_networking_enable, flag_filesystem_allow_all, flag_help, flag_filesystem_ask, flag_common_allow};
    string flag_node_allow = "--node-allow:";
    string flag_node_allow_raw = "--node-allow-raw:";
    vector<string> flags_prefix = {flag_node_allow, flag_node_allow_raw};

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

                cout << "Flags that can be called by themselves (example: " << flag_networking_enable << "):\n";
                for(string& flag : flags_match){
                    cout << flag << endl;
                }
            
                cout << "Flags that require an argument (example: " << flag_node_allow << "/home/user123/data):\n";
                for(string& flag : flags_prefix){
                    cout << flag << endl;
                }
            
                exit(0);
            
            }else if(arg == flag_filesystem_ask){

                settings.filesystem_ask = true;

            }else if(arg == flag_common_allow){

                vector<string> common_nodes = {
                    // linker
                    "/etc/ld.so.cache",
                    // libraries
                    "/usr/lib",
                    // ...
                    "/dev/null",
                };

                for(const string& node : common_nodes){
                    string resolved = resolve_path_at_cwd(node);
                    settings.filesystem_allowed_nodes.push_back(resolved);
                }
            
            // flags that are used as prefixes

            }else if(arg.starts_with(flag_node_allow)){

                arg = arg.substr(flag_node_allow.size(), arg.size() - flag_node_allow.size());
                string resolved = resolve_path_at_cwd(arg);
                settings.filesystem_allowed_nodes.push_back(resolved);

            }else if(arg.starts_with(flag_node_allow_raw)){

                arg = arg.substr(flag_node_allow_raw.size(), arg.size() - flag_node_allow_raw.size());

                if(arg.ends_with("/")){
                    cerr << "Raw path must not end with `/`: " << arg << endl;
                    exit(1);
                }

                if( ("/"+arg+"/").contains("/../") ){
                    cerr << "Raw path must not contain `..`: " << arg << endl;
                    exit(1);
                }

                if( ("/"+arg+"/").contains("/./") ){
                    cerr << "Raw path must not contain `.`: " << arg << endl;
                    exit(1);
                }

                settings.filesystem_allowed_nodes.push_back(arg);

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
