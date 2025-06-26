NAME	:=	libasm.a

override SOURCE_DIR		:=	src/
SOURCE_FILES			:=	print_hello
override SOURCE			:=	$(addprefix $(SOURCE_DIR), $(addsuffix .s, $(SOURCE_FILES)))

BUILD_DIR		:=	.dist/
override OBJ	:=	$(patsubst $(SOURCE_DIR)%.s, $(BUILD_DIR)%.o, $(SOURCE))
override DIRS	:=	$(sort $(dir $(NAME) $(OBJ)))

ASM			:=	nasm
AR			:=	ar
ASMFLAGS	:=	-f elf64
ARFLAGS		:=	rcs
LDFLAGS		:=
RM			:=	rm -rf
VG			:=	valgrind
VGFLAGS		:=	--leak-check=full --show-leak-kinds=all --track-origins=yes --show-mismatched-frees=yes --track-fds=yes --trace-children=yes

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.s Makefile | $(DIRS)
	$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS):
	@mkdir -p $@

.PHONY: bonus
bonus: all

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean
	$(MAKE)

# ---------------------------------------------------------------------------- #
# This section includes the test of the project.
# It tests everything of the libasm and prints everything.
# ---------------------------------------------------------------------------- #

TESTING_NAME	:=	$(TESTING_DIR)libasm_tests

override TESTING_DIR	:=	testing/
TESTING_HEADERS			:=	$(addsuffix .h, testing)
TESTING_FILES			:=	main
override TESTING		:=	$(addprefix $(TESTING_DIR), $(addsuffix .c, $(TESTING_FILES)))

TESTING_BUILD_DIR		:=	$(TESTING_DIR).dist
override TESTING_OBJ	:=	$(patsubst %.c, $(TESTING_BUILD_DIR)%.o, $(TESTING))
override TESTING_DEPS	:=	$(patsubst %.o, %.d, $(TESTING_OBJ))
override TESTING_DIRS	:=	$(sort $(dir $(TESTING_NAME) $(LIBASM_TESTS) $(DEPS)))

CC			:=	gcc
CFLAGS		:=	-Wall -Wextra -Werror -MD -g3

.PHONY: tests
tests: $(TESTING_NAME)

$(TESTING_NAME): $(TESTING_OBJ) $(NAME)
	$(CC) $(CFLAGS) $(TESTING_OBJ) $(NAME) -o $(TESTING_NAME)

$(TESTING_BUILD_DIR)%.o: $(TESTING_DIR)%.c Makefile $(TESTING_HEADERS) | $(TESTING_DIRS)
	$(CC) $(CFLAGS) -c -I$(TESTING_DIR) $< -o $@

$(TESTING_DIRS):
	@mkdir -p $@

.PHONY: run
run:
	@clear
	@$(MAKE) bonus
	@$(MAKE) compile
	@clear
	@./libasm_tests

.PHONY: vg
vg:
	@clear
	@$(MAKE) bonus
	@$(MAKE) compile
	@clear
	$(VG) $(VGFLAGS) ./libasm_tests

-include $(DEPS)