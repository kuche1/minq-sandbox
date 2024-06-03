
{
    auto [faliure, code] = return_code;
    if(faliure){
        cerr << "Could not determine return code of main process\n";
        exit(1);
    }

    return code;
}
