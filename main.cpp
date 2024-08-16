#include "args_parser.hpp"

std::vector<ArgOption> args = 
{
    ArgOption("p", "path", "path to object", true, 1),
    ArgOption("r", "recursion", "recursively search", false, 0),
    ArgOption("a", "all", "select all objects", false, 0),
};

int main(int argc, char const *argv[]) {
    auto parser = new ArgsParser("Example app", args, argc, argv);
    return 0;
}