
NAME_NEWSPROG = newsprog
NAME_CONTROL = control

.PHONY: all, clean, touch_$(NAME_NEWSPROG), touch_$(NAME_CONTROL)

all: DEBUG=
all: touch_$(NAME_NEWSPROG) $(NAME_NEWSPROG) touch_$(NAME_CONTROL) $(NAME_CONTROL)

debug: DEBUG=-ggdb3
debug: touch_$(NAME_NEWSPROG) $(NAME_NEWSPROG) touch_$(NAME_CONTROL) $(NAME_CONTROL)

$(NAME_NEWSPROG): $(NAME_NEWSPROG).c
	gcc -Wall $(DEBUG) -o $@ $<

$(NAME_CONTROL): $(NAME_CONTROL).c
	gcc -Wall $(DEBUG) -o $@ $<

clean:
	rm -rf $(NAME_NEWSPROG) $(NAME_CONTROL)

touch_$(NAME_NEWSPROG):
	$(shell [ "x$(DEBUG)" == "x" ] && { nm -a $(NAME_NEWSPROG) | grep -q debug && touch $(NAME_NEWSPROG).c; } )
	$(shell [ "x$(DEBUG)" != "x" ] && { nm -a $(NAME_NEWSPROG) | grep -q debug || touch $(NAME_NEWSPROG).c; } )

touch_$(NAME_CONTROL):
	$(shell [ "x$(DEBUG)" == "x" ] && { nm -a $(NAME_CONTROL) | grep -q debug && touch $(NAME_CONTROL).c; } )
	$(shell [ "x$(DEBUG)" != "x" ] && { nm -a $(NAME_CONTROL) | grep -q debug || touch $(NAME_CONTROL).c; } )
