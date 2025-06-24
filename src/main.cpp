#include <iostream>
#include <json.h>
#include <vector>
#include <string>
#include <stack>

using namespace json;

int main() {
    std::cout << "----------------------------------------" << std::endl;
    std::string json_str = R"({
        "name": "John Doe",
        "age": 30,
        "is_student": false,
        "grades": [85.5, 90.0, 78.5],
        "address": {
            "street": "123 Main St",
            "city": "Anytown",
            "zip": "12345"
        },
        "courses": [
            {"name": "Math", "credits": 3},
            {"name": "Science", "credits": 4}
        ]
    })";
    json_document doc;
    doc.from_string(json_str);
    std::cout << doc.to_string();

    std::cout << "\n----------------------------------------" << std::endl;
}