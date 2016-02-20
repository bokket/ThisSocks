#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include "Logger.h"

class Utils
{
public:
	static bool TrimStart(std::string &str);
	static bool TrimEnd(std::string &str);
	static bool Trim(std::string &str);
	static void Split(
			const std::string &str, char c, std::vector<std::string> &vec);

    static std::string GetSocketPair(int connfd);
};
#endif