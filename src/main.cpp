#include <iostream>
#include <json.h>
#include <vector>
#include <string>
#include <stack>

using namespace json;

int main() {
    std::cout << "----------------------------------------" << std::endl;

    std::string json_str = R"([1,    2, {"key" "value"}])";
    json_document doc;
    doc.from_string(json_str);
    if (doc.is_valid()) {
        std::cout << "Parsed JSON: " << doc.to_string() << std::endl;
    } else {
        std::cout << "Error parsing JSON: " << doc.error_message() << std::endl;
    }

    std::cout << "\n----------------------------------------" << std::endl;
}