BUILDDIR := build

TMPFILE := $(BUILDDIR)/runtests.out

$(BUILDDIR):
	cmake -S . -B $(BUILDDIR)

all: $(BUILDDIR)
	@cmake --build $(BUILDDIR) -- --no-print-directory
	@$(BUILDDIR)/runtests --out=$(TMPFILE) && echo "[√] Unit tests passed: $(notdir $(PWD))" || cat $(TMPFILE)

clean:
	rm -rf $(BUILDDIR)


.PHONY: all clean
