#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Most basic game object -- can be positioned somewhere in the scene and can contain children.
//============================================================================================================

class Object
{
public:

	// Script creation delegate type
	typedef FastDelegate<Script* (void)>	CreateDelegate;

	struct Flag
	{
		enum
		{
			Enabled		= 1 << 1,
			Visible		= 1 << 2,
			Selectable	= 1 << 3,
		};
	};

	// Renderable object entry -- used to gather a list of renderable objects when culling the scene
	struct Renderable
	{
		Object*		mObject;		// Pointer to the object that will be rendered
		int			mLayer;			// Object's layer -- lower layers are drawn first
		const void*	mGroup;			// Groups allow grouping of similar objects in order to avoid state switches
		float		mDistSquared;	// Squared distance to the camera, used to sort objects

		// Comparison operator for sorting -- groups objects by layer, pointer, and lastly -- distance
		bool operator < (const Renderable& obj) const
		{
			if (mLayer < obj.mLayer) return true;
			if (mLayer > obj.mLayer) return false;
			if (mGroup < obj.mGroup) return true;
			if (mGroup > obj.mGroup) return false;
			return (mDistSquared < obj.mDistSquared);
		}
	};

	// Types used by this class
	typedef Object*					ObjectPtr;
	typedef Array<Renderable>		Renderables;
	typedef PointerArray<Object>	Children;
	typedef PointerArray<Script>	Scripts;
	typedef Thread::Lockable		Lockable;

protected:

	// Culling parameters passed from one object to the next during the culling stage
	struct CullParams
	{
		const Frustum&	 mFrustum;		// Frustum used to cull the scene
		const Vector3f&	 mCamPos;		// Current camera position, used to sort objects
		const Vector3f&	 mCamDir;		// Current camera direction

		Renderables&	 mRenderables;	// Sorted list of visible objects
		ILight::List&	 mLights;		// List of visible lights

		CullParams (const Frustum& f, const Vector3f& pos, const Vector3f& dir, Renderables& o, ILight::List& l)
			: mFrustum(f), mCamPos(pos), mCamDir(dir), mRenderables(o), mLights(l) {}
	};

protected:

	String		mName;			// Name of this object
	Flags		mFlags;			// Internal flags associated with this object
	Object*		mParent;		// Object's parent
	Core*		mCore;			// Engine's core that the object was created with
	Vector3f	mRelativePos;	// Local space position
	Quaternion	mRelativeRot;	// Local space rotation
	float		mRelativeScale;	// Local space scale
	Vector3f	mAbsolutePos;	// World space position, calculated every Update()
	Quaternion	mAbsoluteRot;	// World space rotation
	float		mAbsoluteScale;	// World space scale
	bool		mIsDirty;		// Whether the object's absolute coordinates should be recalculated
	bool		mSerializable;	// Whether the object will be serialized out

	Lockable	mLock;
	Children	mChildren;
	Scripts		mScripts;

private:

	// In order to eliminate virtual function calls, all virtual functions in this class
	// automatically set these flags, thus marking themselves as not being overwritten,
	// which in turn will make sure that they are never called again.

	struct Ignore
	{
		enum
		{
			Cull			= 1 << 1,
			PreUpdate		= 1 << 2,
			Update			= 1 << 3,
			PostUpdate		= 1 << 4,
			Select			= 1 << 5,
			SerializeFrom	= 1 << 6,
			SerializeTo		= 1 << 7,
		};
	};

	mutable Flags mIgnore;

protected:

	// Objects should never be created manually. Use AddObject<> template instead.
	Object();

public:

	virtual ~Object() { Release(); }

	// This is a top-level base class
	R5_DECLARE_BASE_CLASS("Object", Object);

	// Registers a new script type that can be created by the object -- shared across all objects
	static void _RegisterScript (const String& type, const CreateDelegate& callback);

private:

	friend class Scene;
	friend class Script;

	// Internal functions
	void _Add			(Object* ptr);
	void _Remove		(Object* ptr);
	void _SetParent		(Object* ptr)	 { mParent = ptr; }
	void _SetCore		(Core*	 ptr)	 { mCore   = ptr; }

public:

