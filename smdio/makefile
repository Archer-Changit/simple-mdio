# ============================================================================
# Makefile - 
# ============================================================================

# compiler
CC       = gcc
CXX      = g++
CFLAGS   = -Wall -O2
CXXFLAGS = -Wall -O2
LDFLAGS += 

# executable file name
TARGET = smdio

SRCDIR = .
OBJDIR = objs
BINDIR = .

# .c file
C_SOURCES   = $(wildcard $(SRCDIR)/*.c)

# geneate .o file list(in OBJDIR directory)
C_OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(C_SOURCES:.c=.o)))

ALL_OBJECTS = $(C_OBJECTS)

INC_DIRS = 
CFLAGS   += $(addprefix -I,$(INC_DIRS))
CXXFLAGS += $(addprefix -I,$(INC_DIRS))

# ifeq ($(SMDIO_DEBUG),1)
# CFLAGS   += -DSMDIO_DEBUG=$(SMDIO_DEBUG)
# CXXFLAGS += -DSMDIO_DEBUG=$(SMDIO_DEBUG)
# endif

# ============================================================================
# rule definition
# ============================================================================

.PHONY: all clean distclean rebuild info

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(ALL_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

distclean: clean
	rm -f $(BINDIR)/$(TARGET)

rebuild: distclean all

info:
	@echo "C sources: $(C_SOURCES)"
	@echo "Objects: $(ALL_OBJECTS)"


	