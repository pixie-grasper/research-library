CC = clang
CXX = clang++
LINK = clang++

LIBS = $(wildcard *.h)
SRCS = $(LIBS:%.h=tests/%-test.cc)
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
HEADERS = $(SRCS:.cc=.h) $(LIBS)
EXES = $(SRCS:.cc=.out)

# if exists libc++, use it.
LIBCPP = $(shell if $(CXX) dummy.cc -o dummy.out -lc++ > /dev/null 2>&1; then echo '-lstdc++'; else echo '-lc++'; fi)

default: SYNTAX_CHECK check

.PHONY: SYNTAX_CHECK
SYNTAX_CHECK:
	./cpplint.py --filter=-build/c++11 $(SRCS) $(HEADERS)

.PHONY: check
check: $(EXES)
	@for e in $(EXES); do ./$$e; if [ $$? -ne 0 ]; then echo "[test] [1;31mfailed. [0m $$e"; else echo "[test] [32msucceed.[0m $$e"; fi; done

%.out: %.o
	$(LINK) $< -o $@ $(LIBCPP)

%.o: %.cc Makefile
	$(CXX) -c $< -o $@ -std=c++1y -MMD -MP -Weverything -Wno-c++98-compat -Wno-reserved-id-macro -Wno-padded -Wno-format-nonliteral -Wno-c++98-compat-pedantic -Wno-weak-vtables

.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXES) $(DEPS) dummy.out

.PHONY: sync
sync:
	git pull origin master
	git push origin master

-include $(DEPS)
