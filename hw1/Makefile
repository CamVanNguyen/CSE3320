submission:
	tar -c *.txt | gzip -n > $(USER).tgz

submit:
	make submission
	sudo /usr/local/share/CSE3320/scripts/submit.py -a $(assignment) -i $(USER).tgz -u $(USER)

verify:
	make submission
	sudo /usr/local/share/CSE3320/scripts/verify.py -a $(assignment) -i $(USER).tgz -u $(USER)
