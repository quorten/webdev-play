CC = cc
CFLAGS =
LDFLAGS =
bin_PROGRAMS = autopay tlog.cgi create_account.cgi payment_form.cgi \
	autopay_form.cgi end_autopay_form.cgi \
	grpbuy_form.cgi grpsell_form.cgi view_logs.cgi

all: $(bin_PROGRAMS)

autopay: autopay.c
	cc -o autopay autopay.c

tlog.cgi: tlog.c xmalloc.c
	cc -o tlog.cgi tlog.c xmalloc.c

dumb_cgipm.o: dumb_cgipm.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

create_account.o: create_account.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

create_account.cgi: create_account.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

payment_form.o: payment_form.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

payment_form.cgi: payment_form.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

autopay_form.o: autopay_form.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

autopay_form.cgi: autopay_form.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

end_autopay_form.o: end_autopay_form.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

end_autopay_form.cgi: end_autopay_form.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

grpbuy_form.o: grpbuy_form.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

grpbuy_form.cgi: grpbuy_form.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

grpsell_form.o: grpsell_form.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

grpsell_form.cgi: grpsell_form.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

view_logs.o: view_logs.c dumb_cgipm.h
	$(CC) $(CFLAGS) -c $<

view_logs.cgi: view_logs.o dumb_cgipm.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f autopay tlog.cgi create_account.cgi \
	  payment_form.cgi autopay_form.cgi end_autopay_form.cgi \
	  grpbuy_form.cgi grpsell_form.cgi view_logs.cgi
	rm -f dumb_cgipm.o create_account.o payment_form.o \
	  autopay_form.o end_autopay_form.o \
	  grpbuy_form.o grpsell_form.o view_logs.o
