#pragma once
#ifndef CITATION_H
#define CITATION_H

#include <string>

class Citation
{
    std::string id;

public:
    Citation(std::string x) : id(x) {}
    virtual void print() = 0;
    std::string getid()
    {
        return id;
    }
};

class CitBook : public Citation
{
    std::string isbn;

public:
    std::string author{"Pendragon"};
    std::string title{"Hello"};
    std::string p{"Press"};
    int year{}; //! test
    CitBook(std::string x, std::string i) : isbn(i), Citation(x) {}
    void print() override
    {
        std::cout << "[" << getid() << "] book: " << author << ", " << title << ", " << p << ", " << year << std::endl;
    }
};
class CitWeb : public Citation
{
    std::string url;

public:
    std::string title{"sword"}; //! test
    CitWeb(std::string x, std::string i) : url(i), Citation(x) {}
    void print() override
    {
        std::cout << "[" << getid() << "] webpage: " << title << ". Available at " << url << std::endl;
    }
};
class CitArt : public Citation
{
    std::string title;
    std::string author;
    std::string journal;
    int year;
    int volume;
    int issue;

public:
    CitArt(std::string x, std::string t, std::string a, std::string j, int y, int v, int i) : title(t), author(a), journal(j), year(y), volume(v), issue(i), Citation(x) {}
    void print() override
    {
        std::cout << "[" << getid() << "] article: " << author << ", " << title << ", " << journal << ", " << year << ", " << volume << ", " << issue << std::endl;
    }
};

#endif