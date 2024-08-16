#ifndef __ARGS_PARSER_CXX_H
#define __ARGS_PARSER_CXX_H

#include <string>
#include <vector>
#include <system_error>

typedef struct ArgOption
{
    std::string name;
    std::string full_name;
    std::string description;
    bool required;
    int least_params;

    int idx;
    int params;

    ArgOption(const std::string &n, const std::string &fn, const std::string &desc, bool req, int least) : 
    name(n), full_name(fn), description(desc), required(req), least_params(least), idx(-1), params(0) {}
} 
ArgOption;


class ArgsParser
{
private:
    int argc;
    const char **argv;

    std::string description;
    std::vector<ArgOption> args_list;
    
public:
    ArgsParser(const std::string description);
    ArgsParser(const std::string description, const std::vector<ArgOption> args_list, int argc, const char **argv);
    ~ArgsParser();
    
    std::error_code addArgOpt(std::string name, std::string full_name, std::string description, bool required, int least_params);
    std::error_code addArgOpt(ArgOption &arg);
    std::error_code parseArgs(int argc, const char **argv);
    std::error_code showHelpMsg();

    ArgOption* findArgOpt(std::string name);
};

#endif