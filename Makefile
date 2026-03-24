# MLB-Tubes-IF2224-2026 - Linux / WSL (GNU Make + g++)

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Isrc
LDFLAGS  :=
LDLIBS   :=

# Set DEBUG=1 for symbols and no optimization: `make DEBUG=1`
ifeq ($(DEBUG),1)
  CXXFLAGS += -g -O0
else
  CXXFLAGS += -O2
endif

TARGET   := arion
SRCDIR   := src
OBJDIR   := bin

# One level of subdirs (e.g. src/lexer/*.cpp); add more globs if you nest deeper
SOURCES := $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)

OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

-include $(DEPS)
