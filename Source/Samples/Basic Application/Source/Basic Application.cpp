#include "../Include/Basic Application.h"
using namespace R5;

//============================================================================================================

TestApp::TestApp() : mWin(0), mGraphics(0), mUI(0), mCore(0)
{
	mWin		= new GLWindow();
	mGraphics	= new GLGraphics();
	mUI			= new UI(mGraphics, mWin);
	mCore		= new Core(mWin, mGraphics, mUI);
}

//============================================================================================================

TestApp::~TestApp()
{
	if (mCore)		delete mCore;
	if (mUI)		delete mUI;
	if (mGraphics)	delete mGraphics;
	if (mWin)		delete mWin;
}

//============================================================================================================

void TestApp::Run()
{
	if (*mCore << "Config/Basic Application.txt")
	{
		while (mCore->Update());
		//*mCore >> "Config/Basic Application.txt";
	}
}

//============================================================================================================

#include <windows.h>

R5_MAIN_FUNCTION
{
#ifdef _MACOS
	String path ( System::GetPathFromFilename(argv[0]) );
	System::SetCurrentPath(path.GetBuffer());
	System::SetCurrentPath("../../../");
#endif
	System::SetCurrentPath("../../../Resources/");

	typedef void* (*FUNC)();

	HMODULE lib = (HMODULE)LoadLibrary("Test.dll");
	FUNC getScripts = 0;

	if (lib != 0)
	{
		getScripts = (FUNC)GetProcAddress(lib, "GetScriptTypes");
		if (getScripts != 0) Script::SetTypeList((Script::List*)((*getScripts)()));

		TestApp app;
		app.Run();
	}
	if (lib != 0) FreeLibrary(lib);
	return 0;
}