#include "engine-precompiled-header.h"
#include "AudioManager.h"
#include "engine/Engine.h"
#include "engine/core/exception/EngineException.h"
#include "engine/events/engineEvents/EngineCustomEvent.h"

longmarch::FMODInstance::FMODInstance()
	:
	mnNextChannelId(0)
{
	mpStudioSystem = NULL;
	AudioManager::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
	// The init below will enable TCP/IP host for profiling.
	// Since Digipen does not want offline game to use any form of networking, we will disable that.
	//AudioManager::ErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));
	AudioManager::ErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL));

	mpSystem = NULL;
	AudioManager::ErrorCheck(mpStudioSystem->getCoreSystem(&mpSystem));
}

longmarch::FMODInstance::~FMODInstance()
{
	AudioManager::ErrorCheck(mpStudioSystem->unloadAll());
	AudioManager::ErrorCheck(mpStudioSystem->release());
}

void longmarch::FMODInstance::Update()
{
	std::vector<int> pStoppedChannels;
	for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
	{
		bool bIsPlaying = false;
		it->second->isPlaying(&bIsPlaying);
		if (!bIsPlaying)
		{
			pStoppedChannels.push_back(it->first);
		}
	}
	for (auto& it : pStoppedChannels)
	{
		mChannels.erase(it);
		mSound2Channels.erase(mChannels2Sound[it]);
		mChannels2Sound.erase(it);
	}
	AudioManager::ErrorCheck(mpStudioSystem->update());
}

longmarch::AudioManager* longmarch::AudioManager::GetInstance()
{
	static AudioManager instance;
	return &instance;
}

longmarch::AudioManager::AudioManager()
{
	m_fmodInstance = std::make_unique<FMODInstance>();
	{
		Engine::GetInstance()->PostRenderUpdate().Connect(std::bind(&AudioManager::Update, this));
	}
	{
		auto queue = EventQueue<EngineEventType>::GetInstance();
		ManageEventSubHandle(queue->Subscribe<AudioManager>(this, EngineEventType::ENG_WINDOW_INTERRUTPTION, &AudioManager::_ON_PAUSE));
	}
}

void longmarch::AudioManager::Update()
{
	m_fmodInstance->Update();
}

int longmarch::AudioManager::ErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		std::cout << "FMOD ERROR " << result << std::endl;
		return 1;
	}
	// std::cout << "FMOD all good" << std::endl;
	return 0;
}

int longmarch::AudioManager::GetSoundChannel(const std::string& strSoundName)
{
	int id = -1;
	if (m_fmodInstance->mSound2Channels.find(strSoundName) != m_fmodInstance->mSound2Channels.end())
	{
		id = m_fmodInstance->mSound2Channels[strSoundName];
	}
	return id;
}

void longmarch::AudioManager::LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags)
{
	auto tFoundIt = m_fmodInstance->mBanks.find(strBankName);
	if (tFoundIt != m_fmodInstance->mBanks.end())
		return;
	FMOD::Studio::Bank* pBank;
	AudioManager::ErrorCheck(m_fmodInstance->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &pBank));
	if (pBank) {
		m_fmodInstance->mBanks[strBankName] = pBank;
	}
}

void longmarch::AudioManager::LoadEvent(const std::string& strEventName)
{
	auto tFoundit = m_fmodInstance->mEvents.find(strEventName);
	if (tFoundit != m_fmodInstance->mEvents.end())
		return;
	FMOD::Studio::EventDescription* pEventDescription = NULL;
	AudioManager::ErrorCheck(m_fmodInstance->mpStudioSystem->getEvent(strEventName.c_str(), &pEventDescription));
	if (pEventDescription) {
		FMOD::Studio::EventInstance* pEventInstance = NULL;
		AudioManager::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
		if (pEventInstance) {
			m_fmodInstance->mEvents[strEventName] = pEventInstance;
		}
	}
}

/*
	Load sound from path
*/
void longmarch::AudioManager::LoadSound(const std::string& strSoundName, const fs::path& strSoundPath, bool bLooping, bool b3d, bool bStream)
{
	auto tFoundIt = m_fmodInstance->mSounds.find(strSoundName);
	if (tFoundIt != m_fmodInstance->mSounds.end())
		return;

	FMOD_MODE eMode = FMOD_DEFAULT;
	eMode |= b3d ? FMOD_3D : FMOD_2D;
	eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD::Sound* pSound = nullptr;
	auto path = FileSystem::ResolveProtocol(strSoundPath);
	AudioManager::ErrorCheck(m_fmodInstance->mpSystem->createSound(path.string().c_str(), eMode, nullptr, &pSound));
	if (pSound) {
		// Find the name of the sound file from the input path
		// std::std::string name = strSoundPath.substr(strSoundPath.find_last_of('/')+1, strSoundPath.find_last_of('.') - strSoundPath.find_last_of('/') -1);
		UnLoadSound(strSoundName);
		m_fmodInstance->mSounds[strSoundName] = pSound;
	}
	else
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Sound at " + path.wstring() + L" has failed to create!");
	}
}

