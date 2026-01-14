#pragma once
class App
{
public:
	App() {};

	virtual void Initialize() = 0;
	virtual void Frame() = 0;
	virtual void Shutdown() = 0;

	bool quitMainLoop = false;
};

