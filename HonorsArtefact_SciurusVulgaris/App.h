#pragma once

/// <summary>
/// Abstract App class for general functionality of the 2 parts of the application. 
/// </summary>
class App
{
public:
	App() {};

	/// <summary>
	/// Ran once at the start of an app
	/// </summary>
	virtual void Initialize() = 0;
	/// <summary>
	/// Ran every frame
	/// </summary>
	virtual void Frame() = 0;
	/// <summary>
	/// Ran once at the end of an app
	/// </summary>
	virtual void Shutdown() = 0;

	bool quitMainLoop = false; // If this is true, the loop the app is contained in should break.
};

