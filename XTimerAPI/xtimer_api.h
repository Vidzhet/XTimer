#pragma once
#include <stdint.h>
#include <string>

namespace xtimer {
	void test();
	void reset();
	void start();
	void pause();
	void resume();
	void next();
	void prev();
	uint64_t get_time_ms();
	void get_time_str(char* buf, size_t buf_size);
	std::string get_time_str();
	void set_time_ms(uint64_t time_ms);
}