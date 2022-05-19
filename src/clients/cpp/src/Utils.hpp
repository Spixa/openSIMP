#include <string>
class Utils {
public:
    // lexer functionality for commands
    static void lexer(std::string str, std::string arg[], char divider)
    {
            std::string word = ""; // Reset string
            int i = 0; // counter
            for (auto x : str) // count against size of string
            {
                if (x == divider) // if it stumbled upon the letter we want it to divide
                {
                    i++; // add once to iter counter
                    arg[i] = word; // save it to the iter from this array
                    word = ""; // reset the string again.
                }
                else
                    word = word += x; // else, add the char to the string
                
                arg[i] = word;
            }
    }
};