	// These functions are meant to be accessed only through templates such as AddObject<> and FindObject<>
	Object*	_AddObject	(const String& type, const String& name);
    Object*	_FindObject	(const String& name, bool recursive = true);
	Script* _GetScript	(const char* type);
	Script* _AddScript	(const char* type);

private:

	// Adds a new script of specified type -- should only be used internally
	Script* _AddScript (const String& type);

public:

	// Release all resources associated with this object
	void Release();

	// Thread-safe locking functionality
	void Lock()		const	{ mLock.Lock(); }
	void Unlock()	const	{ mLock.Unlock(); }

public:

	// Name and flag field retrieval
	const String&		GetName()				const	{ return mName;				}
	const Flags&		GetFlags()				const	{ return mFlags;			}
	bool				GetFlag (uint flag)		const	{ return mFlags.Get(flag);	}
	bool				IsSerializable()		const	{ return mSerializable;		}

	// Retrieves relative (local space) position / rotation
	const Vector3f&		GetRelativePosition()	const	{ return mRelativePos;		}
	const Quaternion&	GetRelativeRotation()	const	{ return mRelativeRot;		}
	float				GetRelativeScale()		const	{ return mRelativeScale;	}

	// Retrieves absolute (world space) position / rotation
	const Vector3f&		GetAbsolutePosition()	const	{ return mAbsolutePos;		}
	const Quaternion&	GetAbsoluteRotation()	const	{ return mAbsoluteRot;		}
	float				GetAbsoluteScale()		const	{ return mAbsoluteScale;	}

public:

	// Every object has a name that can be changed
	void SetName (const String& name) { mName = name; }

	// Every object can have object-specific flags that can be changed
	void SetFlag (uint flag, bool val) { mFlags.Set(flag, val); }

	// Marks the object as dirty, making it recalculate its absolute position next update
	void SetDirty() { mIsDirty = true; }

	// It's possible to switch object's parents
	void SetParent (Object* ptr);

	// Whether the object will be saved out
	void SetSerializable (bool val) { mSerializable = val; }

	// Sets all relative values
	void SetRelativePosition ( const Vector3f& pos )	{ mRelativePos	 = pos;		mIsDirty = true; }
	void SetRelativeRotation ( const Quaternion& rot )	{ mRelativeRot	 = rot;		mIsDirty = true; }
	void SetRelativeScale	 ( float scale )			{ mRelativeScale = scale;	mIsDirty = true; }

	// Sets both relative and absolute values using provided absolute values
	void SetAbsolutePosition ( const Vector3f& pos );
	void SetAbsoluteRotation ( const Quaternion& rot );
	void SetAbsoluteScale	 ( float scale );

	// Selects the object closest to this position
	Object*	Select (const Vector3f& pos);

protected:

	// Updates the object, calling appropriate virtual functions
	void _Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved);

	// Culls the object based on the viewing frustum.
	// The last parameter specifies whether to actually place this object into the rendering queue.
	void _Cull (CullParams& params, bool isParentVisible, bool render);

	// Recursive Object::OnSelect caller
	void _Select (const Vector3f& pos, ObjectPtr& ptr, float& radius);

public:

	// Serialization
	bool SerializeTo (TreeNode& root) const;
	bool SerializeFrom (const TreeNode& root, bool forceUpdate = false);

protected:

	// Called prior to object's Update function
	virtual void OnPreUpdate();

	// Called after the object's absolute coordinates have been calculated
	virtual void OnUpdate();

	// Called after the object has updated all of its children
	virtual void OnPostUpdate();

	// Called when the object is being culled -- should return whether the object is visible
	virtual bool OnCull (CullParams& params, bool isParentVisible, bool render);

	// Render the object using the specified technique. This function will only be
	// called if this object has been added to the list of renderable objects in
	// OnCull. It should return the number of triangles rendered.
	virtual uint OnRender (IGraphics* graphics, const ITechnique* tech, bool insideOut) { return 0; }

	// Called when the object is being selected -- may update the referenced values
	virtual void OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius);

	// Called when the object is being saved
	virtual void OnSerializeTo (TreeNode& root) const;

	// Called when the object is being loaded
	virtual bool OnSerializeFrom (const TreeNode& root);
};