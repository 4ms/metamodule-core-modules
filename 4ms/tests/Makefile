BUILDDIR := build

TMPFILE := $(BUILDDIR)/runtests.out

# The EnOsc golden-master regression tests are slow (~30s), so they're grouped
# into the "enosc-golden" doctest suite and excluded from the default run.
# Run them on demand with: make enosc-golden-check
GOLDEN_SUITE := enosc-golden

$(BUILDDIR):
	cmake -S . -B $(BUILDDIR)

all: $(BUILDDIR)
	@cmake --build $(BUILDDIR)
	@$(BUILDDIR)/runtests --out=$(TMPFILE) -tse=$(GOLDEN_SUITE) && echo "[√] Unit tests passed: $(notdir $(PWD))" || cat $(TMPFILE)

# Run only the slow EnOsc golden-master regression tests.
enosc-golden-check: $(BUILDDIR)
	@cmake --build $(BUILDDIR)
	@$(BUILDDIR)/runtests --out=$(TMPFILE) -ts=$(GOLDEN_SUITE) && echo "[√] EnOsc golden tests passed: $(notdir $(PWD))" || cat $(TMPFILE)

clean:
	rm -rf $(BUILDDIR)


.PHONY: all clean enosc-golden-check
