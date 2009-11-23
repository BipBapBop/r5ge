#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// All registered script types that can be created are stored here
//============================================================================================================

Hash<Object::CreateDelegate> gScriptTypes;

//============================================================================================================
// Each object is enabled by default
//============================================================================================================

Object::Object() :
	mParent			(0),
	mCore			(0),
	mRelativeScale	(1.0f),
	mAbsoluteScale	(1.0f),
	mIsDirty		(false),
	mSerializable	(true)
{
	mFlags.Set(Flag::Enabled);
}

//============================================================================================================
// INTERNAL: Registers a new script type that can be created by the object -- shared across all objects
//============================================================================================================

void Object::_RegisterScript (const String& type, const CreateDelegate& callback)
{
	gScriptTypes.Lock();
	gScriptTypes[type] = callback;
	gScriptTypes.Unlock();
}

//============================================================================================================
// INTERNAL: Adds a child object
//============================================================================================================

void Object::_Add (Object* obj)
{
	if (obj != 0)
	{
		obj->mAbsolutePos	= obj->mRelativePos;
		obj->mAbsoluteRot	= obj->mRelativeRot;
		obj->mAbsoluteScale	= obj->mRelativeScale;

		obj->mRelativePos	= Interpolation::GetDifference(mAbsolutePos, obj->mAbsolutePos) / mAbsoluteScale;
		obj->mRelativeRot	= Interpolation::GetDifference(mAbsoluteRot, obj->mAbsoluteRot);
		obj->mRelativeScale	= obj->mAbsoluteScale / mAbsoluteScale;

		mChildren.Expand() = obj;
	}
}

//============================================================================================================
// INTERNAL: Removes a child object
//============================================================================================================

void Object::_Remove (Object* obj)
{
	if (obj != 0)
	{
		mChildren.Remove(obj);

		obj->mRelativePos	= obj->mAbsolutePos;
		obj->mRelativeRot	= obj->mAbsoluteRot;
		obj->mRelativeScale	= obj->mAbsoluteScale;
	}
}

//============================================================================================================
// INTERNAL: Adds a child object of specified type (or returns an existing one)
//============================================================================================================

