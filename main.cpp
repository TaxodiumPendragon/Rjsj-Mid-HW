#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

#include "utils.hpp"
#include "citation.h"

//?感觉没有必要
bool fileExits(const std::string &filename)
{
    std::ifstream file(filename);
    return file.good();
}

std::vector<Citation *> loadCitations(const std::string &filename)
{
    // FIXME: load citations from file
}

int main(int argc, char **argv)
{
    bool stdo{true};
    bool stdi{true};
    // 处理一些命令行参数错误输入情况：命令行本身参数有误
    if (argc != 4 && argc != 6)
        std::exit(1);
    std::string citationPath;
    std::string inputPath;
    std::string outputPath;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-c" && i + 1 < argc)
        {
            citationPath = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc)
        {
            stdo = false;
            outputPath = argv[++i];
        }
        else
        {
            // 错误的参数
            std::exit(1);
        }
    }
    // "docman", "-c", "citations.json", "input.txt"
    // TODO 处理错误：文章的中括号有误
    auto citations = loadCitations(citationPath);
    std::vector<Citation *> printedCitations{};

    // FIXME: read all input to the string, and process citations in the input text
    // auto input = readFromFile(argv[3]);
    // ...

    std::ostream &output = std::cout;

    // output << input;  // print the paragraph first
    // output << "\nReferences:\n";

    for (auto c : printedCitations)
    {
        // FIXME: print citation
    }

    for (auto c : citations)
    {
        delete c;
    }
}
