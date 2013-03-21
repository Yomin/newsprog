.PHONY: all, clean

NAME = newsprog

all: $(NAME)

$(NAME): $(NAME).c
	gcc -Wall -o $@ $<

clean:
	rm -rf $(NAME)
