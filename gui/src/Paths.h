#pragma once
#include <string>
#include "files.2/files.h"

struct Paths
{
#ifdef mydebug
    std::string fxpipe_core = "/media/romain/Donnees/Programmation/cpp/cmake/fxpipe/core/build";
#else
    std::string fxpipe_core = "/opt/fxpipe";
#endif
};
