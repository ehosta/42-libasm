NAME = libasm.a
NAME_BONUS = libasm_bonus.a

SOURCE_DIR = src/
MANDATORY_DIR = $(SOURCE_DIR)mandatory/
BONUS_DIR = $(SOURCE_DIR)bonus/
MANDATORY_FILES = ft_calloc ft_read ft_strcmp ft_strcpy ft_strdup ft_strlen ft_write
BONUS_FILES = ft_atoi_base ft_list_push_front ft_list_remove_if ft_list_size ft_list_sort ft_simd_memchr

SOURCE = $(addprefix $(MANDATORY_DIR), $(addsuffix .s, $(MANDATORY_FILES)))
SOURCE_BONUS = $(addprefix $(BONUS_DIR), $(addsuffix .s, $(BONUS_FILES)))
BUILD_DIR = .dist/
BUILD_DIR_BONUS = .dist_bonus/
OBJECTS = $(patsubst $(MANDATORY_DIR)%.s, $(BUILD_DIR)%.o, $(SOURCE))
OBJECTS_BONUS = $(patsubst $(BONUS_DIR)%.s, $(BUILD_DIR_BONUS)%.o, $(SOURCE_BONUS))

DIRS = $(sort $(dir $(OBJECTS)))
DIRS_BONUS = $(sort $(dir $(OBJECTS_BONUS)))

MAKEFLAGS = --no-print-directory
ASM = nasm
AR = ar
ASMFLAGS = -f elf64
ARFLAGS = rcs
LDFLAGS =
RM = rm -rf

RESET = \e[0m
YELLOW = \e[38;2;255;224;112m [LIBASM] \

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
	@printf "$(YELLOW)❭ Linking$(RESET)\n"
	@$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)%.o: $(MANDATORY_DIR)%.s Makefile | $(DIRS)
	@printf "$(YELLOW)❭ Compiling $<$(RESET)\n"
	@$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS):
	@mkdir -p $@

.PHONY: bonus
bonus: $(NAME_BONUS)

$(NAME_BONUS): $(OBJECTS_BONUS)
	@printf "$(YELLOW)❭ Linking bonus$(RESET)\n"
	@$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR_BONUS)%.o: $(BONUS_DIR)%.o Makefile | $(DIRS_BONUS)
	@printf "$(YELLOW)❭ Compiling bonus $<$(RESET)\n"
	@$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS_BONUS):
	@mkdir -p $@

.PHONY: re
re: fclean
	@$(MAKE)

.PHONY: build
build:
	@printf "$(YELLOW)❭ Building Docker$(RESET)\n"
	@docker build --platform=linux/amd64 -t libasm .

.PHONY: run
run:
	@printf "$(YELLOW)❭ Running Docker$(RESET)\n"
	@docker run --platform=linux/amd64 -v "$(PWD)":/app -w /app -it libasm sh

.PHONY: clean
clean:
	@printf "$(YELLOW)❭ Cleaning project$(RESET)\n"
	@$(RM) $(BUILD_DIR) $(BUILD_DIR_BONUS)

.PHONY: fclean
fclean: clean
	@printf "$(YELLOW)❭ Full cleaning project$(RESET)\n"
	@$(RM) $(NAME) $(NAME_BONUS)

.PHONY: prune
prune: fclean
	@printf "$(YELLOW)❭ Pruning Docker$(RESET)\n"
	@docker builder prune -a -f || true

.PHONY: test
test:
	@$(MAKE) -C tester run

print-%:
	@printf '%s' "$($*)"
