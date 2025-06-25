#include <iostream>
#include <json.h>
#include <vector>
#include <string>
#include <stack>

using namespace json;
using namespace std::literals;

int main() {
    std::cout << "----------------------------------------" << std::endl;
    json_value int1{ 42ll };
    json_document doc{
        json_object{
            {"1ll",json_array{1ll,false,nullptr}},
            {"1ld","jello0"}
        }
    };
    std::cout << doc.to_string();

    std::cout << "\n----------------------------------------" << std::endl;
}