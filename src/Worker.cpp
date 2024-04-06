
#include "Worker.hpp"

#include <nstd/Console.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>
#include <nstd/Directory.hpp>
#include <nstd/Error.hpp>
#include <nstd/Process.hpp>

Worker::Worker(const Settings& settings, Socket& client, ICallback& callback)
    : _settings(settings)
    , _callback(callback)
    , _finished(false)
{
    _client.swap(client);
    _thread.start(*this, &Worker::main);
}

bool Worker::join()
{
    {
        Mutex::Guard guard(_mutex);
        if (!_finished)
            return false;
    }
    _thread.join();
    return true;
}


uint Worker::main()
{
    handleRequest();
    {
        Mutex::Guard guard(_mutex);
        _finished = true;
    }
    _callback.onFinished();
    return 0;
}

namespace {

bool readHttpRequest(Socket& client, String& header, String& body)
{
    byte buffer[1024];
    usize size = 0;
    for (;;)
    {
        ssize i = client.recv(buffer + size, sizeof(buffer) - 1 - size);
        if (i <= 0)
            return false;
        size += i;
        buffer[i] = '\0';
        const char* headerEnd = String::find((const char*)buffer, "\r\n\r\n");
        if (headerEnd)
        {
            header = String((const char*)buffer, headerEnd - (const char*)buffer);
            const char* contentLengthAttr = header.find("\r\nContent-Length: ");
            if (contentLengthAttr)
            {
                contentLengthAttr += 18;
                const char* lineEnd = String::find(contentLengthAttr, "\r\n");
                if (!lineEnd)
                    lineEnd = (const char*)header + header.length();
                String contentLengthStr = header.substr(contentLengthAttr - (const char*)header, lineEnd - contentLengthAttr);
                uint64 contentLength = contentLengthStr.toUInt64();
                body.clear();
                body.reserve(contentLength);
                body.append(headerEnd + 4, size - (headerEnd + 4 - (const char*)buffer));
                byte* buf = (byte*)(char*)body;
                while (body.length() < contentLength)
                {
                    ssize i = client.recv(buf + body.length(), contentLength - body.length());
                    if (i <= 0)
                        return false;
                    body.resize(body.length() + i);
                }
            }
            return true;
        }
        if (size == sizeof(buffer) - 1)
            return false;
    }
}

bool parseHeader(const String& header, String& method, String& path)
{
    const char* n = header.find("\r\n");
    if (!n)
        return false;
    String firstLine = header.substr(0, n - (const char*)header);
    List<String> tokens;
    firstLine.split(tokens, " ");
    if (tokens.size() < 2)
        return false;
    method = tokens.front();
    path = *(++tokens.begin());
    return true;
}

String getRequestUrl(const String& path)
{
    String result = path.startsWith("/") ? path.substr(1) : path;
    bool https = result.startsWith("https://");
    if (!https && !result.startsWith("http://"))
        result.prepend("http://");
    return result;
}

bool parseGetRequestPath(const String& path, String& url, String& repoUrl, String& repo, String& service)
{
    url = getRequestUrl(path);
    const char* x = url.find("/info/refs?service=");
    if (!x)
        return false;
    usize split = x - (const char*)url;
    repoUrl = url.substr(0, split);
    service = url.substr(split + 19);
    bool https = url.startsWith("https://");
    usize serverStart = https ? 8 : 7;
    repo = url.substr(serverStart, split - serverStart);
    return true;
}


bool parsePostRequestPath(const String& path, String& url, String& repoUrl, String& repo, String& service)
{
    url = getRequestUrl(path);
    if (!url.endsWith("/git-upload-pack"))
        return false;
    repoUrl = url.substr(0, url.length() - 16);
    service = url.substr(repoUrl.length() + 1);
    bool https = url.startsWith("https://");
    usize serverStart = https ? 8 : 7;
    repo = url.substr(serverStart, repoUrl.length() - serverStart);
    return true;
}

}

