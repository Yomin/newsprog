.PHONY: all, clean, touch

NAME = newsprog

all: DEBUG=
all: touch $(NAME)

debug: DEBUG=-ggdb3
debug: touch $(NAME)

$(NAME): $(NAME).c
	gcc -Wall $(DEBUG) -o $@ $<

clean:
	rm -rf $(NAME)

touch:
	$(shell [ "x$(DEBUG)" == "x" ] && { nm -a $(NAME) | grep -q debug && touch $(NAME).c; } )
	$(shell [ "x$(DEBUG)" != "x" ] && { nm -a $(NAME) | grep -q debug || touch $(NAME).c; } )