Object* Object::_AddObject (const String& type, const String& name)
{
	Object* ptr (0);

	for (uint i = mChildren.GetSize(); i > 0; )
	{
		Object* obj = mChildren[--i];

		if ( obj != 0 && name == obj->GetName() )
		{
			ptr = obj;
			break;
		}
	}

	if (ptr == 0)
	{
		ptr = mCore->GetScene()->_CreateNode(type);

		if (ptr != 0)
		{
			ptr->SetName(name);
			ptr->_SetParent(this);
			mChildren.Expand() = ptr;
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Finds an object in the scene
//============================================================================================================

Object* Object::_FindObject (const String& name, bool recursive)
{
	Object* node (0);

	if (mChildren.IsValid())
	{
		for (uint i = mChildren.GetSize(); i > 0; )
		{
			if ( name == mChildren[--i]->GetName() )
			{
				node = mChildren[i];
				break;
			}
		}

		if ( node == 0 && recursive )
		{
			for (uint i = 0; i < mChildren.GetSize(); ++i)
				if ( node = mChildren[i]->_FindObject(name, recursive) )
					break;
		}
	}
	return node;
}

//============================================================================================================
// INTERNAL: Retrieves a script of specified type
//============================================================================================================

Script* Object::_GetScript (const char* type)
{
	Script* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		Script* script = mScripts[i];

		if ( script != 0 && type == script->GetClassID() )
		{
			ptr = script;
			break;
		}
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Adds a script of specified type (or returns an existing one)
//============================================================================================================

Script* Object::_AddScript (const char* type)
{
	Script* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		Script* script = mScripts[i];

		if ( script != 0 && type == script->GetClassID() )
		{
			ptr = script;
			break;
		}
	}

	if (ptr == 0)
	{
		gScriptTypes.Lock();
		{
			const CreateDelegate* callback = gScriptTypes.GetIfExists(type);
			
			if (callback != 0)
			{
				ptr = (*callback)();
				ptr->mObject = this;
				mScripts.Expand() = ptr;
			}
		}
		gScriptTypes.Unlock();
	}
	return ptr;
}

//============================================================================================================
// INTERNAL: Adds a script of specified type (or returns an existing one). While it's visually identical
// to the function above, the type differs, which in turn changes the code's behavior.
//============================================================================================================

Script* Object::_AddScript (const String& type)
{
	Script* ptr (0);

	for (uint i = mScripts.GetSize(); i > 0; )
	{
		Script* script = mScripts[i];

		if ( script != 0 && type == script->GetClassID() )
		{
			ptr = script;
			break;
		}
	}

	if (ptr == 0)
	{
		gScriptTypes.Lock();
		{
			const CreateDelegate* callback = gScriptTypes.GetIfExists(type);
			
			if (callback != 0)
			{
				ptr = (*callback)();
				ptr->mObject = this;
				mScripts.Expand() = ptr;
			}
		}
		gScriptTypes.Unlock();
	}
	return ptr;
}

//============================================================================================================
// Releases the object and all its children
//============================================================================================================

void Object::Release()
{
	Lock();
	{
		if (mParent != 0)
		{
			mParent->_Remove(this);
			mParent = 0;
		}

		if (mScripts.IsValid())
		{
			// Run through all scripts
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				// Remove the reference to this object
				if (mScripts[--i] != 0)
				{
					mScripts[i]->mObject = 0;
				}
			}
			
			// Release all scripts
			mScripts.Release();
		}

		if (mChildren.IsValid())
		{
			// Run through all children
			for (uint i = mChildren.GetSize(); i > 0; )
			{
				// Remove all references to this object
				if (mChildren[--i] != 0)
				{
					mChildren[i]->_SetParent(0);
				}
			}

			// Release all children
			mChildren.Release();
		}
	}
	Unlock();
}

//============================================================================================================
// Changes the parent object
//============================================================================================================

void Object::SetParent (Object* ptr)
{
	if (mParent != ptr)
    {
        Lock();
		{
			if (mParent) mParent->_Remove(this);
			mParent = ptr;
			if (mParent) mParent->_Add(this);
		}
        Unlock();
    }
}

//============================================================================================================
// Sets the position using an absolute value
//============================================================================================================

void Object::SetAbsolutePosition (const Vector3f& pos)
{
	mIsDirty = true;
	mAbsolutePos = pos;

	if (mParent != 0)
	{
		float invScale = 1.0f / mParent->GetAbsoluteScale();
		mRelativePos = Interpolation::GetDifference(mParent->GetAbsolutePosition(), mAbsolutePos) * invScale;
	}
	else mRelativePos = mAbsolutePos;
}

//============================================================================================================
// Sets the rotation using an absolute value
//============================================================================================================

void Object::SetAbsoluteRotation (const Quaternion& rot)
{
	mIsDirty = true;
	mAbsoluteRot = rot;

	if (mParent != 0)
	{
		mRelativeRot = Interpolation::GetDifference(mParent->GetAbsoluteRotation(), mAbsoluteRot);
	}
	else mRelativeRot = mAbsoluteRot;
}

//============================================================================================================
// Sets the position using an absolute value
//============================================================================================================

void Object::SetAbsoluteScale (float scale)
{
	mIsDirty = true;
	mAbsoluteScale = scale;

	if (mParent != 0)
	{
		mRelativeScale = mAbsoluteScale / mParent->GetAbsoluteScale();
	}
	else mRelativeScale = mAbsoluteScale;
}

//============================================================================================================
// Returns the object closest to the given position
//============================================================================================================

Object* Object::Select (const Vector3f& pos)
{
	float radius = 65535.0f;
	Object* ptr = 0;
	_Select(pos, ptr, radius);
	return ptr;
}

//============================================================================================================
// INTERNAL: Updates the object, calling appropriate virtual functions
//============================================================================================================

void Object::_Update (const Vector3f& pos, const Quaternion& rot, float scale, bool parentMoved)
{
	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			// If the parent has moved then we need to recalculate the absolute values
			if (parentMoved) mIsDirty = true;

			// Call the pre-update function
			if (!mIgnore.Get(Ignore::PreUpdate)) OnPreUpdate();

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::PreUpdate))
				{
					script->OnPreUpdate();
				}
			}

			// If something has changed, update the absolute values
			if (mIsDirty)
			{
				mAbsolutePos = (mRelativePos * scale) * rot + pos;
				mAbsoluteScale = mRelativeScale * scale;
				mAbsoluteRot.Combine(rot, mRelativeRot);
			}

			// Call the update function
			if (!mIgnore.Get(Ignore::Update)) OnUpdate();

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::Update))
				{
					script->OnUpdate();
				}
			}

			// Update all children
			if (mChildren.IsValid())
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
				{
					Object* obj = mChildren[i];

					if (obj != 0 && obj->GetFlag(Flag::Enabled))
					{
						mChildren[i]->_Update(mAbsolutePos, mAbsoluteRot, mAbsoluteScale, mIsDirty);
					}
				}
			}

			// Call the post-update function
			if (!mIgnore.Get(Ignore::PostUpdate)) OnPostUpdate();

			// Run through all scripts and notify them
			for (uint i = mScripts.GetSize(); i > 0; )
			{
				Script* script = mScripts[--i];

				// If the script is listening to pre-update events
				if (!script->mIgnore.Get(Script::Ignore::PostUpdate))
				{
					script->OnPostUpdate();
				}
			}

			// The absolute values have now been set
			mIsDirty = false;
		}
		Unlock();
	}
}

//============================================================================================================
// INTERNAL: Culls the object based on the viewing frustum
//============================================================================================================

