#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

#include "third_parties/nlohmann/json.hpp"
#include "utils.hpp"
#include "citation.h"

std::string readFromFile(const std::string &inputPath)
{
    std::ifstream file(inputPath);
    if (!file)
    {
        std::cerr << "无法打开文件: " << inputPath << std::endl;
        std::exit(1);
    }

    std::string text;
    std::string line;
    while (std::getline(file, line))
    {
        text += line + "\n";
    }

    return text;
}

std::vector<Citation *> loadCitations(const std::string &filename)
{
    // load citations from file
    std::vector<Citation *> citations;
    std::ifstream file{filename};
    if (!file.is_open())
    {
        std::exit(1);
    }
    nlohmann::json data = nlohmann::json::parse(file);
    for (auto &cite : data["citations"])
    {
        Citation *citation = nullptr;
        std::string t = cite["type"];
        std::string id = cite["id"];

        if (t == "book")
        {
            citation = new CitBook(id, cite["isbn"]);
        }
        else if (t == "webpage")
        {
            citation = new CitWeb(id, cite["url"]);
        }
        else if (t == "article")
        {
            int year = cite["year"].get<int>();
            int volume = cite["volume"].get<int>();
            int issue = cite["issue"].get<int>();
            citation = new CitArt(id, cite["title"], cite["author"], cite["journal"], year, volume, issue);
        }
        if (citation != nullptr)
        {
            citations.push_back(citation);
        }
        else
            std::exit(1);
    }
    return citations;
}

int main(int argc, char **argv)
{
    bool stdo{true};
    bool stdi{true};
    // 处理一些命令行参数错误输入情况，处理了命令行本身参数有误的可能情况
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
        else if (arg == "-")
            break;
        else
        {
            // 错误的参数
            std::exit(1);
        }
    }
    // "docman", "-c", "citations.json", "input.txt"
    // TODO 处理错误：文章的中括号有误
    auto citations = loadCitations(citationPath);
    std::vector<Citation *> printedCitations{citations};

    // FIXME: read all input to the string, and process citations in the input text
    std::string input;
    if (stdi)
    {
        std::string line;
        while (std::getline(std::cin, line))
        {
            if (line.empty())
            {
                break;
            }
            input += line + "\n";
        }
    }
    else
    {
        input = readFromFile(inputPath);
    }
    std::ostream &output = std::cout;
    output << input; // print the paragraph first
    output << "\nReferences:\n";

    for (auto c : printedCitations)
    {
        // FIXME: 打印引用格式输出
        c->print();
    }

    for (auto c : citations)
    {
        delete c;
    }
}
