#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/moduleFactory.hh"
#include "HubMedium_info.hh"

namespace MetaModule
{

class HubMedium : public CoreProcessor {
public:
	using Info = HubMediumInfo;
	using ThisCore = HubMedium;

	HubMedium() = default;

	void update() final {
	}

	void set_samplerate(float sr) final {
	}

	void set_param(int param_id, float val) final {
	}

	void set_input(int jack_id, float val) final {
		if (jack_id >= 0 && jack_id < 8)
			jackvals[jack_id] = val;
	}

	float get_output(int jack_id) const final {
		if (jack_id >= 0 && jack_id < 8)
			return jackvals[jack_id];
		else
			return 0;
	}

	float jackvals[8]{};

	// Boilerplate to auto-register in ModuleFactory
	static inline bool s_registered = ModuleFactory::registerModuleType(
		Info::slug, create_module<ThisCore>, ModuleInfoView::makeView<Info>(), Info::png_filename);
};

} // namespace MetaModule
