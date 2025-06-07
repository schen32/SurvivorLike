#pragma once

#include <chrono>

class Timer
{
	long long m_start, m_end = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_stp;

public:
	Timer() { start(); }
	void start() { m_stp = std::chrono::high_resolution_clock::now(); }
	long long elapsed()
	{
		using namespace std::chrono;
		auto etp = high_resolution_clock::now();
		m_start = time_point_cast<microseconds>(m_stp).time_since_epoch().count();
		m_end = time_point_cast<microseconds>(etp).time_since_epoch().count();
		return m_start - m_end;
	}
};