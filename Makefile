LIBS = burrows-wheeler-transform.h size-type.h range-coder.h move-to-front.h fast-fourier-transform.h
SRCS = $(LIBS:%.h=tests/%-test.cc)
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
HEADERS = $(SRCS:.cc=.h) $(LIBS)
EXES = $(SRCS:.cc=.out)

default: SYNTAX_CHECK $(OBJS) $(PROJECT) TEST

.PHONY: SYNTAX_CHECK
SYNTAX_CHECK:
	./cpplint.py --filter=-build/c++11 $(SRCS) $(HEADERS)

.PHONY: TEST
TEST: $(EXES)
	@for e in $(EXES); do ./$$e; if [ $$? -ne 0 ]; then echo "[test] [1;31mfailed. [0m $$e"; else echo "[test] [32msucceed.[0m $$e"; fi; done

%.out: %.o
	clang++ $< -o $@

%.o: %.cc Makefile
	clang++ -c $< -o $@ -std=c++1y -MMD -MP -Weverything -Wno-c++98-compat -Wno-reserved-id-macro -Wno-padded -Wno-format-nonliteral -Wno-c++98-compat-pedantic -Wno-weak-vtables

.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXES) $(DEPS)

.PHONY: sync
sync:
	git pull origin master
	git push origin master

-include $(DEPS)
