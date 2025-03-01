
#pragma once

#include <nstd/String.hpp>

#include "Address.hpp"

struct Settings
{
    String askpassPath;
    Address listenAddr;
    String cacheDir;

    Settings();

    static void loadSettings(const String& file, Settings& settings);
};
