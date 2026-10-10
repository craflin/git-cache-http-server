
#pragma once

#include <nstd/String.hpp>

void storeUpdateTime(const String& repo, int64 time);
int64 getUpdateTime(const String& repo);
