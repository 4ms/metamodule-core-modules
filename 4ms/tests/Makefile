BUILDDIR := build

TMPFILE := $(BUILDDIR)/runtests.out

# The EnOsc golden-master regression tests are slow (~30s) and are marked with
# doctest's skip() decorator (see enosc_golden_tests.cc), so doctest never runs
# them by default -- not from `make all`, not from CI, not even when runtests is
# invoked directly. Opt in explicitly (runtests --no-skip) via the dedicated
# target below: make enosc-golden-check
GOLDEN_SUITE := enosc-golden

$(BUILDDIR):
	cmake -S . -B $(BUILDDIR)

all: $(BUILDDIR)
	@cmake --build $(BUILDDIR)
	@$(BUILDDIR)/runtests --out=$(TMPFILE) && echo "[√] Unit tests passed: $(notdir $(PWD))" || cat $(TMPFILE)

# Explicitly run the slow, skip()-marked EnOsc golden-master regression tests.
# --no-skip is what un-gates them; -ts then narrows the run to just that suite.
enosc-golden-check: $(BUILDDIR)
	@cmake --build $(BUILDDIR)
	@$(BUILDDIR)/runtests --out=$(TMPFILE) --no-skip -ts=$(GOLDEN_SUITE) && echo "[√] EnOsc golden tests passed: $(notdir $(PWD))" || cat $(TMPFILE)

clean:
	rm -rf $(BUILDDIR)


.PHONY: all clean enosc-golden-check
