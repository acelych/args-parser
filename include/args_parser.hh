#ifndef __ARGS_PARSER_HH__
#define __ARGS_PARSER_HH__

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
        if (option.typer == ArgOption::to_str && a.type() == typeid(const char*))
            option.default_value = std::string(std::any_cast<const char*>(a));
        else
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

    bool allow_unexpected_arg = false;
public:
    ArgsParser(const std::string description, int argc, char **argv, bool allow_unexpected_arg = false);
    ArgsParser(const std::string            description,
               const std::vector<ArgOption> args_list,
               int                          argc,
               char                       **argv,
               bool                         allow_unexpected_arg = false);
    ArgsParser(const std::string            cmd_name,
               const std::string            description,
               const std::vector<ArgOption> args_list,
               int                          argc,
               char                       **argv,
               bool                         allow_unexpected_arg = false);
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
    ArgOption *getOpt(std::string name)
    {
        if (!this->is_subparser && this->relative != nullptr)
            return this->relative->getOpt(name); // Search in the relative parser

        for (auto &&arg : this->args_list)
        {
            if (arg.name == name || arg.full_name == name)
            {
                return &arg;
            }
        }
        return nullptr;
    }

    bool has(std::string name)
    {
        auto it = this->getOpt(name);
        if (it == nullptr)
            return false;     // Not found
        return it->idx != -1; // Found and has been set
    }

    template <typename T>
    T get(std::string name, int pos = 0)
    {
        ArgOption *opt = this->getOpt(name);
        if (opt == nullptr)
            throw std::runtime_error("Non-exist arg option.");
        if (pos >= opt->params)
        {
            if (!opt->has_default)
                throw std::out_of_range("Pos is out of the amount of params.");
            return std::any_cast<T>(opt->default_value);
        }
        auto param = std::string(this->argv[opt->idx + pos + 1]);
        return std::any_cast<T>(opt->typer(param));
    }
};

/* ---------============================--------- */

inline static std::string toUpperString(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return src;
}

inline ArgsParser::ArgsParser(const std::string description, int argc, char **argv, bool allow_unexpected_arg)
    : description(description), argc(argc), argv(argv), allow_unexpected_arg(allow_unexpected_arg)
{
    this->addArgOpt("h", "help", "show this help message and exit", false, 0);

    // Main task name is the executable name
    const char *exe_start_mark = this->argv[0] + std::string(this->argv[0]).size() - 1;
    auto        isSlash        = [](char c) { return c == '/' || c == '\\'; };
    while (exe_start_mark > this->argv[0] && !isSlash(*exe_start_mark))
        exe_start_mark--; // Skip trailing slashes or backslashes
    this->cmd_name = std::string(
        exe_start_mark +
        (isSlash(*exe_start_mark) ? 1 : 0)); // Get the task name from the executable path
}

inline ArgsParser::ArgsParser(const std::string            description,
                              const std::vector<ArgOption> args_list,
                              int                          argc,
                              char                       **argv,
                              bool                         allow_unexpected_arg)
    : ArgsParser(description, argc, argv, allow_unexpected_arg)
{
    this->args_list.insert(this->args_list.end(), args_list.begin(), args_list.end());
}

inline ArgsParser::ArgsParser(const std::string            cmd_name,
                              const std::string            description,
                              const std::vector<ArgOption> args_list,
                              int                          argc,
                              char                       **argv,
                              bool                         allow_unexpected_arg)
    : ArgsParser(description, argc, argv, allow_unexpected_arg)
{
    this->args_list.insert(this->args_list.end(), args_list.begin(), args_list.end());
    this->cmd_name     = cmd_name; // Set the task name for subparser
    this->is_subparser = true;     // this is a subparser
}

inline ArgsParser::~ArgsParser()
{
}

inline std::error_code ArgsParser::addSubParser(ArgsParser subparser)
{
    if (this->is_subparser)
    {
        printf("Cannot add subparser to a subparser!\n");
        return std::make_error_code(std::errc::invalid_argument);
    }
    subparser.is_subparser = true;
    subparser.relative     = this;
    this->subparsers.emplace_back(subparser);
    return std::error_code();
}

inline std::error_code ArgsParser::addArgOpt(std::string name,
                                      std::string full_name,
                                      std::string description,
                                      bool        required,
                                      int         least_params,
                                      str2any     typer,
                                      std::any    default_value)
{
    this->args_list.emplace_back(ArgOption(name, full_name, description, required, least_params, typer, default_value));
    return std::error_code();
}

inline std::error_code ArgsParser::addArgOpt(ArgOption &arg)
{
    this->args_list.emplace_back(arg);
    return std::error_code();
}