void Worker::handleRequest()
{
    String header;
    String body;
    if (!readHttpRequest(_client, header, body))
        return;
    String method;
    String path;
    if (!parseHeader(header, method, path))
        return;

    // todo: auth
    // logic? 
    // if auth is in header check auth with uplink
    // else check if uplink can be accessed without auth
    // else ? request auth

    if (method == "GET")
        handleGetRequest(path);
    else if (method == "POST")
        handlePostRequest(path, body);
}

void Worker::handleGetRequest(const String& path)
{
    String requestUrl;
    String repoUrl;
    String repo;
    String service;
    if (!parseGetRequestPath(path, requestUrl, repoUrl, repo, service))
        return;
    if (service != "git-upload-pack")
        return;

    // todo: avoid concurrent git fetch !

    // create cache dir
    String cacheDir = _settings.cacheDir + "/" + repo;
    if (!Directory::create(cacheDir))
        return Log::errorf("Could not create directory '%s': %s", (const char*)cacheDir, (const char*)Error::getErrorString());

    // update cache
    {
        String command = String("git -C \"") + cacheDir + "\" fetch --quiet --prune --prune-tags";
        Console::printf("%s\n", (const char*)command);
        Process process;
        uint32 pid = process.start(command);
        if (!pid)
            return Log::errorf("Could not launch command '%s': %s", (const char*)command, (const char*)Error::getErrorString());
        uint32 exitCode = 1;
        process.join(exitCode);
        if (exitCode != 0)
        {
            command = String("git clone --quiet --mirror \"") + repoUrl + "\" \"" + cacheDir + "\"";
            Console::printf("%s\n", (const char*)command);
            uint32 pid = process.start(command);
            if (!pid)
                return Log::errorf("Could not launch command '%s': %s", (const char*)command, (const char*)Error::getErrorString());
            uint32 exitCode = 1;
            process.join(exitCode);
            if (exitCode != 0)
                return;
        }
    }

    // info response
    {
        String command = String("git-upload-pack --stateless-rpc --advertise-refs \"") + cacheDir + "\"";
        Console::printf("%s\n", (const char*)command);
        Process process;
        uint32 pid = process.open(command);
        if (!pid)
            return Log::errorf("Could not launch command '%s': %s", (const char*)command, (const char*)Error::getErrorString());

        String response = "HTTP/1.1 200 OK\r\n";
        response.append(String("Content-Type: application/x-") + service +"-advertisement\r\n");
        response.append("Cache-Control: no-cache\r\n\r\n");
        response.append("001e# service=git-upload-pack\n0000");
        if (_client.send((const byte*)(const char*)response, response.length()) != response.length())
            return;

        byte buf[0xffff];
        ssize len;
        for (;;)
        {
            len = process.read(buf, sizeof(buf));
            if (len < 0)
                return;
            if (len == 0)
                break;
            if (_client.send(buf, len) != len)
                return;
        }
    }
}

void Worker::handlePostRequest(const String& path, String& body)
{
    String requestUrl;
    String repoUrl;
    String repo;
    String service;
    if (!parsePostRequestPath(path, requestUrl, repoUrl, repo, service))
        return;
    if (service != "git-upload-pack")
        return;

    String cacheDir = _settings.cacheDir + "/" + repo;

    {
        String command = String("git-upload-pack --stateless-rpc \"") + cacheDir + "\"";
        Console::printf("%s\n", (const char*)command);
        Process process;
        uint32 pid = process.open(command, Process::stdoutStream | Process::stdinStream);
        if (!pid)
            return Log::errorf("Could not launch command '%s': %s", (const char*)command, (const char*)Error::getErrorString());

        if (process.write((const char*)body, body.length()) != body.length())
            return;

        String response = "HTTP/1.1 200 OK\r\n";
        response.append(String("Content-Type: application/x-") + service +"-advertisement\r\n");
        response.append("Cache-Control: no-cache\r\n\r\n");
        if (_client.send((const byte*)(const char*)response, response.length()) != response.length())
            return;

        byte buf[0xffff];
        ssize len;
        for (;;)
        {
            len = process.read(buf, sizeof(buf));
            if (len < 0)
                return;
            if (len == 0)
                break;
            if (_client.send(buf, len) != len)
                return;
        }
    }
}
