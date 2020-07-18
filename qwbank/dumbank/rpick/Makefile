all: do_rpick do_pays

do_rpick: do_rpick.c xmalloc.c
	cc -o $@ $^

do_pays: do_pays.c xmalloc.c
	cc -o $@ $^

gen_random gen_sched.txt: do_rpick
	./do_rpick 1595019509 | paste patients.txt - | \
	  sed -e 's/	/: /g' >gen_sched.txt

sort_sched.txt: gen_sched.txt
	echo 'Name: Date: Time: Doctor: Travel option' >sort_sched.txt
	sed -e '1d' gen_sched.txt | \
	  awk 'BEGIN { FS = ": "; OFS = ": "; } \
	    { print $$2, $$3, $$4, $$1, $$5; }' | \
	  sort >>sort_sched.txt

sched_nofree.txt: sort_sched.txt
	sed -e '/(free)/d' sort_sched.txt >sched_nofree.txt

tlog_next.txt: do_pays sched_nofree.txt
	./do_pays <sched_nofree.txt >tlog_next.txt

clean:
	rm -f do_rpick do_pays
	rm -f sched_nofree.txt gen_sched.txt tlog_next.txt
