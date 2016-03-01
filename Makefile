SRCS = test.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(SRCS:.cc=.d)
HEADERS = $(SRCS:.cc=.h) burrows-wheeler-transform.h size-type.h range-coder.h

PROJECT = test

default: CHECK $(OBJS) $(PROJECT) RUN

.PHONY: CHECK
CHECK:
	./cpplint.py --filter=-build/c++11 $(SRCS) $(HEADERS)

.PHONY: RUN
RUN: $(PROJECT)
	./$(PROJECT)

$(PROJECT): $(OBJS)
	clang++ $^ -o $@

%.o: %.cc Makefile
	clang++ -c $< -o $@ -std=c++1y -MMD -MP -Weverything -Wno-c++98-compat -Wno-reserved-id-macro -Wno-padded -Wno-format-nonliteral -Wno-c++98-compat-pedantic -Wno-weak-vtables

.PHONY: clean
clean:
	rm -f *.d *.o

.PHONY: sync
sync:
	git push origin master

-include $(DEPS)
