target=		li-a4
src = main.cpp resource_manager.cpp simulator.cpp utils.cpp
header = main.h resource_manager.h simulator.h utils.h
allFiles = Makefile report.pdf jobs.txt $(src) $(header)
scpDir=		chengxua@ug05.cs.ualberta.ca:~/379
portNumber = 9060
serverName = ug05.cs.ualberta.ca
monitor = 250
iteration = 20
core = 6
# ------------------------------------------------------------


concurrent-sim: $(src) $(header)
	g++ -Wall -pthread $(src) -o concurrent-sim

tar:
	touch $(target).tar.gz
	mv $(target).tar.gz  x$(target).tar.gz
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

run:
	taskset -c 0-$(core) ./concurrent-sim jobs.txt $(monitor) $(iteration) >> output.txt

scp:
	scp $(target).tar.gz $(scpDir)

clean:
	rm concurrent-sim 
	rm *.gz
	rm output*.txt
