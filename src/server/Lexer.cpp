#include "Lexer.h"

void Lexer::lex(std::string str, std::string arg[], char seprator) {
    std::string word = "";
    int i = 0;
    for (auto x : str) {
        if (x == seprator)
        {
            i++;
            arg[i] = word;
            word = "";
        }
        else
        {
            word = word += x;
        }
        arg[i] = word;
    }
}

void Lexer::lexSpace(std::string str, std::string arg[]) {
    lex(str, arg, ' ');
}