#include "server_proxy.h"
#include "http_proxy.h"
#include "https_proxy.h"
#include "socks_proxy.h"
#include "logger.h"
#include "recorder.h"

using std::string;

void ServerProxy::Process(int srcfd)
{
	Recorder::CreateKey();
	encrypter = GEncryptFactory.GetEncrypter();
	if (!encrypter->SetServerFd(srcfd)) {
		return;
	}

	if (!ValidateProxyClient()) {
		return;
	}

	int len;
	char request[MAXBUF];
	if ((len = encrypter->Read(request, sizeof(request))) < 0) {
		GLogger.LogErr(LOG_NOTICE, "read proxy request error");
		return;
	}

	ServerProxy* proxy = SelectServerProxy(request, len);
	if (proxy == NULL) {
		return;
	}
	proxy->encrypter = encrypter->clone();
	proxy->Process(srcfd, request, len);
	delete proxy;
}

void ServerProxy::Process(int src, const char* request, int len) const
{
}

/*
 * 用户名密码验证
 * 成功返回true，回复0
 * 用户名或密码错误返回false，回复1
 * 其他错误返回false，不发送回复
 */
bool ServerProxy::ValidateProxyClient() const
{
    char buf[MAXBUF];
    int readn = encrypter->Read(buf, sizeof(buf));
    if (readn < 1 || buf[0] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read username length error");
        return false;
    }

    int userlen = buf[0];
    if (readn < 1 + userlen) {
        GLogger.LogErr(LOG_NOTICE, "read usrename error");
        return false;
    }
    string username = string(buf + 1, userlen);

    if (readn < 1 + userlen + 1 || buf[1 + userlen] <= 0) {
        GLogger.LogErr(LOG_NOTICE, "read password length error");
        return false;
    }
    int pwdlen = buf[1 + userlen];
    if (readn < 1 + userlen + 1 + pwdlen) {
        GLogger.LogErr(LOG_NOTICE, "read password error");
        return false;
    }
    string passwd = string(buf + 1 + userlen + 1, pwdlen);

    char res[2];
    if (GPasswd.IsValidUser(username, passwd)) {
		Recorder::RecordUser(username);
        res[0] = 0;
        if (1 != encrypter->Write(res, 1)) {
            GLogger.LogErr(LOG_ERR, "write auth result error");
            return false;
        }
        return true;
    }

    res[0] = 1;
    encrypter->Write(res, 1);
    return false;
}

int ServerProxy::ConnectRealServer(uint32_t ip, uint16_t port) const
{
	Recorder::RecordAddress(ip, port);

    struct sockaddr_in remoteaddr;
    bzero(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = ip;
    remoteaddr.sin_port = htons(port);

    int remotefd = socket(AF_INET, SOCK_STREAM, 0);
    if (remotefd == -1) {
        GLogger.LogErr(LOG_ERR, "create socket to real server error");
        return -1;
    }

    if (connect(remotefd,
            (struct sockaddr *)&remoteaddr,
            sizeof(remoteaddr)) < 0) {
        GLogger.LogErr(LOG_ERR, "connect() to real server error");
        return -1;
    }

    return remotefd;
}

ServerProxy* ServerProxy::SelectServerProxy(const char *request, int len) const
{
	if (SocksServerProxy::isMatch(request, len)) {
		return new SocksServerProxy();
	}
	if (HttpsServerProxy::isMatch(request, len)) {
		return new HttpsServerProxy();
	}
	return new HttpServerProxy();
}

uint32_t ServerProxy::GetIPv4ByName(const string& hostname) const
{
    Recorder::RecordHost(hostname);

	struct addrinfo hints;
	struct addrinfo* result;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int ret = 0;
	if ((ret = getaddrinfo(hostname.c_str(), NULL, &hints, &result)) != 0)  {
		GLogger.LogMsg(LOG_ERR, "getaddrinfo error: %s", gai_strerror(ret));
		return -1;
	}

	uint32_t ip = ((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(result);
	return ip;
}
