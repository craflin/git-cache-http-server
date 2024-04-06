
#include <nstd/Console.hpp>
#include <nstd/Error.hpp>
#include <nstd/Log.hpp>
#include <nstd/Process.hpp>
#include <nstd/PoolList.hpp>

#include "Settings.hpp"
#include "Worker.hpp"

class Main
    : private Worker::ICallback
{
public:
    Main(const Settings& settings)
        : _settings(settings)
    {
        ;
    }

    bool start()
    {
        if (!_serverSocket.open() ||
            !_serverSocket.setReuseAddress() ||
            !_serverSocket.setNonBlocking() ||
            !_serverSocket.bind(_settings.listenAddr.addr, _settings.listenAddr.port) ||
            !_serverSocket.listen())
            return false;
        _poll.set(_serverSocket, Socket::Poll::acceptFlag);
        return true;
    }

    void run()
    {
        for (Socket::Poll::Event event; _poll.poll(event, 1000 * 1000);)
            if (event.socket == &_serverSocket)
            {
                uint32 ip;
                uint16 port;
                Socket client;
                if (_serverSocket.accept(client, ip, port))
                    _workers.append<const Settings&, Socket&, Worker::ICallback&>(_settings, client, *this);
            }
            else
            {
                for(PoolList<Worker>::Iterator i = _workers.begin(), next; i != _workers.end(); i = next)
                {
                    next = i;
                    ++next;
                    if (i->join())
                        _workers.remove(i);
                }
            }
    }

private:
    const Settings& _settings;
    Socket _serverSocket;
    Socket::Poll _poll;
    PoolList<Worker> _workers;

private:
    void onFinished() override { _poll.interrupt(); }
};

int main(int argc, char* argv[])
{
    String logFile;
    String configFile("/etc/gchsd.conf");

    // parse parameters
    {
        Process::Option options[] = {
            { 'b', "daemon", Process::argumentFlag | Process::optionalFlag },
            { 'c', "config", Process::argumentFlag },
            { 'h', "help", Process::optionFlag },
        };
        Process::Arguments arguments(argc, argv, options);
        int character;
        String argument;
        while (arguments.read(character, argument))
            switch (character)
            {
            case 'b':
                logFile = argument.isEmpty() ? String("/dev/null") : argument;
                break;
            case 'c':
                configFile = argument;
                break;
            case '?':
                Console::errorf("Unknown option: %s.\n", (const char*)argument);
                return -1;
            case ':':
                Console::errorf("Option %s required an argument.\n", (const char*)argument);
                return -1;
            default:
                Console::errorf("Usage: %s [-b] [-c <file>]\n\
\n\
    -b, --daemon[=<file>]\n\
        Detach from calling shell and write output to <file>.\n\
\n\
    -c <file>, --config[=<file>]\n\
        Load configuration from <file>. (Default is /etc/gchsd.conf)\n\
\n",
                    argv[0]);
                return -1;
            }
    }

    Log::setLevel(Log::debug);

    // load settings
    Settings settings;
    Settings::loadSettings(configFile, settings);

    // daemonize process
#ifndef _WIN32
    if (!logFile.isEmpty())
    {
        Log::infof("Starting as daemon...");
        if (!Process::daemonize(logFile))
        {
            Log::errorf("Could not daemonize process: %s", (const char*)Error::getErrorString());
            return -1;
        }
        Log::setDevice(Log::syslog);
        Log::setLevel(Log::info);
    }
#endif

    // start the server
    Main main(settings);
    if (!main.start())
        return Log::errorf("Could not listen on TCP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.listenAddr.addr), (uint16)settings.listenAddr.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)settings.listenAddr.port);

    // run the server
    main.run();
    return 1;
}
