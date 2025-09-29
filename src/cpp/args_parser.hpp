#ifndef __ARGS_PARSER_CXX_H
#define __ARGS_PARSER_CXX_H

#include <any>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <system_error>

namespace ap
{

using str2any = std::function<std::any(const std::string&)>;
using vec_any = std::vector<std::string>;

/**
 * @struct ArgOption
 * @brief  Defines the structure for a command-line option.
 *
 * This struct describes a specific command-line option, such as "-h" or "--help".
 */
struct ArgOption
{
private:
    ArgOption() {}
    static inline auto to_str = [](const std::string& s) { return s; };
    static inline auto to_int = [](const std::string& s) { return std::stoi(s); };
    static inline auto to_double = [](const std::string& s) { return std::stod(s); };
    friend class ArgOptionBuilder;
    friend class ArgsParser;
public:
    std::string name;                   ///< The short name of the option, e.g., "-h".
    std::string full_name;              ///< The full name of the option, e.g., "--help".
    std::string description;            ///< The description of the option for the help message.
    bool        required = false;       ///< A flag indicating whether the option is mandatory.
    int         least_params = 0;       ///< The minimum number of parameters this option requires.
    str2any     typer = to_str;         ///< String to type converter
    bool        has_default = false;    ///< Whether a default value is specified.
    std::any    default_value = {};     ///< The default value (can be any type).

    // --- Members below are populated after parsing ---
    int         idx;                    ///< The index where the option was found in the argv array. -1 if not found.
    int         params;                 ///< The actual number of parameters found for this option.

    // Basic constructor
    ArgOption(
        const std::string &n, 
        const std::string &fn, 
        const std::string &desc, 
        bool req, 
        int least, 
        str2any typer, 
        std::any default_value)
        : name(n), full_name(fn), description(desc), required(req), least_params(least), idx(-1),
          params(0), typer(typer), default_value(default_value), has_default(!default_value.has_value())
    {
    }

    ArgOption(
        const std::string &n, const std::string &fn, const std::string &desc, bool req, int least)
        : ArgOption(n, fn, desc, req, least, ArgOption::to_str, {})
    {
    }
};

struct ArgOptionBuilder {
private:
    ArgOption option;
public:
    ArgOptionBuilder() = default;

    ArgOptionBuilder& set_short(std::string s) { option.name = std::move(s); return *this; }
    ArgOptionBuilder& set_long(std::string l) { option.full_name = std::move(l); return *this; }
    ArgOptionBuilder& set_desc(std::string d) { option.description = std::move(d); return *this; }
    ArgOptionBuilder& set_required() { option.required = true; return *this; }
    ArgOptionBuilder& set_least(int lp) 
    { 
        if (lp < 0) throw std::invalid_argument("least_params cannot be negative.");
        option.least_params = lp; 
        return *this; 
    }
    ArgOptionBuilder& set_default(const std::any &a) 
    { 
        option.default_value = a; 
        option.has_default = true; 
        return *this;
    }

    ArgOptionBuilder& as_string() { option.typer = ArgOption::to_str; return *this; }
    ArgOptionBuilder& as_int() { option.typer = ArgOption::to_int; return *this; }
    ArgOptionBuilder& as_double() { option.typer = ArgOption::to_double; return *this; }
    ArgOptionBuilder& as_custom(str2any func) { option.typer = func; return *this; }

    ArgOption build() 
    {
        if (option.name.empty() && option.full_name.empty())
            throw std::invalid_argument("Both short name and long name cannot be empty.");
        if (option.name.empty()) option.name = option.full_name;
        if (option.full_name.empty()) option.full_name = option.name;
        if (option.required && option.least_params == 0)
            throw std::invalid_argument("Required option must have at least one parameter");
        return option;
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
                              int         least_params,
                              str2any     typer = ArgOption::to_str,
                              std::any    default_value = {});
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

    ArgOption *getOpt(std::string name);
    bool       has(std::string name);

    template <typename T>
    T get(std::string name, int pos = 0)
    {
        ArgOption *opt = this->getOpt(name);
        if (opt == nullptr)
            throw std::runtime_error("Non-exist arg option.");
        if (pos >= opt->params)
            throw std::out_of_range("Pos is out of the amount of params.");
        auto param = std::string(this->argv[opt->idx + pos + 1]);
        return std::any_cast<T>(opt->typer(param));
    }
};

}

#endif