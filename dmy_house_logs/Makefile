all: dmy_house_logs.log
	$(MAKE) -C fast install
	$(MAKE) -C gen_sign_your_name install

# Create an empty log file for development.
dmy_house_logs.log:
	touch $@
	chmod go+rw $@

docker-up:
	sudo docker run -dit --name dmy_house_logs -p 8080:80 \
	  -v $$PWD:/usr/local/apache2/htdocs/ \
	  -v $$PWD/docker-httpd.conf:/usr/local/apache2/conf/httpd.conf \
	  httpd:2.4

docker-down:
	sudo docker stop dmy_house_logs
	sudo docker rm dmy_house_logs

mostlyclean:
	$(MAKE) -C fast clean
	$(MAKE) -C gen_sign_your_name clean

clean: mostlyclean
	$(MAKE) -C fast uninstall
	$(MAKE) -C gen_sign_your_name uninstall
