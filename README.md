# ARGS-PARSER FOR C/C++

ArgsParser is a lightweight, header-only C++17 library for parsing command-line arguments with support for:

Short (-h) and long (--help) options
Typed argument conversion (string, int, double, or custom)
Required/optional flags with parameter counts
Default values
Subcommands (e.g., git commit, docker run)
Automatic help message generation
Designed for ease of use and minimal dependencies—only standard library components.

## 📦 Installation
Since ArgsParser is header-only, simply copy args_parser.hh into your project and include it:

```cpp
#include "args_parser.hh"
```
No linking or build system changes required.

✅ Requirements: C++17 or later (for std::any, std::string_view-like usage, and structured bindings support in user code). 

## 🚀 Quick Start
### Basic Usage

```cpp
#include "args_parser.hh"
#include <iostream>

int main(int argc, char** argv) {
    using namespace ap;

    // Define options using builder pattern
    auto verbose_opt = ArgOptionBuilder()
        .set_short("v")
        .set_long("verbose")
        .set_desc("Enable verbose output")
        .as_int()
        .set_default(0)
        .build();

    auto input_opt = ArgOptionBuilder()
        .set_short("i")
        .set_long("input")
        .set_desc("Input file path")
        .set_required()
        .set_least(1)
        .as_string()
        .build();

    // Create parser
    ArgsParser parser("A simple file processor", {verbose_opt, input_opt}, argc, argv);

    // Parse arguments
    if (auto ec = parser.parseArgs()) {
        return ec.value();
    }

    // Access values
    int verbosity = parser.get<int>("verbose");
    std::string input_file = parser.get<std::string>("input");

    std::cout << "Verbosity level: " << verbosity << "\n";
    std::cout << "Input file: " << input_file << "\n";

    return 0;
}
```
Run with:
```sh
./myapp -i data.txt -v 2
```

## 🔧 Advanced Features
### Subcommands
Support hierarchical commands like app build --release or app test --coverage.
```cpp
ArgsParser main_parser("Main application", argc, argv);

// Subparser for 'build'
ArgsParser build_parser("build", "Compile the project", {}, argc, argv);
build_parser.addArgOpt("r", "release", "Build in release mode", false, 0);

// Subparser for 'test'
ArgsParser test_parser("test", "Run unit tests", {}, argc, argv);
test_parser.addArgOpt("c", "coverage", "Generate coverage report", false, 0);

main_parser.addSubParser(build_parser);
main_parser.addSubParser(test_parser);

if (auto ec = main_parser.parseArgs()) {
    return ec.value();
}

// Check which subcommand was used
if (main_parser.getRelativeParser()) {
    std::cout << "Running subcommand: " << main_parser.getRelativeParser()->getCmdName() << "\n";
}
```
Usage:
```sh
./app build --release
./app test -c
./app --help        # Shows main help
./app build --help  # Shows build-specific help
```
### Custom Type Conversion
Need to parse enums, paths, or custom types?
```cpp
auto to_log_level = [](const std::string& s) -> std::any {
    if (s == "debug") return LogLevel::Debug;
    if (s == "info")  return LogLevel::Info;
    if (s == "error") return LogLevel::Error;
    throw std::invalid_argument("Invalid log level");
};

auto log_opt = ArgOptionBuilder()
    .set_long("log-level")
    .set_desc("Set logging level: debug|info|error")
    .set_least(1)
    .as_custom(to_log_level)
    .build();
```
Then retrieve with:
```cpp
LogLevel level = parser.get<LogLevel>("log-level");
```
## 📝 API Overview

`ArgOptionBuilder`

Fluent interface to construct options:

* `.set_short("s")` / `.set_long("long")`
* `.set_desc("...")`
* `.set_required()`
* `.set_least(N)` – minimum number of parameters
* `.set_default(value)`
* `.as_string()`, `.as_int()`, `.as_double()`, or `.as_custom(func)`

`ArgsParser`

Construct with description, optional initial options, and argc/argv

* `.addArgOpt(...)` – add options post-construction
* `.addSubParser(subparser)` – register subcommands
* `.parseArgs()` – parse and validate; returns std::error_code on failure
* `.has("name")` – check if option was provided
* `.get<T>("name", pos=0)` – retrieve typed value (pos for multi-param options)
* `.showHelpMsg()` – print formatted help (called automatically on -h/--help)

> Note: The `-h` / `--help` option is automatically added unless you override it. 

## 🛑 Error Handling
* Invalid arguments, missing required options, or type conversion failures return non-zero `std::error_code`.
* Help request (`-h` / `--help`) returns `std::errc::bad_message` (treated as intentional exit).
* All errors print descriptive messages to `stdout`.

## 📜 License
MIT License – free for personal and commercial use.

## 🙌 Contributing
Bug reports and feature requests welcome! Since this is a single-header library, contributions should maintain simplicity and zero-dependency philosophy.

> Tip: For complex CLI needs (e.g., positional arguments, groups, or advanced validation), consider mature libraries like CLI11 or Boost.Program_options .
> ArgsParser shines in small-to-medium projects where simplicity and self-containment matter most. 