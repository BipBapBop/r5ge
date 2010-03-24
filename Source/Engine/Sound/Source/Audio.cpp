#include "../Include/_All.h"

//============================================================================================================
// Audio Library
//============================================================================================================

#include <CAudio/Include/cAudio.h>

#ifdef _WINDOWS
  #pragma comment(lib, "cAudio.lib")
#endif

using namespace R5;

//============================================================================================================
// In order to abstract cAudio and make it invisible to the outside projects we keep it as a void*
//============================================================================================================

#define CAUDIO ((cAudio::IAudioManager*)mAudioLib)
#define SOURCE(source) ((cAudio::IAudioSource*)source)

//============================================================================================================
// Initialize the cAudio library
//============================================================================================================

Audio::Audio() : mAudioLib(0), mPos (0.0f, 0.0f, 0.0f)
{
	// cAudio has a strange habit of leaving an HTML log file by default. This disables it.
	cAudio::ILogger* logger = cAudio::getLogger();
	logger->unRegisterLogReceiver("File");

	// Initialize the cAudio library
	mAudioLib = cAudio::createAudioManager();
}

//============================================================================================================

Audio::~Audio()
{
	Release();
	CAUDIO->shutDown();
	cAudio::destroyAudioManager(CAUDIO);
}

//============================================================================================================
// Release all audio resources
//============================================================================================================

void Audio::Release()
{
	Lock();
	{
		mLayers.Release();
		mLibrary.Release();
		CAUDIO->releaseAllSources();
	}
	Unlock();
}

//============================================================================================================
// Update the different sounds every frame.
//============================================================================================================

void Audio::Update()
{
	Lock();
	{
		ulong currentTime = Time::GetMilliseconds();
		PointerArray<AudioLayer>& values = mLayers.GetAllValues();

		FOREACH(b, values)
		{
			AudioLayer* audioLayer = values[b];

			FOREACH(j, audioLayer->mSounds)
			{
				SoundInstance* sound = (SoundInstance*)audioLayer->mSounds[j];
				sound->Update(currentTime);
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Release all resources associated with the specified sound
//============================================================================================================

void Audio::Release (ISound* sound)
{
	Lock();
	{
		if (sound != 0) _Release(sound);
	}
	Unlock();
}

//============================================================================================================
// Sets the sound listener pos/dir/up (usually should be the camera)
//============================================================================================================

void Audio::SetListener (const Vector3f& position, const Vector3f& dir, const Vector3f& up)
{
	mPos = position;

	cAudio::IListener* list = CAUDIO->getListener();
	cAudio::cVector3 p (position.x, position.y, position.z);
	cAudio::cVector3 d (dir.x, dir.y, dir.z);
	cAudio::cVector3 u (up.x, up.y, up.z);

	list->setPosition (p);
	list->setDirection(d);
	list->setUpVector (u);
}

//============================================================================================================
// Set the layers volume
//============================================================================================================

void Audio::SetLayerVolume (uint layer, float volume, float duration)
{
	Lock();
	{
		AudioLayer* audioLayer = mLayers.GetIfExists(layer);
		
		// If the audio layer isn't found create one.
		if (audioLayer == 0)
		{
			AudioLayer* al = new AudioLayer(layer,volume);
			mLayers[layer] = al;
			audioLayer = mLayers.GetIfExists(layer);
		}

		audioLayer->mVolume = volume;

		PlayingSounds& sounds = audioLayer->mSounds;
		
		// For all the playing sounds update there volume level
		if (sounds.IsValid())
		{
			for (uint b = sounds.GetSize(); b > 0; )
			{
				ISoundInstance* sound = sounds[--b];
				sound->SetVolume(sound->GetVolume(), duration);
			}
		}
	}
	Unlock();
}

//============================================================================================================
// Get the volume of a layer if the layer exists
//============================================================================================================

const float Audio::GetLayerVolume (uint layer) const
{
	float volume (1.0f);
	AudioLayer* audioLayer = mLayers.GetIfExists(layer);
	if (audioLayer != 0) volume = audioLayer->mVolume;
	return volume;
}

//============================================================================================================
// Creates a AudioSource
//============================================================================================================

R5::ISound* Audio::GetSound (const String& name, bool createIfMissing)
{
	SoundPtr sound = mLibrary.Find(name);
	if (sound == 0 && createIfMissing)
	{
		Lock();
		{
			sound = mLibrary.AddUnique(name);
			sound->SetAudio(this);
		}
		Unlock();
		
	}
	return sound;
}

//============================================================================================================
// Releases the cAudio Sound source.
//============================================================================================================

void Audio::ReleaseInstance(ISoundInstance* sound)
{
	Lock();
	{
		SoundInstance* inst = (SoundInstance*)sound;
		CAUDIO->release(SOURCE(inst->mAudioSource));
		AudioLayer* audioLayer = _GetAudioLayer(inst->mLayer, 1.0f);
		audioLayer->mSounds.Remove(sound);
		delete sound;
	}
	Unlock();
}

//============================================================================================================
// Returns a newly created cAudio sound source. Needs to return void* as cAudio is not defined outside this
// class and the SoundInstance class
//============================================================================================================

ISoundInstance* Audio::Instantiate (ISound* sound, uint layer, float fadeInTime, bool repeat, Memory& data)
{
	// Load the data
	String name = sound->GetName();
	SoundInstance* soundInst = new SoundInstance();
	soundInst->mSound = sound;
	soundInst->mAudioSource = CreateAudioSource(data, name);
	soundInst->mLayer = layer;

	Lock();
	{
		AudioLayer* audioLayer = _GetAudioLayer(layer, 1.0f);
		PlayingSounds& playing = audioLayer->mSounds;
		soundInst->SetVolume(1.0f, fadeInTime);
		soundInst->SetRepeat(repeat);
		playing.Expand() = soundInst;
	}
	Unlock();
	return soundInst;
}

//============================================================================================================
// Create a new cAudioSource. Returns a void* to hide cAudio
//============================================================================================================

void* Audio::CreateAudioSource (const Memory& data, const String& name)
{
	return CAUDIO->createFromMemory(name, (const char*)data.GetBuffer(), data.GetSize(), 
							System::GetExtensionFromFilename(name).GetBuffer());
}

//============================================================================================================
// Release a cAudioSource. Accepts a void* to hid cAudio
//============================================================================================================

void Audio::ReleaseAudioSource (void* audioSource)
{
	CAUDIO->release(SOURCE(audioSource));
}

//============================================================================================================
// INTERNAL: Release the specified sound
//============================================================================================================

void Audio::_Release (ISound* sound)
{
	if (sound != 0)
	{
		mLibrary.Delete(R5_CAST(Sound, sound));
	}
}

//============================================================================================================
// INTERNAL: Retrieves the specified audio layer
//============================================================================================================

Audio::AudioLayer* Audio::_GetAudioLayer (uint layer, float volume)
{
	AudioLayerPtr& ptr = mLayers[layer];
	if (ptr == 0) ptr = new AudioLayer(layer, volume);
	return ptr;
}