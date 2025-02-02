

#pragma once

#include <nstd/String.hpp>
#include <nstd/Buffer.hpp>
#include <nstd/HashMap.hpp>

class HttpRequest
{
public:
  HttpRequest();
  ~HttpRequest();

  const String& getErrorString() {return error;}

  bool get(const String& url, Buffer& data, const HashMap<String, String>& headerFields = HashMap<String, String>(), bool checkCertificate = true);

private:
  void* curl;
  String error;
};
