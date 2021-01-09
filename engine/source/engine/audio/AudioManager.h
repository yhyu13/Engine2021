#pragma once

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "engine/core/EngineCore.h"
#include "engine/math/Geommath.h"
#include "engine/core/thread/Lock.h"
#include "engine/events/engineEvents/EngineEventType.h"
#include "engine/events/EventQueue.h"

namespace longmarch {
	using AudioVector3 = Vec3f;
	struct ENGINE_API FMODInstance
	{
		FMODInstance();
		~FMODInstance();

		void Update();

		FMOD::Studio::System* mpStudioSystem;
		FMOD::System* mpSystem;

		int mnNextChannelId;

		typedef std::map<std::string, FMOD::Sound*> SoundMap;
		typedef std::map<std::string, int> Sound2ChannelMap;
		typedef std::map<int, std::string> Channel2SoundMap;
		typedef std::map<int, FMOD::Channel*> ChannelMap;
		typedef std::map<std::string, FMOD::Studio::EventInstance*> EventMap;
		typedef std::map<std::string, FMOD::Studio::Bank*> BankMap;

		BankMap mBanks;
		EventMap mEvents;
		SoundMap mSounds;
		ChannelMap mChannels;
		Sound2ChannelMap mSound2Channels;
		Channel2SoundMap mChannels2Sound;
	};

	class ENGINE_API AudioManager final : public BaseAtomicClassNC, public BaseEventSubHandleClass
	{
	private:
		NONCOPYABLE(AudioManager);
		AudioManager();

	public:
		static AudioManager* GetInstance();
		void Update(double deltaTime);

		static int ErrorCheck(FMOD_RESULT result);

		int GetSoundChannel(const std::string& strSoundName);
		void LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
		void LoadEvent(const std::string& strEventName);

		void LoadSound(const std::string& strSoundName, const fs::path& strSoundPath, bool bLooping = false, bool b3d = true, bool bStream = true);
		void UnLoadSound(const std::string& strSoundName);
		void Set3dListenerAndOrientation(const AudioVector3& vPos = AudioVector3{ 0, 0, 0 }, float fVolumedB = 1.0f);
		int PlaySoundByName(const std::string& strSoundName, const AudioVector3& vPos = AudioVector3{ 0, 0, 0 }, float fVolumedB = 1.0f, float frequency = 1.0f);
		int StopSound(const std::string& strSoundName);
		void SetSoundFreqency(const std::string& strSoundName, float frequency);
		void SetSoundDB(const std::string& strSoundName, float fVolumedB);
		void SetSoundVol(const std::string& strSoundName, float vol);
		void SetSoundPause(const std::string& strSoundName, bool pause);
		void SetSoundMute(const std::string& strSoundName, bool mute);
		/////////////////////////////////////////////////////////
		void PlayEvent(const std::string& strEventName);
		void StopChannel(int nChannelId);
		void StopEvent(const std::string& strEventName, bool bImmediate = false);
		void StopAllChannels();
		void PauseAllChannels(bool pause);
		void SetChannel3dPosition(int nChannelId, const AudioVector3& vPosition);
		void SetChannelvolume(int nChannelId, float fVolumedB);
		bool IsPlaying(const std::string& strSoundName);
		bool IsPlaying(int nChannelId);
		bool IsEventPlaying(const std::string& strEventName) const;
		float dbToVolume(float db);
		float VolumeTodb(float volume);
		FMOD_VECTOR VectorToFmod(const AudioVector3& vPosition);
		bool FadeIn(int fadetime);
		bool FadeOut(int fadetime);

	private:
		void _ON_PAUSE(EventQueue<EngineEventType>::EventPtr e);

	private:
		std::unique_ptr<FMODInstance> m_fmodInstance{ nullptr };
	};
}