void Object::_Cull (CullParams& params, bool isParentVisible, bool render)
{
	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			if (!mIgnore.Get(Ignore::Cull))
			{
				// Note that by default parent's visibility doesn't affect children, and OnCull will be
				// called regardless of whether the parent was visible or not. This was done to ensure that
				// such cases like an in-game character carrying a light source would work properly. An obvious
				// optimization would be to never cull children if this object is not visible. This optimization
				// should be used where the object will never move past its parent's bounds (ex.: terrain).

				isParentVisible = OnCull(params, isParentVisible, render);
				mFlags.Set(Flag::Visible, isParentVisible);
			}

			// Inform all children that they're being culled and let them decide themselves whether they
			// should do anything about it or not depending on whether this object is visible.

			if (mChildren.IsValid())
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
					mChildren[i]->_Cull(params, isParentVisible, render);
			}
		}
		Unlock();
	}
}

//============================================================================================================
// INTERNAL: Recursive Object::OnSelect caller
//============================================================================================================

void Object::_Select (const Vector3f& pos, ObjectPtr& ptr, float& radius)
{
	if (mFlags.Get(Flag::Enabled))
	{
		Lock();
		{
			if (mFlags.Get(Flag::Visible) && mFlags.Get(Flag::Selectable) && !mIgnore.Get(Ignore::Select))
			{
				OnSelect(pos, ptr, radius);
			}

			if (mChildren.IsValid())
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
				{
					mChildren[i]->_Select(pos, ptr, radius);
				}
			}
		}
		Unlock();
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

bool Object::SerializeTo (TreeNode& root) const
{
	if (mSerializable)
	{
		Lock();
		{
			TreeNode& node = root.AddChild( GetClassID(), mName );
			node.AddChild("Position", mRelativePos);
			node.AddChild("Rotation", mRelativeRot);
			node.AddChild("Scale", mRelativeScale);

			if (!mIgnore.Get(Ignore::SerializeTo))
			{
				OnSerializeTo(node);
			}

			if (mScripts.IsValid())
			{
				for (uint i = 0; i < mScripts.GetSize(); ++i)
				{
					const Script* script = mScripts[i];
					TreeNode& child = node.AddChild(Script::ClassID(), script->GetClassID());
					script->SerializeTo(child);
				}
			}

			if (mChildren.IsValid())
			{
				for (uint i = 0; i < mChildren.GetSize(); ++i)
					mChildren[i]->SerializeTo(node);
			}
		}
		Unlock();
		return true;
	}
	return false;
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

bool Object::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	Lock();
	{
		for (uint i = 0; i < root.mChildren.GetSize(); ++i)
		{
			const TreeNode& node  = root.mChildren[i];
			const String&	tag   = node.mTag;
			const Variable&	value = node.mValue;

			if		(tag == "Serializable")		  value >> mSerializable;
			else if	(tag == "Position")		{ if (value >> mRelativePos)	mIsDirty = true; }
			else if (tag == "Rotation")		{ if (value >> mRelativeRot)	mIsDirty = true; }
			else if (tag == "Scale")		{ if (value >> mRelativeScale)	mIsDirty = true; }
			else if (tag == Script::ClassID())
			{
				Script* ptr = _AddScript(value.IsString() ? value.AsString() : value.GetString());
				if (ptr != 0) ptr->SerializeFrom(node);
			}
			else if (mIgnore.Get(Ignore::SerializeFrom) || !OnSerializeFrom(node))
			{
				Object* ptr = _AddObject(tag, value.IsString() ? value.AsString() : value.GetString());
				if (ptr != 0) ptr->SerializeFrom(node, forceUpdate);
			}
		}
	}
	Unlock();
	return true;
}

//============================================================================================================
// Called prior to object's Update function
//============================================================================================================

void Object::OnPreUpdate()
{
	mIgnore.Set(Ignore::PreUpdate, true);
}

//============================================================================================================
// Called after the object's absolute coordinates have been calculated
//============================================================================================================

void Object::OnUpdate()
{
	mIgnore.Set(Ignore::Update, true);
}

//============================================================================================================
// Called after the object has updated all of its children
//============================================================================================================

void Object::OnPostUpdate()
{
	mIgnore.Set(Ignore::PostUpdate, true);
}

//============================================================================================================
// Called when the object is being culled
//============================================================================================================

bool Object::OnCull (CullParams& params, bool isParentVisible, bool render)
{
	mIgnore.Set(Ignore::Cull, true);
	return false;
}

//============================================================================================================
// Called when the object is being selected -- may update the referenced values
//============================================================================================================

void Object::OnSelect (const Vector3f& pos, ObjectPtr& ptr, float& radius)
{
	mIgnore.Set(Ignore::Select, true);
}

//============================================================================================================
// Called when the object is being loaded
//============================================================================================================

bool Object::OnSerializeFrom (const TreeNode& root)
{
	mIgnore.Set(Ignore::SerializeFrom, true);
	return false;
}

//============================================================================================================
// Called when the object is being saved
//============================================================================================================

void Object::OnSerializeTo (TreeNode& root) const
{
	mIgnore.Set(Ignore::SerializeTo, true);
}