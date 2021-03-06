#ifndef PROXY_H
#define PROXY_H
#include "passwd.h"
#include "encrypt.h"
#include <unistd.h>
#include <math.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAXBUF 8192

class Proxy
{
public:
    Proxy();
	Proxy(const Proxy& proxy);
    virtual ~Proxy();
	Proxy& operator=(const Proxy& proxy);
	static void Run(int srcfd);

protected:
	virtual void Process(int srcfd) = 0;
    void ForwardData(int srcfd, int tarfd) const;
    EncryptBase* encrypter;

private:
    int ForwardData(int srcfd, int tarfd, bool isRequest) const;
};

#endif
