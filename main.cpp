#include "args_parser.hpp"

int main(int argc, char *argv[]) {
    std::vector<ArgOption> direct_opts = {
        {"f", "file", "输入二进制文件路径", true, 1},
        {"r", "rows", "矩阵行数", true, 1},
        {"c", "cols", "矩阵列数", true, 1},
        {"hi", "hist", "是否使用直方图均衡量化", false, 0},
    };
    std::vector<ArgOption> watch_opts = {
        {"f", "file", "输入二进制文件路径", true, 1},
        {"r", "rows", "矩阵行数", true, 1},
        {"c", "cols", "矩阵列数", true, 1},
        {"hi", "hist", "是否使用直方图均衡量化", false, 0},
    };

    ArgsParser parser("从二进制文件中读取并可视化复数数据.", argc, argv);
    parser.addSubParser(ArgsParser("direct", "直接绘制一次", direct_opts, argc, argv));
    parser.addSubParser(ArgsParser("watch", "持续监控文件变化并自动重绘", watch_opts, argc, argv));
    if (parser.parseArgs() != std::error_code())
    {
        return 1;
    }
    if (parser.getRelativeParser() == nullptr)
    {
        parser.showHelpMsg();
        return 0;
    }

    // Fulfill the required parameters
    auto   cmd       = parser.getRelativeParser()->getCmdName();
    auto   file_path = parser.findParam("file", 0);
    size_t rows      = std::stoull(parser.findParam("rows", 0));
    size_t cols      = std::stoull(parser.findParam("cols", 0));
    bool   hist      = parser.isArgOpt("hist");
    
    if (cmd == "direct")
    {
        //TODO
    }
    else if (cmd == "watch")
    {
        //TODO
    }
    return 0;
}