#include "Application.h"
#include "Controls.h"
#include <string>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // parse args
    std::string levelFilename = "level1.txt";
    Controls::InputMode inputMode = Controls::NORMAL;
    std::string recordFilename;
    Application::RenderMode renderMode = Application::RENDER_OPENGL;

    std::string usage = "Usage: ./SeniorProject [-l filename] [-r filename] [-p filename] [-t filename]\n" \
        "\t-l: open level (default: level1.txt)\n"
        "\t-r: record input to file\n" \
        "\t-p: playback input from file\n" \
        "\t-t: render frames with raytracing";

    int opt;
    while ((opt = getopt(argc, argv, "l:r:p:t:")) != -1) {
        switch (opt) {
            case 'l':
                levelFilename = std::string(optarg);
                break;
            case 'r':
                recordFilename = std::string(optarg);
                inputMode = Controls::RECORD;
                break;
            case 'p':
                recordFilename = std::string(optarg);
                inputMode = Controls::PLAYBACK;
                break;
            case 't':
                recordFilename = std::string(optarg);
                inputMode = Controls::PLAYBACK;
                renderMode = Application::RENDER_RAYTRACE;
                break;
            default:
                std::cerr << usage << std::endl;
                return 1;
        }
    }

    app.run(levelFilename, inputMode, recordFilename, renderMode);
    return 0;
}
