#ifndef __ARGS_PARSER_CXX_H
#define __ARGS_PARSER_CXX_H

#include <string>
#include <system_error>
#include <vector>

/**
 * @struct ArgOption
 * @brief  Defines the structure for a command-line option.
 *
 * This struct describes a specific command-line option, such as "-h" or "--help".
 */
struct ArgOption
{
    std::string name;         ///< The short name of the option, e.g., "-h".
    std::string full_name;    ///< The full name of the option, e.g., "--help".
    std::string description;  ///< The description of the option for the help message.
    bool        required;     ///< A flag indicating whether the option is mandatory.
    int         least_params; ///< The minimum number of parameters this option requires.

    // --- Members below are populated after parsing ---
    int idx;    ///< The index where the option was found in the argv array. -1 if not found.
    int params; ///< The actual number of parameters found for this option.

    ArgOption(
        const std::string &n, const std::string &fn, const std::string &desc, bool req, int least)
        : name(n), full_name(fn), description(desc), required(req), least_params(least), idx(-1),
          params(0)
    {
    }
};

/**
 * @class ArgsParser
 * @brief A command-line argument parser class.
 *
 * This class is designed to parse argc and argv from the main function,
 * supporting sub-commands and automatic help message generation.
 */
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