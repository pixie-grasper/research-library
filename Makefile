LIBS = burrows-wheeler-transform.h size-type.h range-coder.h
SRCS = $(LIBS:.h=-test.cc)
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
	for e in $(EXES); do\
		./$$e;\
	done

%.out: %.o
	clang++ $< -o $@

%.o: %.cc Makefile
	clang++ -c $< -o $@ -std=c++1y -MMD -MP -Weverything -Wno-c++98-compat -Wno-reserved-id-macro -Wno-padded -Wno-format-nonliteral -Wno-c++98-compat-pedantic -Wno-weak-vtables

.PHONY: clean
clean:
	rm -f *.d *.o

.PHONY: sync
sync:
	git pull origin master
	git push origin master

-include $(DEPS)
