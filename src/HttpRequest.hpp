

#pragma once

#include <nstd/String.hpp>
#include <nstd/Buffer.hpp>

class HttpRequest
{
public:
  HttpRequest();
  ~HttpRequest();

  const String& getErrorString() {return error;}

  bool get(const String& url, Buffer& data, bool checkCertificate = true);

private:
  void* curl;
  String error;
};
