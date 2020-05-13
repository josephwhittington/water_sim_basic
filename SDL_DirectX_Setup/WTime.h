#pragma once
#include <chrono>

class WTime
{
private:
	std::chrono::high_resolution_clock m_clock;
	std::chrono::milliseconds lastTime;
public:
	double deltaTime;
	double timeScale;
	double fps;
	double time;

	WTime();
	void Update();
	void ResetTime();
};