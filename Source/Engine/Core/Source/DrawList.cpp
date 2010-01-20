#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Add a new entry
//============================================================================================================

void DrawList::Add (const void* group, Object* object, float distance)
{
	typedef DrawGroup* DrawGroupPtr;
	DrawGroupPtr& ptr = mGroups[(uint)group];
	if (ptr == 0) ptr = new DrawGroup();
	ptr->Add(object, distance);
}

//============================================================================================================
// Clear all entries in the draw list
//============================================================================================================

void DrawList::Clear()
{
	PointerArray<DrawGroup>& groups = mGroups.GetAllValues();

	for (uint i = groups.GetSize(); i > 0; )
	{
		groups[--i]->Clear();
	}
}

//============================================================================================================
// Draw all objects in the list
//============================================================================================================

uint DrawList::Draw (const ITechnique* tech, bool insideOut)
{
	PointerArray<DrawGroup>& groups = mGroups.GetAllValues();

	bool sort = false;

	// If the list needs to be sorted and the technique requires sorting, mark it as such
	if (mNeedsSorting && (tech->GetSorting() != ITechnique::Sorting::None))
	{
		sort = true;
		mNeedsSorting = false;
	}

	uint retVal = 0;

	// Back-to-front so that group 0 is drawn last, as that's where all groupless objects end up,
	// including objects that are purposely groupless in order for them to be blended correctly.

	for (uint i = groups.GetSize(); i > 0; )
	{
		DrawGroup* group = groups[--i];

		// Sort the group if necessary
		if (sort) group->Sort();

		// Draw the group
		retVal = group->Draw(tech, insideOut);
	}
	return retVal;
}