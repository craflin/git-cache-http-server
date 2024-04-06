
#pragma once

#include <nstd/String.hpp>

#include "Address.hpp"

struct Settings
{
    Address listenAddr;
    String cacheDir;

    Settings();

    static void loadSettings(const String& file, Settings& settings);
};
