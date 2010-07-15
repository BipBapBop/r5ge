#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Similar static model instance children of this object will be drawn in fewer draw calls
//============================================================================================================

class ModelInstanceGroup : public Octree
{
protected:

	// All meshes using the same material will be drawn into a single set of VBOs
	struct Batch
	{
		typedef Array<ModelInstance*> Instances;

		IMaterial*		mMat;		// Material used by this batch
		Instances		mInstances;	// List of all instances that will be rendered
		Memory			mVertices;	// Temporary memory used by the VBO
		Array<ushort>	mIndices;	// Temporary memory used by the IBO

		IVBO*	mVBO;
		IVBO*	mIBO;
		uint	mStride;
		uint	mIndexCount;
		uint	mPrimitive;
		uint	mNormalOffset;
		uint	mTanOffset;
		uint	mTexOffset;
		uint	mColorOffset;

		Batch() : mMat(0), mVBO(0), mIBO(0), mStride(0), mIndexCount(0), mPrimitive(0),
			mNormalOffset(0), mTanOffset(0), mTexOffset(0), mColorOffset(0) {}
	};

	// Custom data struct containing batched mesh data
	struct TerrainData : Octree::Data
	{
		PointerArray<Batch> mBatches;
		virtual ~TerrainData() {}
	};

protected:

	// List of all newly added instances that have not yet been added to batches
	Array<ModelInstance*> mNewInstances;

public:

	R5_DECLARE_INHERITED_CLASS("ModelInstanceGroup", ModelInstanceGroup, Octree, Object);

protected:

	// Helper function that retrieves the data associated with the model instance
	TerrainData* GetData (ModelInstance* inst, bool create);

	// Function called when a new child object has been added
	virtual void OnAddChild (Object* obj);

	// Function called just before the child gets removed
	virtual void OnRemoveChild (Object* obj);

	// Called when the object is being culled
	virtual bool OnFill (FillParams& params);

	// We don't want to fill models that have been marked as batched
	virtual void OnFillNode (Node& node, FillParams& params);

	// Draw the batched objects
	virtual uint OnDraw (TemporaryStorage& storage, uint group, const ITechnique* tech, void* param, bool insideOut);
};