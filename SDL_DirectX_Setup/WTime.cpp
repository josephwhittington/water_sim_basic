#include "WTime.h"

WTime::WTime()
{
	time = 0;
	lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now().time_since_epoch());
	deltaTime = 0;
	timeScale = 1;
	fps = 0;
}

void WTime::Update()
{
	// Time
	std::chrono::milliseconds currenttime = std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now().time_since_epoch());
	deltaTime = (double)(currenttime - lastTime).count();
	deltaTime /= 1000;

	// Set last time
	lastTime = currenttime;
	time += deltaTime;
	fps = 60 / deltaTime;
}

void WTime::ResetTime()
{
	time = 0;
}