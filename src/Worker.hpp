
#pragma once

#include <nstd/Mutex.hpp>
#include <nstd/Socket/Socket.hpp>
#include <nstd/Thread.hpp>

#include "Settings.hpp"

class Worker
{
public:
    class ICallback
    {
    public:
        virtual void onFinished() = 0;
    };

public:
    Worker(const Settings& settings, Socket& client, ICallback& callback);
    bool join();

private:
    const Settings& _settings;
    ICallback& _callback;
    Socket _client;
    Thread _thread;
    
    Mutex _mutex;
    bool _finished;

private:
    uint main();
    void handleRequest();
    void handleGetRequest(const String& path);
    void handlePostRequest(const String& path, String& body);
};
