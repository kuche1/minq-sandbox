
typedef struct{

    // executable

    char* executable = nullptr;
    char** executable_args = nullptr;

    // networking

    bool networking_enable = false;

    // filesystem

    bool fs_allow_all = false;
    bool fs_ask = false;
    vector<string> fs_allowed = {}; // if the names match we'll allow it AND if it's a file that is contains in a folder with such name // TODO after initially filling this, we should do 1 more round of also adding the cannonical paths
    bool fs_metadata_allow_all = false;

} Sandbox_settings;

Sandbox_settings parse_cmdline(int argc, char**argv){

    // flags // would be cool if all of these were constexpr
    string flag_networking_enable = "--networking-enable";
    string flag_fs_allow_all = "--fs-allow-all";
    string flag_help = "--help";
    string flag_fs_ask = "--fs-ask";
    string flag_fs_common_allow = "--fs-common-allow";
    string flag_fs_metadata_allow_all = "--fs-metadata-allow-all";
    vector<string> flags_match = {flag_networking_enable, flag_fs_allow_all, flag_help, flag_fs_ask, flag_fs_common_allow, flag_fs_metadata_allow_all};
    string flag_fs_allow = "--fs-allow:";
    string flag_fs_allow_raw = "--fs-allow-raw:";
    vector<string> flags_prefix = {flag_fs_allow, flag_fs_allow_raw};

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

            }else if(arg == flag_fs_allow_all){

                settings.fs_allow_all = true;

            }else if(arg == flag_help){ // not the best, but good enough

                cout << "Flags that can be called by themselves (example: " << flag_networking_enable << "):\n";
                for(string& flag : flags_match){
                    cout << flag << endl;
                }
            
                cout << "Flags that require an argument (example: " << flag_fs_allow << "/home/user123/data):\n";
                for(string& flag : flags_prefix){
                    cout << flag << endl;
                }
            
                exit(0);
            
            }else if(arg == flag_fs_ask){

                settings.fs_ask = true;

            }else if(arg == flag_fs_common_allow){

                vector<string> common_nodes = {
                    // linker
                    "/etc/ld.so.cache",
                    "/etc/ld.so.preload",
                    // binaries
                    "/usr/bin",
                    // libraries
                    "/usr/lib",
                    // null
                    "/dev/null",
                };

                for(const string& node : common_nodes){
                    string resolved = resolve_path_at_cwd(node);
                    settings.fs_allowed.push_back(resolved);
                }

            }else if(arg == flag_fs_metadata_allow_all){

                settings.fs_metadata_allow_all = true;
            
            // flags that are used as prefixes

            }else if(arg.starts_with(flag_fs_allow)){

                arg = arg.substr(flag_fs_allow.size(), arg.size() - flag_fs_allow.size());
                string resolved = resolve_path_at_cwd(arg);
                settings.fs_allowed.push_back(resolved);

            }else if(arg.starts_with(flag_fs_allow_raw)){

                arg = arg.substr(flag_fs_allow_raw.size(), arg.size() - flag_fs_allow_raw.size());

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

                settings.fs_allowed.push_back(arg);

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
