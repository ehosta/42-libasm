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
VGFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --show-mismatched-frees=yes --track-fds=yes #--trace-children=yes

RESET = \e[0m
PURPLE = \e[38;2;164;112;255m [TESTER] \

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
	@printf "$(PURPLE)❭ Linking$(RESET)\n"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.c $(HEADER) Makefile | $(DIRS)
	@printf "$(PURPLE)❭ Compiling $<$(RESET)\n"
	@$(CC) $(CFLAGS) -MF $(DEPS_DIR)/$*.d -I$(HEADER_DIR) -c $< -o $@

$(DIRS):
	@mkdir -p $@

.PHONY: re
re: fclean
	@$(MAKE)

.PHONY: run
run: fclean all
	@printf "$(PURPLE)❭ Running mandatory$(RESET)\n"
	@./$(NAME) mandatory

.PHONY: brun
brun: fclean all
	@printf "$(PURPLE)❭ Running bonus$(RESET)\n"
	@./$(NAME) bonus

.PHONY: vg
vg: fclean all
	@printf "$(PURPLE)❭ Running mandatory with Valgrind$(RESET)\n"
	@$(VG) $(VGFLAGS) ./$(NAME) mandatory

.PHONY: bvg
bvg: fclean all
	@printf "$(PURPLE)❭ Running bonus with Valgrind$(RESET)\n"
	@$(VG) $(VGFLAGS) ./$(NAME) bonus

.PHONY: clean
clean:
	@printf "$(PURPLE)❭ Cleaning project$(RESET)\n"
	@$(RM) $(BUILD_DIR) $(DEPS_DIR)
	@$(MAKE) clean -C ../

.PHONY: fclean
fclean: clean
	@printf "$(PURPLE)❭ Full cleaning project$(RESET)\n"
	@$(RM) $(NAME) unit_test
	@$(MAKE) fclean -C ../

-include $(DEPS)
