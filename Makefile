#NAME: Zicong Mo, Benjamin Yang
#ID: 804654167, 904771533
#EMAIL: josephmo1594@ucla.edu, byang77@ucla.edu

default:
	gcc -Wall -Wextra lab3a.c -o lab3a 
clean:
	rm -f lab3a lab3a-804654167.tar.gz
dist:
	tar -xvcf lab3a-804654167.tar.gz Makefile README ext2_fs.h lab3a.c