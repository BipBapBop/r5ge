#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Development Testing Application
//============================================================================================================

#include "../../../Engine/OpenGL/Include/_All.h"
#include "../../../Engine/Core/Include/_All.h"
#include "../../../Engine/UI/Include/_All.h"

namespace R5
{
class TestApp
{
	IWindow*	mWin;
	IGraphics*	mGraphics;
	UI*			mUI;
	Core*		mCore;

public:

	TestApp();
	~TestApp();

	void Init();
	void Run();
};
}