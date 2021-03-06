#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include "server_proxy.h"

class HttpServerProxy : public ServerProxy
{
public:
    static bool isMatch(const char *request, int len);

protected:
	void Process(int srcfd, const char *request, int len) const;

private:
    bool ParseIpPort(std::string &domain, uint32_t &ip, uint16_t &port) const;
};

#endif // HTTPPROXY_H
