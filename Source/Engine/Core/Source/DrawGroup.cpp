#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Draw all objects in the group
//============================================================================================================

uint DrawGroup::Draw (const ITechnique* tech, bool insideOut)
{
	uint retVal = 0;

	if (tech->GetSorting() == ITechnique::Sorting::BackToFront)
	{
		for (uint i = mEntries.GetSize(); i > 0; )
		{
			retVal += mEntries[--i].mObject->OnDraw(tech, insideOut);
		}
	}
	else
	{
		for (uint i = 0, bmax = mEntries.GetSize(); i < bmax; ++i)
		{
			retVal += mEntries[i].mObject->OnDraw(tech, insideOut);
		}
	}
	return retVal;
}