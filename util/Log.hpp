#pragma once

#include <sstream>
#include <iostream>
using namespace std;

struct Log {
	template<class T>
	Log& operator<<(const T& t) {
		ss << t;
		return *this;
	}
	~Log() {
		if (!ignore) {
			cout<<ss.str()<<'\n';
			cout.flush();
		}
	}
	Log(): ignore(0) {}
	Log(std::string channel);

private:
	std::ostringstream ss;
	bool ignore;
};

#define LOG_CHANNEL(name) static std::string logChannel = #name
#define LOG Log(logChannel)
