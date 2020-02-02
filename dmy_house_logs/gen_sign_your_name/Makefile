all: level_1.html level_2.html level_3.html

level_1.html: level_1.yml
	./ygen.pl $< 'level 1' >$@

level_2.html: level_2.yml
	./ygen.pl $< 'level 2' >$@

level_3.html: level_3.yml
	./ygen.pl $< 'level 3' >$@

../sign_your_name/level_1.html: level_1.html
	cp -p $< $@

../sign_your_name/level_2.html: level_2.html
	cp -p $< $@

../sign_your_name/level_3.html: level_3.html
	cp -p $< $@

install: ../sign_your_name/level_1.html \
	../sign_your_name/level_2.html \
	../sign_your_name/level_3.html

uninstall:
	rm -f ../sign_your_name/level_1.html \
	  ../sign_your_name/level_2.html \
	  ../sign_your_name/level_3.html

clean:
	rm -f level_1.html level_2.html level_3.html
