
// This function is called by F4SE while initializing. During this call, you
// provide F4SE with your plugin's name, version and info version, by modifying
// the F4SE::PluginInfo you get passed using a pointer. You can set up a logger
// here too, if you want, but it isn't strictly required.
// Finally, we check if our plugin can safely be loaded by F4SE.
// If we can, we return true. Otherwise false.
extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);
    F4SE::GetMessagingInterface()->RegisterListener([](F4SE::MessagingInterface::Message *message) {

        if (message->type == F4SE::MessagingInterface::kGameDataReady) {
            auto* dataHandler = RE::TESDataHandler::GetSingleton();
            auto& forms = dataHandler->GetFormArray<RE::TESLevItem>();
            for (auto* lvlform : forms) {
                if (lvlform->chanceNone > 0 && lvlform->chanceNone < 100) {
                logger::info("patching the form : {0}", lvlform->GetFormID());
                    lvlform->chanceNone = 0;
                }
            }
        }
    });
	logger::info("hello world!");
	
	return true;
}
