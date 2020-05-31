#pragma once

#include <string>
#include <memory>
#include <INIReader.h>

class Settings {
public:
    void load(const std::string &file);
    void reload();
    std::unique_ptr<INIReader> map;
private:
    std::string file;
};