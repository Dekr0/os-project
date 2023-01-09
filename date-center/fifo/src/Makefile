target=		submit
allFiles=       Makefile handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp const.h handler.h master.h message.h rule.h tor.h utils.h ex1.dat ex2.dat ex3.dat
scpDir=		chengxua@ug05.cs.ualberta.ca:~/379
# ------------------------------------------------------------


starter:  handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp const.h handler.h master.h message.h rule.h tor.h utils.h
	g++ -Wall -pthread handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp -o network 

tar:
	touch $(target).tar.gz
	mv $(target).tar.gz  x$(target).tar.gz
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

s1:
	./network psw1 ex3.dat null psw2 100-110

s2:
	./network psw2 ex3.dat psw1 null 200-210
	
master:
	./network master 2

scp:
	scp $(target).tar.gz $(scpDir)

clean:
	find ./ -name "fifo*" -delete
	rm network
