SRCS = coder.cpp

TGT_TEST_DIR = test
TGT_TEST     = $(TGT_TEST_DIR)/coder_test
SRCS_TEST    =  $(wildcard $(addsuffix *.cpp,$(TGT_TEST_DIR)/))

CATCH_VERSION = v3.3.2
CATCH_DIR     = catch
CATCH_PATH    = $(CATCH_DIR)/catch_amalgamated
CATCH_SRC     = $(CATCH_PATH).cpp
CATCH_OBJ     = $(CATCH_SRC:.cpp=.o)
CATCH_DL      = $(addprefix $(CATCH_PATH),.cpp .hpp)

CXXFLAGS      = -Wall -Wextra -std=c++20
CXXFLAGS_TEST = -MMD -MP -ggdb

OBJS_TEST = $(SRCS_TEST:.cpp=.o)
DEPS_TEST = $(SRCS_TEST:.cpp=.d)
DBG_TEST  = $(DEPS_TEST:.d=.dSYM)

CC = g++
RM = rm -rf

.PHONY: all test clean dist-clean

all: $(TGT_TEST) test

test: $(TGT_TEST)
	@./$<

$(CATCH_DL):
	mkdir -p catch
	curl -sS -L -o $@ https://github.com/catchorg/Catch2/releases/download/$(CATCH_VERSION)/$(notdir $@)

$(CATCH_OBJ): $(CATCH_DL)
	$(CC) -c $< $(CXXFLAGS) -o $@

$(TGT_TEST): $(CATCH_OBJ) $(SRCS_TEST)
	$(CC) $(CXXFLAGS) $(CXXFLAGS_TEST) $^ -o $@

clean:
	$(RM) $(CATCH_OBJ) $(TGT_TEST) $(DEPS_TEST) $(DBG_TEST)

dist-clean: clean
	$(RM) $(CATCH_DIR)

-include $(DEPS_TEST)
