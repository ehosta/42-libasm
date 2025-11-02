NAME = libasm_insider

HEADER_DIR = include/
SOURCE_DIR = src/
HEADER_FILES = insider
SOURCE_FILES = main string subprocess

HEADER = $(addsuffix .h, $(addprefix $(HEADER_DIR), $(HEADER_FILES)))
SOURCE = $(addsuffix .c, $(addprefix $(SOURCE_DIR), $(SOURCE_FILES)))
BUILD_DIR = .objects/
OBJECTS = $(patsubst $(SOURCE_DIR)%.c, $(BUILD_DIR)%.o, $(SOURCE))
DEPS_DIR = .deps/
DEPS = $(patsubst $(SOURCE_DIR)%.c, $(DEPS_DIR)%.d, $(SOURCE))

DIRS = $(sort $(dir $(OBJECTS) $(DEPS)))

MAKEFLAGS = --no-print-directory
CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wshadow -g3 -std=c99 -MMD -MP
RM = rm -rf
VG = valgrind
VGFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --show-mismatched-frees=yes --track-fds=yes --trace-children=yes --suppressions=make.supp

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.c $(HEADER) Makefile | $(DIRS)
	$(CC) $(CFLAGS) -MF $(DEPS_DIR)/$*.d -I$(HEADER_DIR) -c $< -o $@

$(DIRS):
	@mkdir -p $@

.PHONY: re
re: fclean
	$(MAKE)

.PHONY: run
run: fclean all
	./$(NAME) all

.PHONY: vg
vg: fclean all
	$(VG) $(VGFLAGS) ./$(NAME) all

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR) $(DEPS_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

-include $(DEPS)
