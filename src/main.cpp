#include <iostream>
#include <json.h>

using namespace std::literals;
using namespace json;

int main() {


    std::cout << "\n-----------------------\n";

    std::string str = "Boo";
    std::string& str_ref = str;

    json_document doc{
        str,
        1,
        false,
        json_object{
            { "key1", "value1" },
            { "key2", 42 },
            { "key3", true },
            { "key4", 3.14 },
            { "key5",
                {
                    "item1",
                    "item2",
                    "item3",
                    false,
                    json_object {
                        {"key1", false},
                        {"key7", "value"s}
                    }
                }
            }
        },
        {
            false,nullptr,"hello",123,5.66
        }
    };

    std::cout << "Document root: " << doc.to_string() << "\n";


    std::cout << "\n-----------------------\n";
    return 0;
}