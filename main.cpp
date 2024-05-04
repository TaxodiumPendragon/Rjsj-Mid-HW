#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

#include "json.hpp"
#include "httplib.h"
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
        if (!cite["type"].is_string() || !cite["id"].is_string())
        {
            printf("ID or type Error");
            std::exit(1);
        }
        std::string t = cite["type"].get<std::string>();
        std::string id = cite["id"].get<std::string>();

        if (t == "book")
        {
            if (!cite["isbn"].is_string())
            {
                printf("ISBN Error!");
                std::exit(1);
            }
            citation = new CitBook(id, cite["isbn"]);
        }

        else if (t == "webpage")
        {
            if (!cite["url"].is_string())
            {
                std::exit(1);
            }
            citation = new CitWeb(id, cite["url"]);
        }
        else if (t == "article")
        {
            if (!cite["year"].is_number_integer() || !cite["volume"].is_number_integer() || !cite["issue"].is_number_integer() ||
                !cite["title"].is_string() || !cite["author"].is_string() || !cite["journal"].is_string())
            {
                std::exit(1);
            }
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

httplib::Client cli{API_ENDPOINT};

void CitWeb::ask()
{
    auto result = cli.Get("/title/" + encodeUriComponent(url));
    if (result && result->status == httplib::OK_200)
    {
        nlohmann::json t = nlohmann::json::parse(result->body);
        if (t["title"].is_string())
        {
            std::exit(1);
        }
        title = t["title"].get<std::string>();
    }
    else
    {
        auto err = result.error();
        std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl;
    }
}

void CitBook::ask()
{
    auto result = cli.Get("/isbn/" + encodeUriComponent(isbn));
    if (result && result->status == httplib::OK_200)
    {
        nlohmann::json t = nlohmann::json::parse(result->body);
        if (t["author"].is_string() || t["title"].is_string() || t["publisher"].is_string() || t["year"].is_string())
        {
            std::exit(1);
        }
        author = t["author"].get<std::string>();
        title = t["title"].get<std::string>();
        p = t["publisher"].get<std::string>();
        year = t["year"].get<std::string>();
    }
    else
    {
        auto err = result.error();
        std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl;
    }
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
            std::cout << "Command Error" << std::endl;
            std::exit(1);
        }
    }
    // "docman", "-c", "citations.json", "input.txt"
    // TODO 处理错误：文章的中括号有误
    auto citations = loadCitations(citationPath);

    // read all input to the string
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
    if (!stdo)
    {
        std::ofstream file(outputPath);
        if (!file)
        {
            std::cerr << "打不开文件：" << outputPath << std::endl;
            std::exit(1);
        }
        std::ostream &output = file;
    }
    output << input; // print the paragraph first
    output << "\nReferences:\n";

    std::vector<Citation *> printedCitations{};
    // TODO process citations in the input text
    std::vector<std::string> ids;
    std::string::size_type pos = 0;
    std::string::size_type endPos = 0;
    while ((pos = input.find('[', pos)) != std::string::npos)
    {
        endPos = input.find(']', pos);
        std::string::size_type nextPos = input.find('[', pos + 1);
        std::string::size_type prevEndPos = input.rfind(']', pos);
        if (endPos == std::string::npos || (nextPos != std::string::npos && nextPos < endPos) || (prevEndPos != std::string::npos && prevEndPos > pos))
        {
            std::exit(1);
        }
        std::string idStr = input.substr(pos + 1, endPos - pos - 1);
        ids.push_back(idStr);
        pos = endPos;
    }

    for (const auto &id : ids)
    {
        for (auto c : citations)
        {
            if (c->getid() == id)
            {
                printedCitations.push_back(c);
                c->ask();
                break;
            }
        }
    }
    for (auto c : printedCitations)
    {
        // 打印引用格式输出
        c->print(output);
    }

    for (auto c : citations)
    {
        delete c;
    }
}
