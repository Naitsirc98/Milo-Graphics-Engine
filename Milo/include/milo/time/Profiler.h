#pragma once

#include "Time.h"
#include "milo/io/Files.h"

#define MILO_PROFILE_FUNCTION milo::ProfileTimer _profileTimer(__FUNCTION__, milo::DEFAULT_PROFILER_SESSION_NAME)

namespace milo {

	const String DEFAULT_PROFILER_SESSION_NAME = "Milo Benchmark";

	struct ProfileResult {
		String name;
		float startTime;
		float endTime;
		size_t threadId;
	};

	class Profiler {
		friend class MiloSubSystemManager;
		friend class ProfileTimer;
	private:
		struct Session {
			String name;
			String filename;
			OutputStream output;
			uint32_t count=0;
		};
	private:
		HashMap<String, Session*> m_Sessions;
		Mutex m_Mutex;
	private:
		Profiler();
		~Profiler();
		void writeProfile(const String& session, const ProfileResult& result);
	private:
		void writeHeader(Session* session);
		void writeFooter(Session* session);
		void writeProfile(Session* session, const ProfileResult& result);
	private:
		static Profiler* s_Profiler;
	public:
		static Profiler& get();
	private:
		static void init();
		static void shutdown();
	};

	struct ProfileTimer {

		String session;
		String name;
		float start;

		ProfileTimer(const String& name, const String& session = DEFAULT_PROFILER_SESSION_NAME);
		~ProfileTimer();
	};
}