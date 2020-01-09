all: 
	gcc -o MakeTencentDeb *.c

.PHONY:clean
clean:
	rm -rf MakeTencentDeb
