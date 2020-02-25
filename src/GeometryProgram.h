#pragma once

#ifndef GEOMETRY_PROGRAM_H
#define GEOMETRY_PROGRAM_H

#include "Program.h"

class GeometryProgram : public Program {
    public:
        bool init();
        void setShaderNames(const std::string &v, const std::string &f, const std::string &g);

    protected:
        std::string gShaderName;
};

#endif