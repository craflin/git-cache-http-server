
#pragma once

#include <nstd/String.hpp>

void storeAuth(const String& repo, const String& auth);
void removeAuth(const String& repo, const String& auth);
bool checkAuth(const String& repo, const String& auth);

