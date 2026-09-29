#include "doctest.h"

#include "CoreModules/4ms/core/envvca/TriangleOscillator.h"

namespace
{

constexpr float TimeStep = 0.001f;

// Rise and fall each take 1s, so each step moves the output by 5V * TimeStep
TriangleOscillator make_osc() {
	TriangleOscillator osc;
	osc.setRiseTimeInS(1.f);
	osc.setFallTimeInS(1.f);
	return osc;
}

void run(TriangleOscillator &osc, unsigned steps) {
	for (unsigned i = 0; i < steps; i++)
		osc.proceed(TimeStep);
}

} // namespace

TEST_CASE("Retrigger without reset (RETRIG jumper off)") {
	auto osc = make_osc();
	osc.doRetrigger();
	run(osc, 200);
	CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::RISING);
	auto before = osc.getOutput();
	CHECK(before == doctest::Approx(1.f).epsilon(0.01));

	SUBCASE("Trigger while rising is ignored") {
		osc.doRetrigger();
		osc.proceed(TimeStep);
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::RISING);
		CHECK(osc.getOutput() == doctest::Approx(before + 5.f * TimeStep));
	}

	SUBCASE("Trigger while falling rises from the current voltage") {
		run(osc, 1000); // peak at 5V after 1s, then falls
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::FALLING);
		auto falling_level = osc.getOutput();
		CHECK(falling_level > 1.f);

		osc.doRetrigger();
		osc.proceed(TimeStep);
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::RISING);
		CHECK(osc.getOutput() == doctest::Approx(falling_level + 5.f * TimeStep));
	}
}

TEST_CASE("Retrigger with reset (RETRIG jumper on)") {
	auto osc = make_osc();
	osc.doRetrigger(true);
	run(osc, 200);
	CHECK(osc.getOutput() == doctest::Approx(1.f).epsilon(0.01));

	SUBCASE("Trigger while rising restarts from 0V") {
		osc.doRetrigger(true);
		osc.proceed(TimeStep);
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::RISING);
		CHECK(osc.getOutput() == doctest::Approx(5.f * TimeStep));
	}

	SUBCASE("Trigger while falling restarts from 0V") {
		run(osc, 1000);
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::FALLING);

		osc.doRetrigger(true);
		osc.proceed(TimeStep);
		CHECK(osc.getSlopeState() == TriangleOscillator::SlopeState_t::RISING);
		CHECK(osc.getOutput() == doctest::Approx(5.f * TimeStep));
	}

	SUBCASE("Reset only applies to the trigger it was requested with") {
		osc.doRetrigger(true);
		osc.proceed(TimeStep);
		run(osc, 199);
		auto before = osc.getOutput();

		osc.doRetrigger();
		osc.proceed(TimeStep);
		CHECK(osc.getOutput() == doctest::Approx(before + 5.f * TimeStep));
	}
}
