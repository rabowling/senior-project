#include "Application.h"

int main(int argc, char *argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(std::string(argv[i]));
    }
    app.run(args);
    return 0;
}
