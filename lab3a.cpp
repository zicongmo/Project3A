#include <stdio.h>
#include <iostream> 
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

int main(int argc, char** argv){
	if(argc != 2){
		fprintf(stderr, "Error: Must include file system image\n");
		exit(1);
	}

	exit(0);
}
