
#include <nstd/String.hpp>
#include <nstd/Console.hpp>
#include <nstd/Process.hpp>

int main(int argc, char* argv[])
{
    String arg1;
    if (argc > 1)
        arg1.attach(argv[1], String::length(argv[1]));
    if (arg1.startsWith("Username"))
        Console::print(Process::getEnvironmentVariable("GCHSD_USERNAME"));
    else if (arg1.startsWith("Password"))
        Console::print(Process::getEnvironmentVariable("GCHSD_PASSWORD"));
    else
        return 1;
    return 0;
}
