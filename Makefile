NAME	:=	libasm.a

override SOURCE_DIR		:=	src/
SOURCES					:=	main
override SOURCE			:=	$(addprefix $(SOURCE_DIR), $(addsuffix .s, $(SOURCES)))

IMAGE			:= full-nasm-env
TAG				:= latest
DOCKER_RUN_ARGS	:= --rm -v "$(PWD)" -w / $(IMAGE):$(TAG)

BUILD_DIR		:=	.dist/
override OBJ	:=	$(patsubst $(SOURCE_DIR)%.s, $(BUILD_DIR)%.o, $(SOURCE))
override DIRS	:=	$(sort $(dir $(NAME) $(OBJ) $(DEPS)))

ASM			:=	nasm
LD			:=	ld
ASMFLAGS	:= -f elf64
LDFLAGS		:=
RM			:=	rm -rf
VG			:=	valgrind
VGFLAGS		:=	--leak-check=full --show-leak-kinds=all --track-origins=yes --show-mismatched-frees=yes --track-fds=yes --trace-children=yes

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@ $^

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.s Makefile | $(DIRS)
	$(ASM) $(ASMFLAGS) -o $@ $<

$(DIRS):
	@mkdir -p $@

docker-build:
	docker build -t $(IMAGE):$(TAG) .

docker:
	$(MAKE) docker-build
	docker run --env TERM=xterm-256color $(DOCKER_RUN_ARGS) make all -C libasm/

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

.PHONY: run
run:
	@clear
	@$(MAKE) bonus
	@clear
	@./$(NAME)

.PHONY: vg
vg:
	@clear
	@$(MAKE) bonus
	@clear
	$(VG) $(VGFLAGS) ./$(NAME)