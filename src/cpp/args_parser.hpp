#ifndef __ARGS_PARSER_CXX_H
#define __ARGS_PARSER_CXX_H

#include <string>

typedef struct ArgOption
{
    std::string name;
    std::string full_name;
    std::string description;
    bool required;
    int least_params;

    int idx;
    int params;
} 
ArgOption;


class ArgsParser
{
private:
    
public:
    ArgsParser();
    ~ArgsParser();
};

ArgsParser::ArgsParser()
{
}

ArgsParser::~ArgsParser()
{
}

#endif