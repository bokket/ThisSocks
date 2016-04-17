#ifndef COUNTER_H
#define COUNTER_H

#include <string>
#include <pthread.h>

class ThreadInfo
{
public:
	std::string User;
	unsigned int IP;
	unsigned int Port;
	unsigned int Upload;
	unsigned int Download;

	ThreadInfo();
	void Print();
};

class Counter
{
public:
	static void CreateKey();
	static void RecordUser(std::string user);
	static void RecordAddress(unsigned int ip, unsigned short port);
	static void RecordUpload(unsigned int size);
	static void RecordDownload(unsigned int size);

private:
	static void InitThread();
	static ThreadInfo* GetThreadInfo();
	static void DeleteKey(void* arg);
	static pthread_key_t pkey;
	static pthread_once_t once;
};

#endif // COUNTER_H