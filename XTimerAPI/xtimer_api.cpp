#include "xtimer_api.h"

#if defined(_WIN32)
#include "xtimer_api_win.cpp"
#else
#include "xtimer_api_unix.cpp"
#endif

#include <string>
#include <chrono>

void xtimer::test()
{
	xtimer_send("test");
}

void xtimer::reset()
{
	xtimer_send("reset");
}

void xtimer::start()
{
	xtimer_send((std::string("start ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str());
}

void xtimer::pause()
{
	xtimer_send((std::string("pause ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str());
}

void xtimer::resume()
{
	xtimer_send((std::string("resume ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str());
}

void xtimer::next()
{
	xtimer_send((std::string("next ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str());
}

void xtimer::prev()
{
	xtimer_send((std::string("prev ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str());
}

uint64_t xtimer::get_time_ms()
{
	char buf[64] = { 0 };
	xtimer_send((std::string("get_time_ms ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str(), buf, 64);
	return atoll(buf);
}

void xtimer::get_time_str(char* buf, size_t buf_size)
{
	xtimer_send((std::string("get_time_str ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str(), buf, buf_size);
}

std::string xtimer::get_time_str()
{
	char buf[64] = { 0 };
	xtimer_send((std::string("get_time_str ") + std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count())).c_str(), buf, 64);
	return std::string(buf);
}

void xtimer::set_time_ms(uint64_t time_ms)
{
	xtimer_send((std::string("set_time_ms ") + std::to_string(time_ms)).c_str()); // bad code yeah. i dont wanna bother too much with strings
}
