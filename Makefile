CC = clang
CXX = clang++
LINK = clang++
MKDIR = mkdir
LS = ls --color=no
CP = cp
CXXWARNFLAGS = -Weverything -Wno-c++98-compat -Wno-reserved-id-macro -Wno-padded -Wno-format-nonliteral -Wno-c++98-compat-pedantic -Wno-weak-vtables -Wno-documentation-unknown-command -Wno-documentation -Wno-missing-prototypes

LIBS = $(wildcard includes/*.h)
SRCS = $(LIBS:includes/%.h=tests/%-test.cc)
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
HEADERS = $(SRCS:.cc=.h) $(LIBS)
EXES = $(SRCS:.cc=.out) tests/multiple-link-checker.out

# if exists libc++, use it.
LIBCPP = $(shell if $(CXX) dummy.cc -o dummy.out -lc++ -std=c++1y > /dev/null 2>&1; then echo '-lc++'; else echo '-lstdc++'; fi)

INSTALL_DIR = $(shell cat libresearch.pc | grep 'includedir=' | sed 's/includedir=//')

default: SYNTAX_CHECK check docs

.PHONY: SYNTAX_CHECK
SYNTAX_CHECK:
	./cpplint.py --filter=-build/c++11 $(SRCS) $(HEADERS)

.PHONY: check
check: $(EXES)
	@for e in $(EXES); do echo "[test] [36mtesting[0m  $$e[F"; ./$$e; if [ $$? -ne 0 ]; then echo "[test] [1;31mfailed. [0m $$e"; else echo "[test] [32msucceed.[0m $$e"; fi; done

.PHONY: install
install:
	$(MKDIR) -p $(INSTALL_DIR)
	$(CP) $(LIBS) $(INSTALL_DIR)/
	$(CP) libresearch.pc /usr/local/lib/pkgconfig/

.PHONY: uninstall
uninstall:
	$(RM) /usr/local/lib/pkgconfig/libresearch.pc
	$(RM) -rf $(INSTALL_DIR)

.PHONY: docs
docs:
	doxygen Doxyfile

%.out: %.o
	$(LINK) $< -o $@ $(LIBCPP) -lm

tests/multiple-link-checker.out: tests/multiple1.o tests/multiple2.o
	$(LINK) $^ -o $@ $(LIBCPP) -lm

tests/multiple1.o tests/multiple2.o: tests/multiple.h

tests/multiple.h: $(LIBS)
	$(LS) ./includes | sed 's!^!#include "../includes/!' | sed 's/$$/"/' > $@

%.o: %.cc Makefile
	$(CXX) -c $< -o $@ -std=c++1y -MMD -MP -DRESEARCHLIB_OFFLINE_TEST $(CXXWARNFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXES) $(DEPS) dummy.out

.PHONY: sync
sync:
	git pull origin master
	git push origin master

-include $(DEPS)
