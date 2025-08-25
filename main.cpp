#include <iostream>
#include <vector>
#include <string>

// 假设您的头文件名为 "args_parser.hpp"
#include "args_parser.hpp"

int main(int argc, char *argv[]) {
    // --- 1. 定义子命令的选项 ---
    // 我们将创建两个子命令: 'init' 和 'add'

    // 'init' 命令的选项列表
    std::vector<ArgOption> init_opts = {
        // {"短名称", "全称", "描述", 是否必需, 至少需要的参数个数}
        {"n", "name", "新项目的名称", true, 1},
        {"t", "template", "要使用的项目模板", false, 1},
    };

    // 'add' 命令的选项列表
    std::vector<ArgOption> add_opts = {
        {"f", "file", "要添加的文件路径", true, 1},
        {"T", "type", "文件的类型 (例如: source, doc, header)", true, 1},
        {"d", "dest", "文件在项目中的目标路径", false, 1},
    };

    // --- 2. 创建和配置解析器 ---

    // a. 创建主解析器
    // 主解析器本身也可以有自己的选项，这里我们为它添加一个 '--verbose' 选项
    ArgsParser parser("projmgr: 一个简单的项目管理工具", argc, argv);
    parser.addArgOpt("v", "verbose", "输出详细的日志信息", false, 0);

    // b. 创建子解析器并添加到主解析器中
    // 子解析器构造函数: {"命令名", "命令描述", 选项列表, argc, argv}
    parser.addSubParser(ArgsParser("init", "初始化一个新项目", init_opts, argc, argv));
    parser.addSubParser(ArgsParser("add", "向项目中添加一个新文件", add_opts, argc, argv));

    // --- 3. 执行解析 ---
    // parseArgs() 会自动识别子命令并进行相应的解析
    // 如果发生错误（例如：缺少必需参数），它会打印错误信息并返回非零错误码
    if (parser.parseArgs() != std::error_code())
    {
        // 解析出错时，错误信息已由解析器内部打印，这里直接退出即可
        return 1;
    }

    // --- 4. 处理解析结果 ---

    // a. 检查全局选项
    // 无论后续是否调用子命令，我们都可以先检查和处理全局选项。
    bool verbose_mode = parser.isArgOpt("verbose");
    if (verbose_mode) {
        std::cout << "[Verbose Mode Enabled]" << std::endl;
    }

    // b. 检查是否调用了子命令，并根据情况分发逻辑
    if (parser.getRelativeParser() != nullptr)
    {
        // --- 情况 1: 调用了子命令 ---
        // 获取被激活的子命令的名称
        std::string cmd = parser.getRelativeParser()->getCmdName();

        std::cout << "\n--- 执行子命令: " << cmd << " ---\n" << std::endl;

        if (cmd == "init")
        {
            // 通过主 parser 对象获取子命令的参数
            std::string project_name = parser.findParam("name", 0);
            std::cout << "正在初始化项目: '" << project_name << "'" << std::endl;

            if (parser.isArgOpt("template")) {
                std::string template_name = parser.findParam("template", 0);
                std::cout << "使用模板: '" << template_name << "'" << std::endl;
            } else {
                std::cout << "使用默认模板。" << std::endl;
            }
        }
        else if (cmd == "add")
        {
            std::string file_path = parser.findParam("file", 0);
            std::string file_type = parser.findParam("type", 0);
            
            std::cout << "准备添加文件: '" << file_path << "'" << std::endl;
            std::cout << "文件类型: '" << file_type << "'" << std::endl;
            
            if (parser.isArgOpt("dest")) {
                std::string dest_path = parser.findParam("dest", 0);
                std::cout << "目标路径: '" << dest_path << "'" << std::endl;
            } else {
                std::cout << "将添加到默认路径。" << std::endl;
            }
        }
    }
    else
    {
        // --- 情况 2: 没有调用子命令 ---
        // 在这里，我们可以处理只针对主命令的操作
        // 检查主命令是否有自己的参数被触发
        if (verbose_mode) {
            std::cout << "\n主命令任务执行：仅激活了详细模式。" << std::endl;
            std::cout << "这里可以执行一些独立的检查或显示状态信息的任务。" << std::endl;
        } else {
            // 如果既没有子命令，也没有触发任何主命令的有效选项，才显示帮助
            std::cout << "\n未指定任何命令或有效选项。请从下方选择一个命令执行：" << std::endl;
            parser.showHelpMsg();
        }
    }

    std::cout << "\n--- 程序执行完毕 ---\n" << std::endl;

    return 0;
}