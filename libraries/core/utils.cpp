#include "sync.hpp"
#include "utils.hpp"


#include <iostream>
#include <string>
#include <vector>

#include <sstream>
#include <iterator>


unsigned long myCeil(unsigned long first, unsigned long second) {
    return first / second + ((first % second != 0) ? 1 : 0);
}

std::vector<std::string> split(const std::string& str, char separator) {
    std::vector<std::string> cont;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, separator)) {
        if(token.empty())
            continue;
        cont.push_back(token);
    }
    return cont;
}

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
std::string trim(const std::string& s)
{
    return rtrim(ltrim(s));
}
