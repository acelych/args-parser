#include <algorithm>
#include <cctype>
#include <stdio.h>

#include "args_parser.hpp"

std::string toUpperString(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return src;
}

ArgsParser::ArgsParser(const std::string description, int argc, char **argv)
    : description(description), argc(argc), argv(argv)
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

ArgsParser::ArgsParser(const std::string            description,
                       const std::vector<ArgOption> args_list,
                       int                          argc,
                       char                       **argv)
    : ArgsParser(description, argc, argv)
{
    this->args_list.insert(this->args_list.end(), args_list.begin(), args_list.end());
}

ArgsParser::ArgsParser(const std::string            cmd_name,
                       const std::string            description,
                       const std::vector<ArgOption> args_list,
                       int                          argc,
                       char                       **argv)
    : ArgsParser(description, argc, argv)
{
    this->args_list.insert(this->args_list.end(), args_list.begin(), args_list.end());
    this->cmd_name     = cmd_name; // Set the task name for subparser
    this->is_subparser = true;     // this is a subparser
}

ArgsParser::~ArgsParser()
{
}

std::error_code ArgsParser::addSubParser(ArgsParser subparser)
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

std::error_code ArgsParser::addArgOpt(std::string name,
                                      std::string full_name,
                                      std::string description,
                                      bool        required,
                                      int         least_params)
{
    this->args_list.emplace_back(ArgOption(name, full_name, description, required, least_params));
    return std::error_code();
}

std::error_code ArgsParser::addArgOpt(ArgOption &arg)
{
    this->args_list.emplace_back(arg);
    return std::error_code();
}

std::error_code ArgsParser::parseArgs()
{
    ArgOption *curr_opt = nullptr;

    if ((!this->is_subparser && argc <= 1) || (this->is_subparser && argc <= 2))
    {
        this->showHelpMsg();
        return std::make_error_code(std::errc::bad_message);
    }

    if (!this->is_subparser) // Main parser
    {
        for (auto &subparser : this->subparsers)
        {
            if (subparser.cmd_name == argv[1])
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
            if (!found)
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

std::error_code ArgsParser::showHelpMsg()
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

bool ArgsParser::isArgOpt(std::string name)
{
    auto it = this->findArgOpt(name);
    if (it == nullptr)
        return false;     // Not found
    return it->idx != -1; // Found and has been set
}

ArgOption *ArgsParser::findArgOpt(std::string name)
{
    if (!this->is_subparser && this->relative != nullptr)
        return this->relative->findArgOpt(name); // Search in the relative parser

    for (auto &&arg : this->args_list)
    {
        if (arg.name == name || arg.full_name == name)
        {
            return &arg;
        }
    }
    return nullptr;
}

std::string ArgsParser::findParam(std::string name, int pos)
{
    ArgOption *opt = this->findArgOpt(name);
    try
    {
        pos++; // Adjust position to match the parameter index
        auto param = std::string(
            opt != nullptr && opt->idx + pos < this->argc ? this->argv[opt->idx + pos] : "");
        return param;
    }
    catch (const std::out_of_range &)
    {
        return ""; // Return empty string if out of range
    }
}
