#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Scripts can be attached to game objects and are the ideal way of inserting code into the scene.
//============================================================================================================

class Script
{
public:

	// Allow Object class to create scripts and access internal memebers in order to simplify code
	friend class Object;

	// Script creation function delegate
	typedef FastDelegate<Script* (void)>  CreateDelegate;

protected:

	struct Ignore
	{
		enum
		{
			PreUpdate		= 1 << 1,
			Update			= 1 << 2,
			PostUpdate		= 1 << 3,
			Fill			= 1 << 4,
		};
	};

	Object*		mObject;
	Flags		mIgnore;
	bool		mEnabled;

protected:

	// It's not possible to create just plain scripts -- they need to be derived from
	Script() : mEnabled(true) {}

	// Destroys this script, deleting it from memory and from its owner
	void DestroySelf() { delete this; }

private:

	// INTERNAL: Registers a new script of the specified type
	static void _Register(const String& type, const CreateDelegate& func);

	// INTERNAL: Creates a new script of the specified type
	static Script* _Create (const String& type);

public:

	// This is a top-level base class
	R5_DECLARE_INTERFACE_CLASS("Script");

	// Registers a new script
	template <typename Type> static void Register() { _Register( Type::ClassID(), &Type::_CreateNew ); }

	// Scripts should be removed via DestroySelf() or using the RemoveScript<> template
	virtual ~Script();

	// Initialization function is called once the script has been created
	virtual void Init() {}

	// Key event notification
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown) { return false; }

	// Mouse movement event notification
	virtual bool OnMouseMove (const Vector2i& pos, const Vector2i& delta) { return false; }

	// Mouse scroll event notification
	virtual bool OnScroll (const Vector2i& pos, float delta) { return false; }

	// Called prior to object's Update function
	virtual void OnPreUpdate() { mIgnore.Set(Ignore::PreUpdate, true); }

	// Called after the object's absolute coordinates have been calculated
	virtual void OnUpdate() { mIgnore.Set(Ignore::Update, true); }

	// Called after the object has updated all of its children
	virtual void OnPostUpdate() { mIgnore.Set(Ignore::PostUpdate, true); }

	// Called when the scene draw queue is being filled
	virtual void OnFill (FillParams& params) { mIgnore.Set(Ignore::Fill, true); }

	// Serialization
	virtual bool SerializeTo	(TreeNode& root) const { return false; }
	virtual bool SerializeFrom	(const TreeNode& root) { return false; }
};