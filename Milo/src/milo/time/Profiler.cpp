#include "milo/time/Profiler.h"
#include <thread>

namespace milo {

	using namespace std::chrono;

	Profiler::Profiler() {

	}

	Profiler::~Profiler() {

		for(auto& [name, session] : m_Sessions) {
			writeFooter(session);
			session->output.close();
			DELETE_PTR(session);
		}

		m_Sessions.clear();
	}

	void Profiler::writeProfile(const String& sessionName, const ProfileResult& result) {
		m_Mutex.lock();
		{
			Session* session = m_Sessions[sessionName];
			if(session == nullptr) {
				session = new Session();
				session->name = sessionName;
				Files::createDirectory("profiling");
				session->filename = Files::append("profiling", sessionName + ".json");
				session->output.open(session->filename);
				writeHeader(session);
				m_Sessions[sessionName] = session;
			}

			writeProfile(session, result);
		}
		m_Mutex.unlock();
	}

	void Profiler::writeHeader(Profiler::Session* session) {
		session->output << R"({"otherData": {},"traceEvents":[)";
		session->output.flush();
	}

	void Profiler::writeFooter(Profiler::Session* session) {
		session->output << "]}";
		session->output.flush();
	}

	void Profiler::writeProfile(Profiler::Session* session, const ProfileResult& result) {

		uint64_t start = duration_cast<microseconds>(result.startTime.time_since_epoch()).count();
		uint64_t end = duration_cast<microseconds>(result.endTime.time_since_epoch()).count();

		if (session->count++ > 0)
			session->output << ",";

		String name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		session->output << "{";
		session->output << R"("cat":"function",)";
		session->output << "\"dur\":" << (end - start) << ',';
		session->output << R"("name":")" << name << "\",";
		session->output << R"("ph":"X",)";
		session->output << "\"pid\":0,";
		session->output << "\"tid\":" << result.threadId << ",";
		session->output << "\"ts\":" << start;
		session->output << "}";

		session->output.flush();
	}

	Profiler* Profiler::s_Profiler = nullptr;

	Profiler& Profiler::get() {
		return *s_Profiler;
	}

	void Profiler::init() {
		s_Profiler = new Profiler();
	}

	void Profiler::shutdown() {
		DELETE_PTR(s_Profiler);
	}

	ProfileTimer::ProfileTimer(const String& name, const String& session)
		: name(name), session(session), start(high_resolution_clock::now()) {
	}

	ProfileTimer::~ProfileTimer() {

		ProfileResult result;
		result.name = std::move(name);
		result.startTime = start;
		result.endTime = high_resolution_clock::now();
		result.threadId = (size_t)std::hash<std::thread::id>{}(std::this_thread::get_id());

		Profiler::get().writeProfile(session, result);
	}
}