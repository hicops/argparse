/* @author: Morris Franken
 * This argparse version relies on copy elision to make sure the same variable is used
 * STATUS: not working, copy elision not forced for trivial copyable types.
 * Awaiting http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2025r1.html
 */
#include <iostream>
#include <cstring>

#include "argparse/argparse.hpp"

using namespace std;

enum Color {
    RED,
    BLUE,
    GREEN,
};

struct Custom {
    std::string message;

    Custom() = default;
    Custom(const std::string &v) {
        message = v;
    }
};

std::pair<int, char**> get_argc_argv(std::string &str) {
    std::stringstream ss(str);
    std::string key;
    std::vector<char*> splits = {(char *)str.c_str()};
    for (int i = 1; i < str.size(); i++) {
        if (str[i] == ' ') {
            str[i] = '\0';
            splits.emplace_back(&str[++i]);
        }
    }
    char** argv = new char*[splits.size()];
    for (int i = 0; i < splits.size(); i++) {
        argv[i] = splits[i];
    }

    return {(int)splits.size(), argv};
}

void TEST_ALL() {
    struct Args : public argparse::Args {
        std::string& src_path           = arg("Source path");
        std::string& dst_path           = arg("Destination path").set_default("world");// default value set to "world"
        std::vector<std::string>& others= arg("Others").multi_argument().set_default<std::vector<std::string>>({});// default value set to empty vector
        int& k                          = kwarg("k", "A required parameter (short only)", "3");   // Implicit value set to 3
        std::shared_ptr<float>& alpha   = kwarg("a,alpha", "An optional float parameter");                    // pointers have a default value of nullptr
        float& beta                     = kwarg("b,beta", "An optional float parameter with default set as float").set_default(0.6f);
        float& beta2                    = kwarg("beta2", "An optional float parameter with default set as string").set_default("0.6f");
        float*& gamma                   = kwarg("g,gamma", "An optional float parameter with implicit value", "0.5"); // pointers have a default value of nullptr
        Custom& custom                  = kwarg("c,custom", "A custom class");                                // Custom classes that support a std::string constructor
        std::vector<int>& numbers       = kwarg("n,numbers", "An optional vector of integers").set_default<std::vector<int>>({1,2});
        std::vector<int>& numbers2      = kwarg("numbers2", "An optional vector of integers").set_default("3,4,5");
        std::vector<std::string> &files = kwarg("files", "mutliple arguments").multi_argument();
        std::optional<float>& opt       = kwarg("o,optional", "An optional float parameter");
        Color &color                    = kwarg("c,color", "An Enum input");
        bool& flag1                     = flag("f,flag", "A test flag");
        bool& verbose                   = flag("v,verbose", "A flag to toggle verbose");

        CONSTRUCTOR(Args);
    };

    {
        std::string command = "argparse_test source_path destination -k=5 --alpha=1 --beta 3.3 --gamma --numbers=1,2,3,4,5 --numbers2 6,7,8 --files f1 f2 f3 --custom hello_custom --optional 1 -c red --verbose";
        const auto &[argc, argv] = get_argc_argv(command);
        Args args(argc, argv);
        assert(args.src_path == "source_path");
        assert(args.dst_path == "destination");
        assert(args.k == 5);
        assert(args.alpha != nullptr && std::abs(*args.alpha - 1) < 0.0001);
        assert(std::abs(args.beta - 3.3) < 0.0001);
        assert(std::abs(args.beta2 - 0.6) < 0.0001);
        assert(args.gamma != nullptr && std::abs(*args.gamma - 0.5) < 0.0001);
        assert(args.numbers.size() == 5 && args.numbers[2] == 3);
        assert(args.numbers2.size() == 3 && args.numbers2[2] == 8);
        assert(args.files.size() == 3 && args.files[2] == "f3");
        assert(std::abs(args.opt.value() - 1.0f) < 0.0001);
        assert(args.custom.message == "hello_custom");
        assert(args.color == RED);
        assert(args.flag1 == false);
        assert(args.verbose);
    }

    {
        std::string command = "argparse_test source_path -k --files f1 f2 f3 --custom hello_custom --optional 1 -c red --verbose";
        const auto &[argc, argv] = get_argc_argv(command);
        Args args(argc, argv);
        assert(args.src_path == "source_path");
        assert(args.dst_path == "world");
        assert(args.k == 3);
        assert(args.alpha == nullptr);
        assert(std::abs(args.beta - 0.6) < 0.0001);
        assert(std::abs(args.beta2 - 0.6) < 0.0001);
        assert(args.gamma == nullptr);
        assert(args.numbers.size() == 2 && args.numbers[1] == 2);
        assert(args.numbers2.size() == 3 && args.numbers2[2] == 5);
        assert(args.files.size() == 3 && args.files[2] == "f3");
        assert(std::abs(args.opt.value() - 1.0f) < 0.0001);
        assert(args.custom.message == "hello_custom");
        assert(args.color == RED);
        assert(args.flag1 == false);
        assert(args.verbose);
    }
}


int main(int argc, char* argv[]) {
    TEST_ALL();
    return 0;
}