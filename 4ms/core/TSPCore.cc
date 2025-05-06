//#include "../../../../../src/medium/debug_raw.h"
#include "CoreModules/register_module.hh"

#include "CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/async_thread.hh"
#include "CoreModules/moduleFactory.hh"
#include "filesystem/async_filebrowser.hh"
#include "info/TSP_info.hh"
#include "util/circular_buffer.hh"
#include "util/static_string.hh"
#include <atomic>
#include <chrono>

namespace MetaModule
{

class TSPCore : public SmartCoreProcessor<TSPInfo> {
	using enum TSPInfo::Elem;

	//1MB max pre-buffering = 2-10sec of buffering, depending on the sample format
	static constexpr size_t MaxBufferSize = 1 * 1024 * 1024;

	CircularBuffer<float, MaxBufferSize> pre_buff;

	float sample_rate = 48000.f;

	enum class FileSysAction {
		Idle,
		FileBrowser,
		// NextBank,
		// PrevBank,
		// NextSample,
		// PrevSample,
		// ReadSampleHeader,
		// Process,
	};

	std::atomic<FileSysAction> fs_state{FileSysAction::Idle};

	// This runs in the low-pri thread:
	AsyncThread fs_thread{this, [this]() {
							  if (fs_state == FileSysAction::FileBrowser) {
								  //
							  }
						  }};

public:
	void update() override {
		// sampler.update();
	}

	void set_param(int id, float val) override {
		if (id == param_idx<SampledirAltParam>) {
			if (val > 0.5) {

				// fs_state = FileSysAction::FileBrowser;
				async_open_file(sample_dir, ".wav, .WAV", "Load sample:", [this](char *path) {
					if (path) {
						sample_filename = path;
						printf("Open file %s\n", path);
						free(path);
					}
				});
			}
		}

		SmartCoreProcessor::set_param(id, val);
	}

	void set_samplerate(float sr) override {
		sample_rate = sr;
	}

	enum class PlayState {
		Stopped,
		Prebuffering,
		FadingUp,
		Playing,
		FadingDownToStop,
		FadingDownToLoop,
	};

	StaticString<255> sample_filename = "";
	StaticString<255> sample_dir = "";

	static inline bool was_registered = register_module<TSPCore, TSPInfo>("4msCompany");
};

} // namespace MetaModule
