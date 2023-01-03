all: level_1.html missing_level_1.html level_2.html level_3.html

level_1.html: level_1.yml
	./ygen.pl $< 'level 1' >$@

missing_level_1.html: level_1.yml miss_head.html
	./ygen.pl $< 'level 1' miss_head.html >$@

level_2.html: level_2.yml
	./ygen.pl $< 'level 2' >$@

level_3.html: level_3.yml
	./ygen.pl $< 'level 3' >$@

../sign_your_name/level_1.html: level_1.html
	cp -p $< $@

../sign_your_name/level_1_log_tail.cgi: ../sign_your_name/log_tail.cgi
	( cd ../sign_your_name && \
	  ln -s log_tail.cgi level_1_log_tail.cgi )

../sign_your_name/missing_level_1.html: missing_level_1.html
	cp -p $< $@

../sign_your_name/level_2.html: level_2.html
	cp -p $< $@

../sign_your_name/level_2_log_tail.cgi: ../sign_your_name/log_tail.cgi
	( cd ../sign_your_name && \
	  ln -s log_tail.cgi level_2_log_tail.cgi )

../sign_your_name/level_3.html: level_3.html
	cp -p $< $@

../sign_your_name/level_3_log_tail.cgi: ../sign_your_name/log_tail.cgi
	( cd ../sign_your_name && \
	  ln -s log_tail.cgi level_3_log_tail.cgi )

install: ../sign_your_name/level_1.html \
	../sign_your_name/level_1_log_tail.cgi \
	../sign_your_name/missing_level_1.html \
	../sign_your_name/level_2.html \
	../sign_your_name/level_2_log_tail.cgi \
	../sign_your_name/level_3.html \
	../sign_your_name/level_3_log_tail.cgi

uninstall:
	rm -f ../sign_your_name/level_1.html \
	  ../sign_your_name/level_1_log_tail.cgi \
	  ../sign_your_name/missing_level_1.html \
	  ../sign_your_name/level_2.html \
	  ../sign_your_name/level_2_log_tail.cgi \
	  ../sign_your_name/level_3.html \
	  ../sign_your_name/level_3_log_tail.cgi

clean:
	rm -f level_1.html missing_level_1.html level_2.html level_3.html
