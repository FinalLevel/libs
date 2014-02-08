PRJ_ROOT=.

all::
# clean
	cd tests; make

clean::
	find . -name "*.core" -exec rm {} \;
	find . -name "*.d" -exec rm {} \;
	find . -name "*.o" -exec rm {} \;

update::
	git status
	git pull origin master

