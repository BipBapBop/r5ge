#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================

#ifndef _GL_GRAPHICS_INCLUDE_H
#define _GL_GRAPHICS_INCLUDE_H

#include "../../Image/Include/_All.h"
#include "../../Font/Include/_All.h"
#include "../../SysWindow/Include/_All.h"

namespace R5
{
	#include "GLTransform.h"	// Model, view and projection matrices bundled into one class
	#include "GLVBO.h"			// OpenGL Vertex Buffer Object
	#include "GLFBO.h"			// Frame Buffer Object
	#include "GLTexture.h"		// Regular 2D texture
	#include "GLFont.h"			// OpenGL implementation of the font class
	#include "GLSubShader.h"	// Single vertex or fragment shader, part of a GLSL program
	#include "GLShader.h"		// GLSL shader program using one or more SubShaders
	#include "GLTechnique.h"	// Material rendering technique
	#include "GLMaterial.h"		// Material management
	#include "GLController.h"	// Low-level graphics controller -- closest level of interaction with the renderer API
	#include "GLGraphics.h"		// Higher level of renderer API interaction, handles resource management for graphical resources
	#include "GLWindow.h"		// OpenGL window creation
};

#endif