
#include "Settings.hpp"

#include <nstd/File.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>
#include <nstd/Directory.hpp>
#include <nstd/Process.hpp>

Settings::Settings()
    : askpassPath(File::getDirectoryName(Process::getExecutablePath()) + "/gchsd-askpass")
    , listenAddr{Socket::anyAddress, 80}
    , cacheDir(Directory::getTempDirectory() + "/gchsd")
{
    ;
}

void Settings::loadSettings(const String& file, Settings& settings)
{
    String conf;
    if (!File::readAll(file, conf))
        return;
    List<String> lines;
    conf.split(lines, "\n\r");
    for (List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
    {
        String line = *i;
        const char* lineEnd = line.find('#');
        if (lineEnd)
            line.resize(lineEnd - (const char*)line);
        line.trim();
        if (line.isEmpty())
            continue;
        List<String> tokens;
        line.split(tokens, " \t");
        if (tokens.size() < 2)
            continue;
        const String& option = *tokens.begin();
        const String& value = *(++tokens.begin());
        if (option == "cacheDir")
            settings.cacheDir = value;
        else if (option == "listenAddr")
            settings.listenAddr.addr = Socket::inetAddr(value, &settings.listenAddr.port);
        else
            Log::warningf("Unknown option: %s", (const char*)option);
    }
}