void longmarch::AudioManager::UnLoadSound(const std::string& strSoundName)
{
	auto tFoundIt = m_fmodInstance->mSounds.find(strSoundName);
	if (tFoundIt == m_fmodInstance->mSounds.end())
		return;

	AudioManager::ErrorCheck(tFoundIt->second->release());
	m_fmodInstance->mSounds.erase(tFoundIt);
}

void longmarch::AudioManager::Set3dListenerAndOrientation(const AudioVector3& vPos, float fVolumedB)
{
	auto pos = VectorToFmod(vPos);
	auto vel = VectorToFmod(AudioVector3(0));
	auto forward = VectorToFmod(AudioVector3(0, 1, 0));
	auto up = VectorToFmod(AudioVector3(0, 0, 1));
	m_fmodInstance->mpSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
}

int longmarch::AudioManager::PlaySoundByName(const std::string& strSoundName, const AudioVector3& vPos, float fVolumedB, float frequency)
{
	auto tFoundIt = m_fmodInstance->mSounds.find(strSoundName);
	if (tFoundIt == m_fmodInstance->mSounds.end())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Sound at " + wStr(strSoundName) + L" has not been loaded!");
	}
	FMOD::Channel* pChannel = nullptr;
	AudioManager::ErrorCheck(m_fmodInstance->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));
	if (pChannel)
	{
		int nChannelId = m_fmodInstance->mnNextChannelId++;
		FMOD_MODE currMode;
		tFoundIt->second->getMode(&currMode);
		if (currMode & FMOD_3D) {
			FMOD_VECTOR position = VectorToFmod(vPos);
			AudioManager::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
		}
		AudioManager::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
		AudioManager::ErrorCheck(pChannel->setPaused(false));
		AudioManager::ErrorCheck(pChannel->setPitch(frequency));
		m_fmodInstance->mSound2Channels[strSoundName] = nChannelId;
		m_fmodInstance->mChannels2Sound[nChannelId] = strSoundName;
		m_fmodInstance->mChannels[nChannelId] = pChannel;
		return nChannelId;
	}
	return -1;
}

int longmarch::AudioManager::StopSound(const std::string& strSoundName)
{
	auto tFoundIt = m_fmodInstance->mSounds.find(strSoundName);
	if (tFoundIt == m_fmodInstance->mSounds.end())
	{
		throw EngineException(_CRT_WIDE(__FILE__), __LINE__, L"Sound at " + wStr(strSoundName) + L" has not been loaded!");
	}
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			StopChannel(id);
			return id;
		}
	}
	return -1;
}

void longmarch::AudioManager::SetSoundFreqency(const std::string& strSoundName, float frequency)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			channel->setPitch(frequency);
		}
	}
}

void longmarch::AudioManager::SetSoundDB(const std::string& strSoundName, float fVolumedB)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			channel->setVolume(dbToVolume(fVolumedB));
		}
	}
}

void longmarch::AudioManager::SetSoundVol(const std::string& strSoundName, float vol)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			channel->setVolume(vol);
		}
	}
}

void longmarch::AudioManager::SetSoundPause(const std::string& strSoundName, bool pause)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			channel->setPaused(pause);
		}
	}
}

void longmarch::AudioManager::SetSoundMute(const std::string& strSoundName, bool mute)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		if (auto channel = m_fmodInstance->mChannels[id])
		{
			channel->setMute(mute);
		}
	}
}

void longmarch::AudioManager::PlayEvent(const std::string& strEventName)
{
	auto tFoundit = m_fmodInstance->mEvents.find(strEventName);
	if (tFoundit == m_fmodInstance->mEvents.end()) {
		LoadEvent(strEventName);
		tFoundit = m_fmodInstance->mEvents.find(strEventName);
		if (tFoundit == m_fmodInstance->mEvents.end())
			return;
	}
	tFoundit->second->start();
}

void longmarch::AudioManager::StopChannel(int nChannelId)
{
	auto tFoundIt = m_fmodInstance->mChannels.find(nChannelId);
	if (tFoundIt == m_fmodInstance->mChannels.end())
		return;

	AudioManager::ErrorCheck(tFoundIt->second->stop());
}

void longmarch::AudioManager::StopEvent(const std::string& strEventName, bool bImmediate)
{
	auto tFoundIt = m_fmodInstance->mEvents.find(strEventName);
	if (tFoundIt == m_fmodInstance->mEvents.end())
		return;

	FMOD_STUDIO_STOP_MODE eMode;
	eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
	AudioManager::ErrorCheck(tFoundIt->second->stop(eMode));
}

