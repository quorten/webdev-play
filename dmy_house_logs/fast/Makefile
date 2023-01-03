CC = gcc
CFLAGS = -g
LDFLAGS =
bin_PROGRAMS = test_dumb_cgipm view_logs.cgi laundry_logs.cgi \
  log_tail.cgi view_logs_tail.cgi add_log.cgi

all: $(bin_PROGRAMS)

dumb_cgipm.o: dumb_cgipm.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

test_dumb_cgipm.o: test_dumb_cgipm.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

test_dumb_cgipm: test_dumb_cgipm.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

view_logs.o: view_logs.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

view_logs.cgi: view_logs.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

laundry_logs.o: laundry_logs.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

laundry_logs.cgi: laundry_logs.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

tmpl_log_tail.o: tmpl_log_tail.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

log_tail.o: log_tail.c tmpl_log_tail.h
	$(CC) $(CFLAGS) -c $<

log_tail.cgi: log_tail.o tmpl_log_tail.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

view_logs_tail.o: view_logs_tail.c tmpl_log_tail.h
	$(CC) $(CFLAGS) -c $<

view_logs_tail.cgi: view_logs_tail.o tmpl_log_tail.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

add_log.o: add_log.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

add_log.cgi: add_log.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

test check: test_dumb_cgipm
	./test_dumb_cgipm | tee test_results.txt

install: $(bin_PROGRAMS)
	cp -p log_tail.cgi add_log.cgi view_logs_tail.cgi \
	  ../sign_your_name/
	cp -p view_logs.cgi laundry_logs.cgi ..

uninstall:
	rm -f ../sign_your_name/log_tail.cgi
	rm -f ../sign_your_name/view_logs_tail.cgi
	rm -f ../sign_your_name/add_log.cgi
	rm -f ../view_logs.cgi ../laundry_logs.cgi

mostlyclean:
	rm -f dumb_cgipm.o test_dumb_cgipm.o test_results.txt \
	  view_logs.o laundry_logs.o tmpl_log_tail.o log_tail.o \
	  view_logs_tail.o add_log.o

clean: mostlyclean
	rm -f $(bin_PROGRAMS)
