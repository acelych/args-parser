#ifndef __ARGS_PARSER_CXX_H
#define __ARGS_PARSER_CXX_H

#include <string>
#include <system_error>
#include <vector>

struct ArgOption
{
    std::string name;
    std::string full_name;
    std::string description;
    bool        required;
    int         least_params;

    int idx;
    int params;

    ArgOption(
        const std::string &n, const std::string &fn, const std::string &desc, bool req, int least)
        : name(n), full_name(fn), description(desc), required(req), least_params(least), idx(-1),
          params(0)
    {
    }
};

class ArgsParser
{
private:
    int    argc;
    char **argv;

    std::string cmd_name;
    bool        is_subparser = false;
    ArgsParser *relative     = nullptr;

    std::string             description;
    std::vector<ArgOption>  args_list;
    std::vector<ArgsParser> subparsers;

public:
    ArgsParser(const std::string description, int argc, char **argv);
    ArgsParser(const std::string            description,
               const std::vector<ArgOption> args_list,
               int                          argc,
               char                       **argv);
    ArgsParser(const std::string            cmd_name,
               const std::string            description,
               const std::vector<ArgOption> args_list,
               int                          argc,
               char                       **argv);
    ~ArgsParser();

    std::error_code addSubParser(ArgsParser subparser);
    std::error_code addArgOpt(std::string name,
                              std::string full_name,
                              std::string description,
                              bool        required,
                              int         least_params);
    std::error_code addArgOpt(ArgOption &arg);
    std::error_code parseArgs();
    std::error_code showHelpMsg();

    std::string &getCmdName()
    {
        return this->cmd_name;
    }
    ArgsParser *getRelativeParser()
    {
        return this->relative;
    }

    bool        isArgOpt(std::string name);
    ArgOption  *findArgOpt(std::string name);
    std::string findParam(std::string name, int pos);
};

#endif