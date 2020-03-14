#include "Application.h"
#include "Controls.h"
#include <string>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // parse args
    Controls::InputMode inputMode = Controls::NORMAL;
    std::string recordFilename;
    Application::RenderMode renderMode = Application::RENDER_OPENGL;

    std::string usage = "Usage: ./SeniorProject [-r filename] [-p filename] [-t filename]\n" \
        "\t-r: record input to file\n" \
        "\t-p: playback input from file\n" \
        "\t-t: render frames with raytracing";

    int opt;
    while ((opt = getopt(argc, argv, "r:p:t:")) != -1) {
        switch (opt) {
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

    app.run(inputMode, recordFilename, renderMode);
    return 0;
}
