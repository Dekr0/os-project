target=		submit
allFiles=       Makefile handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp const.h handler.h master.h message.h rule.h tor.h utils.h main.h ex1.dat ex2.dat ex3.dat
scpDir=		chengxua@ug05.cs.ualberta.ca:~/379
portNumber = 9060
serverName = ug05.cs.ualberta.ca
# ------------------------------------------------------------


starter:  handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp const.h handler.h master.h message.h rule.h tor.h utils.h
	g++ -Wall -pthread handler.cpp main.cpp master.cpp message.cpp rule.cpp tor.cpp utils.cpp -o a3w22

tar:
	touch $(target).tar.gz
	mv $(target).tar.gz  x$(target).tar.gz
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

s1:
	./a3w22 psw1 ex3.dat null psw2 100-110 $(serverName) $(portNumber)

s2:
	./a3w22 psw2 ex3.dat psw1 null 200-210 $(serverName) $(portNumber)
	
master:
	./a3w22 master 1 $(portNumber)

scp:
	scp $(target).tar.gz $(scpDir)

clean:
	find ./ -name "fifo*" -delete
	rm a3w22
