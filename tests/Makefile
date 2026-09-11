BUILDDIR := build

TMPFILE := $(BUILDDIR)/runtests.out

$(BUILDDIR):
	cmake -S . -B $(BUILDDIR)

all: $(BUILDDIR)
	@cmake --build $(BUILDDIR)
	@$(BUILDDIR)/runtests --out=$(TMPFILE) && echo "[√] Unit tests passed: CoreModules" || cat $(TMPFILE)

clean:
	rm -rf $(BUILDDIR)


.PHONY: all clean
