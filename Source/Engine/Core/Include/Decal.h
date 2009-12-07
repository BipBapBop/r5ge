#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Decal (projected texture object)
//============================================================================================================

class Decal : public Object
{
protected:

	Matrix43	mMatrix;
	IShader*	mShader;
	Color4ub	mColor;

	Array<const ITexture*> mTextures;

	Decal();

public:

	// Object creation
	R5_DECLARE_INHERITED_CLASS("Decal", Decal, Object, Object);

	// Changes the default drawing layer that will be used by decals
	static void SetDefaultLayer(byte layer);

	const IShader*	GetShader()	const { return mShader;	}
	const Color4ub&	GetColor()	const { return mColor;	}

	void SetShader	 (IShader* shader);
	void SetColor	 (const Color4ub& val) { mColor = val; }

	Array<const ITexture*>& GetTextureArray() { return mTextures; }

protected:

	// Updates the transformation matrix
	virtual void OnUpdate();

	// Fill the renderable object and visible light lists
	virtual bool OnFill (FillParams& params);

	// Draws the light on-screen if it's visible
	virtual uint OnDraw (const ITechnique* tech, bool insideOut);

	// Serialization
	virtual void OnSerializeTo	  (TreeNode& root) const;
	virtual bool OnSerializeFrom  (const TreeNode& root);
};