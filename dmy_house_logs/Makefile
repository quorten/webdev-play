all:
	$(MAKE) -C fast install
	$(MAKE) -C gen_sign_your_name install

mostlyclean:
	$(MAKE) -C fast clean
	$(MAKE) -C gen_sign_your_name clean

clean: mostlyclean
	$(MAKE) -C fast uninstall
	$(MAKE) -C gen_sign_your_name uninstall