void longmarch::AudioManager::StopAllChannels()
{
	for (auto iter = m_fmodInstance->mChannels.begin(); iter != m_fmodInstance->mChannels.end(); ++iter)
	{
		AudioManager::ErrorCheck(iter->second->stop());
	}
}

void longmarch::AudioManager::PauseAllChannels(bool pause)
{
	for (auto iter = m_fmodInstance->mChannels.begin(); iter != m_fmodInstance->mChannels.end(); ++iter)
	{
		AudioManager::ErrorCheck(iter->second->setPaused(pause));
	}
}

void longmarch::AudioManager::SetChannel3dPosition(int nChannelId, const AudioVector3& vPosition)
{
	auto tFoundIt = m_fmodInstance->mChannels.find(nChannelId);
	if (tFoundIt == m_fmodInstance->mChannels.end())
		return;

	FMOD_VECTOR position = VectorToFmod(vPosition);
	AudioManager::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

void longmarch::AudioManager::SetChannelvolume(int nChannelId, float fVolumedB)
{
	auto tFoundIt = m_fmodInstance->mChannels.find(nChannelId);
	if (tFoundIt == m_fmodInstance->mChannels.end())
		return;

	AudioManager::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

bool longmarch::AudioManager::IsPlaying(const std::string& strSoundName)
{
	int id = GetSoundChannel(strSoundName);
	if (id != -1)
	{
		// Using the sound name to channel map to book keep the is playing or not status of sound
		return true;
	}
	else
	{
		return false;
	}
}

bool longmarch::AudioManager::IsPlaying(int nChannelId)
{
	auto tFoundIt = m_fmodInstance->mChannels.find(nChannelId);
	if (tFoundIt == m_fmodInstance->mChannels.end())
	{
		return false;
	}
	bool bIsPlaying;
	tFoundIt->second->isPlaying(&bIsPlaying);
	return bIsPlaying;
}

bool longmarch::AudioManager::IsEventPlaying(const std::string& strEventName) const
{
	auto tFoundIt = m_fmodInstance->mEvents.find(strEventName);
	if (tFoundIt == m_fmodInstance->mEvents.end())
		return false;

	FMOD_STUDIO_PLAYBACK_STATE* state = NULL;
	if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
		return true;
	}
	return false;
}

float longmarch::AudioManager::dbToVolume(float db)
{
	return powf(10.0f, 0.05f * db);
}

float longmarch::AudioManager::VolumeTodb(float volume)
{
	return 20.0f * log10f(volume);
}

FMOD_VECTOR longmarch::AudioManager::VectorToFmod(const AudioVector3& vPosition)
{
	FMOD_VECTOR fVec;
	fVec.x = vPosition.x;
	fVec.y = vPosition.y;
	fVec.z = vPosition.z;
	return fVec;
}

bool longmarch::AudioManager::FadeIn(int fadetime)
{
	unsigned long long dspClock;
	FMOD::System* sys;
	FMOD::Channel* channel = nullptr;
	int rate;

	auto result = channel->getSystemObject(&sys);
	ErrorCheck(result);
	result = sys->getSoftwareFormat(&rate, 0, 0);
	ErrorCheck(result);

	result = channel->getDSPClock(0, &dspClock);
	ErrorCheck(result);
	result = channel->addFadePoint(dspClock, 0.0f);
	ErrorCheck(result);
	result = channel->addFadePoint(dspClock + (double)rate * fadetime, 1.0f);
	ErrorCheck(result);

	return true;
}

bool longmarch::AudioManager::FadeOut(int fadetime)
{
	unsigned long long dspClock;
	FMOD::System* sys;
	FMOD::Channel* channel = nullptr;
	int rate;

	auto result = channel->getSystemObject(&sys);
	ErrorCheck(result);
	result = sys->getSoftwareFormat(&rate, 0, 0);
	ErrorCheck(result);

	result = channel->getDSPClock(0, &dspClock);
	ErrorCheck(result);
	result = channel->addFadePoint(dspClock, 0.0f);
	ErrorCheck(result);
	result = channel->addFadePoint(dspClock + (double)rate * fadetime, 1.0f);
	ErrorCheck(result);
	result = channel->setDelay(0, dspClock + (double)rate * fadetime, true);
	ErrorCheck(result);

	return true;
}

void longmarch::AudioManager::_ON_PAUSE(EventQueue<EngineEventType>::EventPtr e)
{
	if (auto event = std::dynamic_pointer_cast<EngineWindowInterruptionEvent>(e); event)
	{
		PauseAllChannels(!event->m_isFocused && Engine::GetPaused());
	}
}
