#include "Settings.h"

void Settings::load(const std::string &file) {
    this->file = file;
    reload();
}

void Settings::reload() {
    map = std::make_unique<INIReader>(file);
}
