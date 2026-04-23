CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c23 -g
LDFLAGS ?=
LDLIBS ?=


ifeq ($(OS),Windows_NT)
  EXEEXT := .exe
else
  EXEEXT :=
endif

TARGET := main$(EXEEXT)

RM ?= rm -f
LDLIBS += -lm


all: compile

$(TARGET): main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

compile:
	$(MAKE) $(TARGET)
run:
	./$(TARGET)
	./$(TARGET) --dump

clean:
	-$(RM) "$(TARGET)"
	-$(RM) analisys/json_data/*.json