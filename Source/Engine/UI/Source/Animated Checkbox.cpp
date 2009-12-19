#include "../Include/_All.h"
using namespace R5;

//============================================================================================================

UIAnimatedCheckbox::UIAnimatedCheckbox() : mAnimTime(0.15f)
{
	memset(mCurrentAlpha,	0, sizeof(float)*2);
	memset(mTargetAlpha,	0, sizeof(float)*2);
	memset(mStartTime,		0, sizeof(float)*2);
}

//============================================================================================================
// Changes the checkbox state
//============================================================================================================

bool UIAnimatedCheckbox::SetState(uint state, bool val)
{
	if ( UICheckbox::SetState(state, val) )
	{
		float current = mRoot->GetCurrentTime();

		// Clear all targets
		for (uint i = 0; i < 2; ++i)
		{
			mStartAlpha [i] = mCurrentAlpha[i];
			mTargetAlpha[i] = 0.0f;
			mStartTime  [i] = current;
		}

		// Set the appropriate target as necessary
		if (mState & State::Highlighted)	mTargetAlpha[0] = 1.0f;
		if (mState & State::Checked)		mTargetAlpha[1] = 1.0f;
		return true;
	}
	return false;
}

//============================================================================================================
// Any per-frame animation should go here
//============================================================================================================

bool UIAnimatedCheckbox::OnUpdate (bool dimensionsChanged)
{
	dimensionsChanged |= UICheckbox::OnUpdate(dimensionsChanged);

	// If the button is currently in the process of animation, mark the SubPicture as dirty
	for (uint i = 0; i < 2; ++i)
		if (mCurrentAlpha[i] != mTargetAlpha[i])
			return true;

	return dimensionsChanged;
}

//============================================================================================================
// Called when a queue is being rebuilt
//============================================================================================================

void UIAnimatedCheckbox::OnFill (UIQueue* queue)
{
	if (queue->mLayer == mLayer && queue->mArea == 0 && queue->mTex == mImage.GetTexture())
	{
		static String faceName[] =
		{
			"Checkbox: Disabled",
			"Checkbox: Unchecked",
			"Checkbox: Highlighted",
			"Checkbox: Checked"
		};

		if (mState & State::Enabled)
		{
			// Fill the normal face
			mImage.SetFace(faceName[1], false);
			mImage.GetRegion().SetAlpha(1.0f);
			mImage.GetRegion().Update(mRegion);
			mImage.OnFill(queue);

			// Get the current time for animation
			float time = mRoot->GetCurrentTime();

			for (uint i = 0; i < 2; ++i)
			{
				// If alpha is changing, update it
				if ( Float::IsNotEqual(mCurrentAlpha[i], mTargetAlpha[i]) )
				{
					float factor = (mAnimTime > 0.0f) ? Float::Clamp((time - mStartTime[i]) / mAnimTime, 0.0f, 1.0f) : 1.0f;
					mCurrentAlpha[i] = mStartAlpha[i] * (1.0f - factor) + mTargetAlpha[i] * factor;
				}

				// If the secondary face is visible, draw it
				if (mCurrentAlpha[i] > 0.0f)
				{
					mImage.SetFace(faceName[i+2], false);
					mImage.GetRegion().SetAlpha(mCurrentAlpha[i]);
					mImage.GetRegion().Update(mRegion);
					mImage.OnFill(queue);
				}
			}
		}
		else
		{
			mImage.SetFace(faceName[0], false);
			mImage.GetRegion().SetAlpha(1.0f);
			mImage.GetRegion().Update(mRegion);
			mImage.OnFill(queue);
		}
	}
	else mLabel.OnFill(queue);
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool UIAnimatedCheckbox::OnSerializeFrom (const TreeNode& node)
{
	if (node.mTag == "Animation Time")
	{
		node.mValue >> mAnimTime;
		return true;
	}
	return UICheckbox::OnSerializeFrom(node);
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

void UIAnimatedCheckbox::OnSerializeTo (TreeNode& node) const
{
	UICheckbox::OnSerializeTo(node);
	node.AddChild("Animation Time", mAnimTime);
}