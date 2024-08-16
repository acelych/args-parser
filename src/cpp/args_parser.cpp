#include <cctype>
#include <algorithm>
#include <stdio.h>

#include "args_parser.hpp"

std::string toUpperString(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(), [](unsigned char c){
        return std::toupper(c);
    });
    return src;
}

ArgsParser::ArgsParser(const std::string description)
{
    this->description = description;
    this->addArgOpt("h", "help", "show this help message and exit", false, 0);
}

ArgsParser::ArgsParser(const std::string description, const std::vector<ArgOption> args_list, int argc, const char **argv)
{
    this->description = description;
    this->addArgOpt("h", "help", "show this help message and exit", false, 0);
    this->args_list.insert(this->args_list.end(), args_list.begin(), args_list.end());

    if (this->parseArgs(argc, argv)) std::exit(-1);
}

ArgsParser::~ArgsParser()
{
}

std::error_code ArgsParser::addArgOpt(std::string name, std::string full_name, std::string description, bool required, int least_params)
{
    this->args_list.emplace_back(ArgOption(name, full_name, description, required, least_params));
    return std::error_code();
}

std::error_code ArgsParser::addArgOpt(ArgOption &arg)
{
    this->args_list.emplace_back(arg);
    return std::error_code();
}

std::error_code ArgsParser::parseArgs(int argc, const char **argv)
{
    ArgOption *curr_opt;
    this->argc = argc;
    this->argv = argv;

    if (argc == 1)
    {
        this->showHelpMsg();
        return std::make_error_code(std::errc::bad_message);
    }
    
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')      // is opt
        {
            bool found = false;
            for (int j = 0; j < this->args_list.size(); j++)
            {
                curr_opt = &this->args_list.at(j);
                if (
                    std::string(argv[i]).find(curr_opt->name) != std::string::npos || 
                    std::string(argv[i]).find(curr_opt->full_name) != std::string::npos)
                {
                    curr_opt->idx = i;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                printf("Unexpected argument: \'%s\'\n", argv[i]);
                return std::make_error_code(std::errc::invalid_argument);
            }
        }
        else                        // is param
        {
            if (curr_opt != NULL) (curr_opt->params)++;
            else if (i == 0);
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
        if ((curr_opt->name == "-h") && curr_opt->idx != -1)          // got help arg: show help info & quit
        {
            this->showHelpMsg();
            return std::make_error_code(std::errc::bad_message);
        }
        if (curr_opt->required && curr_opt->idx == -1)                              // missing required argument
        {
            printf("Required argument \'%s\' not found!\n", curr_opt->full_name.c_str());
            return std::make_error_code(std::errc::bad_message);
        }
        if (curr_opt->idx != -1 && curr_opt->least_params > curr_opt->params)       // missing parameters
        {
            printf("Need %d params after \'%s\'/\'%s\', got %d params!\n", 
            curr_opt->least_params, curr_opt->name.c_str(), curr_opt->full_name.c_str(), curr_opt->params);
            return std::make_error_code(std::errc::bad_message);
        }
    }

    return std::error_code();
}

std::error_code ArgsParser::showHelpMsg()
{
    ArgOption *curr_opt;
    std::vector<std::pair<std::string, std::string>> examples;
    size_t max_char_num = 0;

    const char *exe_start_mark = this->argv[0] + std::string(this->argv[0]).size() - 1;
    for (; *exe_start_mark != '/' && *exe_start_mark != '\\'; exe_start_mark--);
    std::string exe_name = exe_start_mark + 1;

    printf("usage: %s", exe_name.c_str());
    for (int i = 0; i < this->args_list.size(); i++)
    {
        curr_opt = &this->args_list.at(i);
        
        std::string curr_opt_form = "";
        curr_opt_form += " ";
        curr_opt_form += curr_opt->required ? "-" : "[-";
        curr_opt_form += curr_opt->name;
        for (int j = 0; j < curr_opt->least_params; j++) curr_opt_form += " " + toUpperString(curr_opt->full_name);
        curr_opt_form += curr_opt->required ? "" : "]";
        printf("%s", curr_opt_form.c_str());

        std::string curr_opt_example = "";
        curr_opt_example += "-" + curr_opt->name;
        for (int j = 0; j < curr_opt->least_params; j++) curr_opt_example += " " + toUpperString(curr_opt->full_name);
        examples.emplace_back(std::pair<std::string, std::string>(curr_opt_example, curr_opt->description));
        max_char_num = std::max(max_char_num, curr_opt_example.size());
    }
    printf("\n\n%s\n\noptions:\n", this->description.c_str());
    for (auto &&pair : examples)
    {
        printf("  %-*s%s\n", max_char_num + 2, pair.first.c_str(), pair.second.c_str());
    }

    return std::error_code();
}

ArgOption* ArgsParser::findArgOpt(std::string name)
{
    for (auto &&arg : this->args_list)
    {
        if (arg.name == name || arg.full_name == name)
        {
            return &arg;
        }
    }
    return nullptr;
}
