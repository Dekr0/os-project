target=		submit
allFiles=       Makefile commands.c handler.c msh379.c utils.c commands.h handler.h utils.h mMyclock.sh myclock.sh
scpDir=		chengxua@ub05.cs.ualberta.ca:~/379
# ------------------------------------------------------------


starter:  commands.c handler.c msh.c utils.c commands.h handler.h utils.h
	gcc -Wall commands.c handler.c msh.c utils.c -o msh

run:	msh
	msh

tar:
	touch $(target).tar.gz
	mv $(target).tar.gz  x$(target).tar.gz
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

scp:
	scp $(target).tar.gz $(scpDir)

clean:
	rm *~ out.* *.o
