NAME	:=	libasm.a

override SOURCE_DIR		:=	src/
SOURCE_FILES			:=	ft_strlen ft_strcpy
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
