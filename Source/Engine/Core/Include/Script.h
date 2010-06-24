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

	// Allow Object class to create scripts and access internal members in order to simplify code
	friend class Object;
	friend class Core;

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
	bool		mSerializable;

private:

	// INTERNAL: Registers a new script of the specified type
	static void _Register(const String& type, const CreateDelegate& func);

	// INTERNAL: Creates a new script of the specified type
	static Script* _Create (const String& type);

protected:

	// Use the AddScript<> template to add new scripts
	Script() : mEnabled(true), mSerializable(true) {}

	// Convenience function meant to be used from inside the protected functions
	void DestroySelf() { DestroySelf(false); }

public:

	// This is a top-level base class
	R5_DECLARE_INTERFACE_CLASS("Script");

	// Registers a new script
	template <typename Type> static void Register() { _Register( Type::ClassID(), &Type::_CreateNew ); }

	// Scripts should be removed via DestroySelf() or using the RemoveScript<> template
	virtual ~Script();

	// Access to the owner of the script
	Object* GetOwner() { return mObject; }
	const Object* GetOwner() const { return mObject; }

	// Convenience function: mObject->GetCore()->IsKeyDown(key);
	bool IsKeyDown (uint key);

	// Destroys this script - this action is queued until next update
	void DestroySelf (bool threadSafe);

	// It's possible to choose not to serialize certain scripts
	bool IsSerializable() const { return mSerializable; }
	void SetSerializable (bool val) { mSerializable = val; }

	// Serialization
	void SerializeTo (TreeNode& root) const;
	void SerializeFrom (const TreeNode& root);

protected:

	// Initialization function is called once the script has been created
	virtual void OnInit() {}

	// Callback triggered when DestroySelf() gets called
	virtual void OnDestroy() {}

	// Called prior to object's Update function
	virtual void OnPreUpdate() { mIgnore.Set(Ignore::PreUpdate, true); }

	// Called after the object's absolute coordinates have been calculated
	virtual void OnUpdate() { mIgnore.Set(Ignore::Update, true); }

	// Called after the object has updated all of its children
	virtual void OnPostUpdate() { mIgnore.Set(Ignore::PostUpdate, true); }

	// Called when the scene draw queue is being filled
	virtual void OnFill (FillParams& params) { mIgnore.Set(Ignore::Fill, true); }

	// Key and mouse events
	virtual bool OnKeyPress (const Vector2i& pos, byte key, bool isDown) { return false; }
	virtual bool OnMouseMove(const Vector2i& pos, const Vector2i& delta) { return false; }
	virtual bool OnScroll	(const Vector2i& pos, float delta)			 { return false; }

	// Serialization
	virtual void OnSerializeTo	(TreeNode& node) const {}
	virtual void OnSerializeFrom(const TreeNode& node) {}
};