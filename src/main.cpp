#include <iostream>
#include <json.h>

using namespace std::literals;
using namespace json;

int main() {


    std::cout << "\n-----------------------\n";

    std::string str = "Hello, World!";

    json_array arr1{ std::move(str),2,false,4 };
    str = "Changed";
    json_array arr2{ 5,6,7,{"key","value","key2",false},{{"key",false},{"log","123123"}} };



    std::cout << "\n-----------------------\n";
    return 0;
}