inline std::error_code ArgsParser::parseArgs()
{
    ArgOption *curr_opt = nullptr;

    if (!this->is_subparser) // Main parser
    {
        for (auto &subparser : this->subparsers)
        {
            if (argc > 1 && subparser.cmd_name == argv[1])
            {
                this->relative = &subparser; // Set the relative parser
                return subparser.parseArgs();
            }
        }
    }

    for (int i = this->is_subparser ? 2 : 1; i < argc; i++)
    {
        if (argv[i][0] == '-') // is opt
        {
            bool found = false;
            for (int j = 0; j < this->args_list.size(); j++)
            {
                curr_opt = &this->args_list.at(j);
                if (std::string(argv[i] + 1) == curr_opt->name ||
                    std::string(argv[i] + 2) == curr_opt->full_name)
                {
                    curr_opt->idx = i;
                    found         = true;
                    break;
                }
            }
            if (!found && !allow_unexpected_arg)
            {
                printf("Unexpected argument: \'%s\'\n", argv[i]);
                return std::make_error_code(std::errc::invalid_argument);
            }
        }
        else // is param
        {
            if (curr_opt != nullptr)
                (curr_opt->params)++;
            else
            {
                printf("Unexpected parameter: \'%s\'\n", argv[i]);
                return std::make_error_code(std::errc::invalid_argument);
            }
        }
    }

    for (int i = 0; i < this->args_list.size(); i++)
    {
        curr_opt = &this->args_list[i];
        if ((curr_opt->name == "h") && curr_opt->idx != -1) // got help arg: show help info & quit
        {
            this->showHelpMsg();
            return std::make_error_code(std::errc::bad_message);
        }
        if (curr_opt->required && curr_opt->idx == -1) // missing required argument
        {
            printf("Required argument \'%s\' not found!\n", curr_opt->full_name.c_str());
            return std::make_error_code(std::errc::bad_message);
        }
        if (curr_opt->idx != -1 && curr_opt->least_params > curr_opt->params) // missing parameters
        {
            printf("Need %d params after \'%s\'/\'%s\', got %d params!\n", curr_opt->least_params,
                   curr_opt->name.c_str(), curr_opt->full_name.c_str(), curr_opt->params);
            return std::make_error_code(std::errc::bad_message);
        }
    }

    return std::error_code();
}

inline std::error_code ArgsParser::showHelpMsg()
{
    ArgOption                                       *curr_opt;
    std::vector<std::pair<std::string, std::string>> examples;
    size_t                                           max_char_num = 0;

    if (!this->is_subparser)
    {
        printf("Usage: %s", this->cmd_name.c_str());
        if (this->subparsers.size() > 0)
            printf(" [command]");
    }
    else
        printf("Usage: %s %s", this->relative->cmd_name.c_str(), this->cmd_name.c_str());

    for (int i = 0; i < this->args_list.size(); i++)
    {
        curr_opt = &this->args_list.at(i);

        std::string curr_opt_form = "";
        curr_opt_form += " ";
        curr_opt_form += curr_opt->required ? "-" : "[-";
        curr_opt_form += curr_opt->name;
        for (int j = 0; j < curr_opt->least_params; j++)
            curr_opt_form += " " + toUpperString(curr_opt->full_name);
        curr_opt_form += curr_opt->required ? "" : "]";
        printf("%s", curr_opt_form.c_str());

        std::string curr_opt_example = "";
        curr_opt_example += "-" + curr_opt->name;
        for (int j = 0; j < curr_opt->least_params; j++)
            curr_opt_example += " " + toUpperString(curr_opt->full_name);
        examples.emplace_back(
            std::pair<std::string, std::string>(curr_opt_example, curr_opt->description));
        max_char_num = std::max(max_char_num, curr_opt_example.size());
    }
    printf("\n\n%s", this->description.c_str());
    printf("\n\noptions:\n");
    for (auto &&pair : examples)
        printf("  %-*s%s\n", static_cast<int>(max_char_num + 2), pair.first.c_str(),
               pair.second.c_str());

    if (this->subparsers.size() > 0)
    {
        examples.clear();
        max_char_num = 0;
        for (auto &&subparser : this->subparsers)
        {
            examples.emplace_back(
                std::pair<std::string, std::string>(subparser.cmd_name, subparser.description));
            max_char_num = std::max(max_char_num, subparser.cmd_name.size());
        }
        printf("\ncommands:\n");
        for (auto &&pair : examples)
            printf("  %-*s%s\n", static_cast<int>(max_char_num + 2), pair.first.c_str(),
                   pair.second.c_str());
    }

    return std::error_code();
}

}

#endif