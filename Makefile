NAME	:=	libasm.a

override SOURCE_DIR		:=	src/
SOURCE_FILES			:=	ft_strlen print_hello
override SOURCE			:=	$(addprefix $(SOURCE_DIR), $(addsuffix .s, $(SOURCE_FILES)))

BUILD_DIR		:=	.dist/
override OBJ	:=	$(patsubst $(SOURCE_DIR)%.s, $(BUILD_DIR)%.o, $(SOURCE))

TESTING_NAME	:=	libasm_tests

override TESTING_DIR	:=	testing/
TESTING_HEADERS			:=	$(addprefix $(TESTING_DIR), $(addsuffix .h, testing))
TESTING_FILES			:=	$(addprefix fct/, ft_strlen print_hello) \
							$(addprefix utils/, random_seq) \
							main
override TESTING		:=	$(addprefix $(TESTING_DIR), $(addsuffix .c, $(TESTING_FILES)))

TESTING_BUILD_DIR		:=	.tests/
override TESTING_OBJ	:=	$(patsubst $(TESTING_DIR)%.c, $(TESTING_BUILD_DIR)%.o, $(TESTING))
override TESTING_DEPS	:=	$(patsubst %.o, %.d, $(TESTING_OBJ))

override DIRS	:=	$(sort $(dir $(NAME) $(OBJ) $(TESTING_NAME) $(TESTING_BUILD_DIR) $(TESTING_DEPS)))

ASM			:=	nasm
AR			:=	ar
ASMFLAGS	:=	-f elf64
ARFLAGS		:=	rcs
LDFLAGS		:=
RM			:=	rm -rf

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.s $(SOURCE_DIR)ft_libasm.h Makefile | $(DIRS)
	$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS):
	@mkdir -p $@

.PHONY: bonus
bonus: all

.PHONY: re
re: fclean
	$(MAKE)

# ---------------------------------------------------------------------------- #
# This section includes the test of the project.
# It tests everything of the libasm and prints everything.
# ---------------------------------------------------------------------------- #

CC			:=	gcc
CFLAGS		:=	-Wall -Wextra -Werror -g3
VG			:=	valgrind
VGFLAGS		:=	--leak-check=full --show-leak-kinds=all --track-origins=yes --show-mismatched-frees=yes --track-fds=yes --trace-children=yes

.PHONY: tests
tests: all $(TESTING_NAME)

$(TESTING_NAME): $(TESTING_OBJ) $(NAME)
	$(CC) $(CFLAGS) $(TESTING_OBJ) $(NAME) -o $(TESTING_NAME)

$(TESTING_BUILD_DIR)%.o: $(TESTING_DIR)%.c Makefile $(TESTING_HEADERS) | $(DIRS)
	$(CC) $(CFLAGS) -c -I$(TESTING_DIR) -I$(SOURCE_DIR) $< -o $@

.PHONY: run
run:
	@clear
	@$(MAKE) bonus
	@$(MAKE) tests
	@clear
	@./libasm_tests

.PHONY: vg
vg:
	@clear
	@$(MAKE) bonus
	@$(MAKE) tests
	@clear
	$(VG) $(VGFLAGS) ./libasm_tests

.PHONY: norm
norm:
	norminette $(TESTING_DIR) $(SOURCE_DIR)

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR)
	$(RM) $(TESTING_BUILD_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)
	$(RM) $(TESTING_NAME)