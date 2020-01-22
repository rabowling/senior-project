#pragma once

#include <string>
#include <vector>
#include <dirent.h>

std::vector<std::string> listDir(std::string dir) {
    struct dirent *entry = nullptr;
    DIR *dp = opendir(dir.c_str());
    std::vector<std::string> dirs;

    while ((entry = readdir(dp))) {
        dirs.push_back(std::string(entry->d_name));
    }

    closedir(dp);
    return dirs;
}