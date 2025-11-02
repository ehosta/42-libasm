NAME = libasm.a
NAME_BONUS = libasm_bonus.a

SOURCE_DIR = src/
MANDATORY_DIR = $(SOURCE_DIR)mandatory/
BONUS_DIR = $(SOURCE_DIR)bonus/
MANDATORY_FILES = ft_strcpy ft_strlen
BONUS_FILES =

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

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)%.o: $(MANDATORY_DIR)%.s Makefile | $(DIRS)
	$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS):
	@mkdir -p $@

.PHONY: bonus
bonus: $(NAME_BONUS)

$(NAME_BONUS): $(OBJECTS_BONUS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR_BONUS)%.o: $(BONUS_DIR)%.o Makefile | $(DIRS_BONUS)
	$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS_BONUS):
	@mkdir -p $@

.PHONY: re
re: fclean
	$(MAKE)
	$(MAKE) bonus

.PHONY: build
build:
	docker build --platform=linux/amd64 -t libasm .

.PHONY: run
run:
	docker run --platform=linux/amd64 -v "$(PWD)":/app -w /app -it libasm bash

.PHONY: clean
clean:
	$(RM) $(BUILD_DIR) $(BUILD_DIR_BONUS)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME) $(NAME_BONUS)

.PHONY: prune
prune: fclean
	docker builder prune -a -f || true

print-%:
	@printf '%s' "$($*)"
