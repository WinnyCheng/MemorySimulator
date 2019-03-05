#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

int swapIn(int process_id, int virtual_address, int* ram, int* freelist, int* registers);

int getvpn(int virtual_address){
	int vpn = (virtual_address & 48) >> 4; // 48 = 110000
	return vpn;
}

// return 1 if there is an empty page else return 0
int isEmpty(int* freelist){
	int full = 0;
	//CHECK FOR FREE SPACE OR NOT
	for(int i = 0; i < 4; i++){
		if(freelist[i] != 0)
			full++;
	}
	if(full < 4)
		return 1;
	return 0;
}
//1 means in physical memory, 0 means not in memory
int getpresent(int process_id, int virtual_address, int* ram, int* registers){
	int pfnT = registers[process_id];
	int ramindex = pfnT*16;
	int vpn = getvpn(virtual_address);
	return ram[ramindex + vpn] /100; 

}

int getprotection(int process_id, int virtual_address, int* ram, int* registers){
	int pfnT = registers[process_id];
	int ramindex = pfnT*16;
	int vpn = getvpn(virtual_address);
	return ram[ramindex + vpn] % 2; 

}

int getpfn(int process_id, int virtual_address, int* ram, int* registers){
	int pfnT = registers[process_id];
	int ramindex = pfnT*16;
	int vpn = getvpn(virtual_address);
	if(ram[ramindex + vpn] == -1)
		return -1;
	return ram[ramindex + vpn] % 100 / 10;
}

// if process not mapped return true aka 1 else return 0
int noProcess(int process_id, int *registers){
	if(registers[process_id] == -1)
		return 1;
	return 0;
}

int count = 0;
int RR = 0; // roundRobin counter

int swapOut(int process_id,int virtual_address, int* ram, int* freelist, int* registers){
	FILE *fd;
	char *str;

	str = (char *)malloc(sizeof(char) * 100);

	strcpy(str, "");
    // open file for writing
    fd = fopen ("swapSpace.txt", "a");
    if (fd == NULL){
        fprintf(stderr, "\nError opend file\n");
        exit (1);
    }

    for(int r = 0; r < 4; r++){
    	if(registers[r] == RR){ // if round robin is going to take a a PT
    		if(r == process_id){
				RR++;
    		 	if(RR > 3){
			 		RR = 0;
 				}
    		}
    	}
    }

    // if taking out a PT, dont do anything
    // if taking out a Page, update PT
    // check if PT of evicting page in memory
    	// if not swap page back in
    // find page table
    // find current vpn
    // update present bit 
    int pid;
    int index;
    int temp;
    int vpn;
    //printf("%d\n", freelist[RR]);

    if(freelist[RR]/10 != 4){
    	// page hit!
		pid = freelist[RR] % 100 / 10 ; // pid of evicting page
		vpn = freelist[RR] % 10;

    	if(registers[pid] == -1 && virtual_address != -1){
    		// if PT not in memory, swap back in
    		swapIn(pid, -1, ram, freelist, registers);
    	}

    	index = registers[pid];
    	temp = ram[index*16 + vpn];
    	ram[index*16 + vpn] = temp % 100;
    }
    else{
    	pid = freelist[RR] % 10;
    }

    //printf("%d %d\n", registers[pid], RR);

    if(registers[pid] == RR)
    	registers[pid] = -1;

    char buf[5];
	int value = freelist[RR];
	snprintf(buf, 10, "%d", value);
    strcat(str, buf);
    strcat(str, " ");
    //printf("my free list is %d\n", freelist[RR]);
    freelist[RR] = 0;

 	//printf("%s\n", str);
 	char buffer[20];

 	for(int i = 0; i < 15; i++){
		value = ram[RR*16 + i];
		snprintf(buffer, 10, "%d", value);

		//printf("%s\n", buffer);

 		strcat(buffer, " ");
 		strcat(str, buffer);

 		ram[RR*16 + i] = -1;
 	}

	value = ram[RR*16 + 15];
	snprintf(buffer, 10, "%d", value);

	//printf("%s\n", buffer);

 	strcat(str, buffer);

 	ram[RR*16 + 15] = -1;

 	//printf("%s\n", str);
 	strcat(str, "\n");
    //strcat(str, "\0");
 	fwrite(str, sizeof(char), strlen(str), fd);
 	// printf("%d\n", i);
 	fclose(fd);

 	printf("Swap frame %d to disk at swap slot %d\n", RR, count);

 	count++;
 	int rr = RR; //round robin for swap in
 	RR++;
 	if(RR > 3){
 		RR = 0;
 	}
	return rr;
}

int swapIn(int process_id, int virtual_address, int* ram, int* freelist, int* registers){

	// check for page table of current process in freelist
	// if not in memory swap into memory
	// update register
	// if not in memory swap error and return ------------------------- continue
	// if in memory, do nothing


	FILE *fd;
	char *str;
	char * name = "swapSpace.txt";
	char *line;
	str = (char *)malloc(sizeof(char) * 100);
	line = (char *)malloc(sizeof(char) * 100);
	strcpy(line, "");
	strcpy(str, "");



    int c = 0;
    int inSwap = 0;
	int index, k;
	char* tok;

    //check if in memory
	if(registers[process_id] == -1){
	    // open file for reading
	    fd = fopen (name, "r");
	    if (fd == NULL){
	        fprintf(stderr, "\nError opend file\n");
	        exit (1);
	    }
		// read from swap space
		while(fgets(str, 100, fd) != NULL){
			//check first character
			k = 0;
			tok = strtok(str, " ");
			//printf("%s", tok);
			index = atoi(str);
			//printf("%d\n", index);

			if (index == 40 + process_id){
				// page table hit!
				int rr = swapOut(process_id,virtual_address , ram, freelist, registers);
				registers[process_id] = rr;
				//update freelist
				freelist[rr] = index;

				// store the page table string into the ram
				tok = strtok(NULL, " ");
				while(tok != NULL){
				 	ram[rr * 16 + k] = atoi(tok);
				 	tok = strtok(NULL, " ");
				 	k++;
				}	

				/*printf("ram ");				
				for(int a = 0; a < 15; a++){
					printf("%d ", ram[rr*16 + a]);
				}
				printf("%d\n", ram[rr*16+15]);*/

				printf("Swap disk slot %d to frame %d\n", c, rr);
				inSwap = 1;
				break;
			}
			c++;
		}
		fclose(fd);

		if(!inSwap){
			printf("ERROR: Page table not allocated\n");
			return -1;
		}

		FILE *newfile;
	    newfile = fopen ("swapSpace3.txt", "a");
	    if (newfile == NULL){
	        fprintf(stderr, "\nError opend file\n");
	        exit (1);
	    }

	    fd = fopen (name, "r");
    	if (fd == NULL){
        	fprintf(stderr, "\nError opend file\n");
        	exit (1);
    	}

		while(fgets(str, 100, fd) != NULL){
			strcpy(line, str);
			tok = strtok(str, " ");
			index = atoi(str);
			if(index != 40 + process_id)
				fwrite(line, sizeof(char), strlen(line), newfile);
		}

		fclose(fd);
		fclose(newfile);
		count--;
		remove(name);
		rename("swapSpace3.txt", name);
	}




    strcpy(line, "");
	strcpy(str, "");

	// check inside of page table for present bit of needed page
	// if not in memory, check in swap space
	// update present bit and update pfn (inside page table)
	// read swap space (how)
	// if not in swap space print error
	// if in memory, do nothing
	int vpn = getvpn(virtual_address);
	inSwap = 0;

	c = 0;

	//check if page is in memory
	if(getpresent(process_id, virtual_address, ram, registers) == 0 && virtual_address != -1){

		fd = fopen (name, "r");
	    if (fd == NULL){
	        fprintf(stderr, "\nError opend file\n");
	        exit (1);
	    }

		//check swap space for page
		while(fgets(str, 100, fd) != NULL){
			//check first character
			//printf("%s\n", str);
			k = 0;
			//printf("%s\n", line);
			tok = strtok(str, " ");
			//printf("%s", tok);
			index = atoi(str);
			//printf("%d\n", index);

			if (index == 100 + process_id*10 + vpn){
				// page hit!
				int rr = swapOut(process_id, virtual_address, ram, freelist, registers);
				//update PT
				int t = ram[registers[process_id]*16 + vpn];
				ram[registers[process_id]*16 + vpn] = 100 + rr*10 + t % 2;
				// store the page table string into the ram
				tok = strtok(NULL, " ");
				while(tok != NULL){
				 	ram[rr * 16 + k] = atoi(tok);
				 	tok = strtok(NULL, " ");
				 	k++;
				}	

				/*printf("ram ");				
				for(int a = 0; a < 15; a++){
					printf("%d ", ram[rr*16 + a]);
				}
				printf("%d\n", ram[rr*16+15]); */
				printf("Swap disk slot %d to frame %d\n", c, rr);
				inSwap = 1;
				break;
			}
			c++; 
		}
		fclose(fd);

		if(!inSwap) {
			printf("ERROR: Page not allocated\n");
			return -1;
		}

	    FILE *newfile2;
	    newfile2 = fopen ("swapSpace4.txt", "a");
	    if (newfile2 == NULL){
	        fprintf(stderr, "\nError opend file\n");
	        exit (1);
	    }

	    fd = fopen (name, "r");
    	if (fd == NULL){
        	fprintf(stderr, "\nError opend file\n");
        	exit (1);
    	}

	    while(fgets(str, 100, fd) != NULL){
			strcpy(line, str);
			tok = strtok(str, " ");
			index = atoi(str);
			if(index != 100 + process_id*10 + vpn)
				fwrite(line, sizeof(char), strlen(line), newfile2);
		}
		fclose(fd);
		fclose(newfile2);
		count--;
		remove(name);
		rename("swapSpace4.txt", name);
	}

	//remove("swapSpace3.txt");
	//remove("swapSpace4.txt");


	free(str);
	free(line);

	return 0;
}

int store(int process_id, int virtual_address, int value, int* ram,int* freelist, int* registers){
	// if(noProcess(process_id, registers)){
	// 	printf("Error: Process %d not mapped; no Page Table\n", process_id);
	// 	return 0;
	// }
	// if(getpfn(process_id, virtual_address, ram, registers) == -1){
	// 	printf("Error: page not mapped to memory\n");
	// 	return 1;
	// }

	int e = swapIn(process_id, virtual_address, ram, freelist, registers);

	if(e == -1)
		return 0;

	if(getprotection(process_id, virtual_address, ram, registers) == 0){
		printf("ERROR: No permission to write\n");
		return 1;
	}else{
		int offset = (virtual_address & 15); //15 = 1111
		int physical = (getpfn(process_id, virtual_address, ram, registers) << 4) | offset;
		ram[physical] = value;
		printf("Stored value %d at virtual address %d (physical address %d)\n", ram[physical], virtual_address, physical);
		return 0;
	}
}

int load(int process_id, int virtual_address, int* ram, int* freelist, int* registers){
	// if(noProcess(process_id, registers)){
	// 	printf("Error: Process %d not mapped; no Page Table\n", process_id);
	// 	return 0;
	// }
	// if(getpfn(process_id, virtual_address, ram, registers) == -1){
	// 	printf("Error: page not mapped to memory\n");
	// 	return 1;
	// }

	int e = swapIn(process_id, virtual_address, ram, freelist, registers);

	if(e == -1)
		return 0;

	int offset = (virtual_address & 15); //15 = 1111
	int physical = (getpfn(process_id, virtual_address, ram, registers) << 4) | offset;
	if(ram[physical] == -1)
		printf("Nothing stored at this address\n");
	else
		printf("The value %d is virtual address %d (physical address %d)\n", ram[physical], virtual_address, physical);
	return 0;
}

int map(int process_id, int virtual_address, int value, int* ram, int* freelist, int* registers){

	int pfnP;
	// make page table
	if(registers[process_id] == -1){
		//check if full swap, make room for PT
		if(!isEmpty(freelist)){
			swapOut(process_id, virtual_address, ram, freelist, registers); // update this
		}
		// no page table allocated for this process
		for(int pfn = 0; pfn < 4; pfn++){
			if (freelist[pfn] == 0){
				freelist[pfn] = 40 + process_id;
				registers[process_id] = pfn;
				printf("Put page table for PID %d into physical frame %d\n", process_id, pfn);
				break;
			}
		}
	}

	int pfnT = registers[process_id];
	// Stores process table info
	int ramindex = pfnT * 16;
	int vpn = getvpn(virtual_address);

	//check page table
	if(ram[ramindex + vpn] != -1){
	//if there - edit
		pfnP = ram[ramindex + vpn] % 100 / 10;
		int p = getprotection(process_id, virtual_address, ram, registers);
		if(p != value)
			printf("Updating permissions for virtual page %d (frame %d)\n", vpn, pfnP);
		else
			printf("Error: vitual page %d is alread mapped with rw_bit = %d\n", vpn, p);
	}
	else{
	//if not there add page and store
		//check if full swap, make room for page
		if(!isEmpty(freelist)){
			swapOut(process_id, virtual_address, ram, freelist, registers);
		}
		//store page of process to memory
		for(int i = 0; i < 4; i++){
			if(freelist[i] == 0){
				freelist[i] = 100 + process_id * 10 + vpn;
				pfnP = i;
				printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtual_address, vpn, pfnP);		
				break;
			}
		}
	}
	
	ram[ramindex + vpn] = 100 + pfnP *10 + value; // page that we are trying to allocate
	return 0;
}

void printRam(int *ram){
	printf("ram0 ");				
	for(int a = 0; a < 15; a++){
		printf("%d ", ram[a]);
	}
	printf("%d\n", ram[15]);

	printf("ram1 ");				
	for(int a = 0; a < 15; a++){
		printf("%d ", ram[16 + a]);
	}
	printf("%d\n", ram[31]);

	printf("ram2 ");				
	for(int a = 0; a < 15; a++){
		printf("%d ", ram[32 + a]);
	}
	printf("%d\n", ram[46]);

	printf("ram3 ");				
	for(int a = 0; a < 15; a++){
		printf("%d ", ram[48 + a]);
	}
	printf("%d\n", ram[63]);
}


int main(){

	int ram[64];

	for(int j = 0; j < 64; j++){
		ram[j] = -1;
	}

	int freelist[4] = {0}; //marks at each physical page to see if its valid or not
	// 0 means is free, 1 means its used 
	// if page table 40 to 43 
	// if page 100 to 133
	int registers[4] = {-1, -1, -1, -1}; // points at each process PageTable, is indexed by process_id

	char *input, *tok;
	char **param;

	input = (char *)malloc(sizeof(char) * 100);
	param = (char **)malloc(sizeof(char *) * 4);

	int j = 0;
	int loop = 1;

	//swapIn(0,0,ram, freelist, registers);

	//printRam(ram);

	while(loop){

		printf("Instruction? (e to exit): \n");
		fgets(input, 100, stdin);

		if(strcmp(input, "e\n") == 0 || feof(stdin)){ //exit or end of file
			break;
		}

		int k = 0;

		tok = strtok(input, ",");
		while(tok != NULL){
			param[k] = tok;
			tok = strtok(NULL, ",");
			k++;
		}

		int process_id = atoi(param[0]);
		char *type = param[1];
		int virtual_address = atoi(param[2]);
		int value = atoi(param[3]);

		if(value > 255 || value < 0){
			printf("ERROR: Value cannot be bigger than 255 or negative\n");
		} else if (virtual_address > 63 || virtual_address < 0){
			printf("ERROR: virtual_address cannot be bigger than 63 or negative\n");
		} else{
			if(strcmp(type, "map") == 0){
				if(value > 1)
					printf("Error: Protection bit only takes value 0 or 1\n");
				else{
					map(process_id, virtual_address, value, ram, freelist, registers);
				}
			}
			else if(strcmp(type, "store") == 0)
				store(process_id, virtual_address, value, ram, freelist, registers);
			else if(strcmp(type, "load") == 0)
				load(process_id, virtual_address, ram, freelist, registers);
			else
				printf("Error: Wrong type of instruction. Instruction types: map, store, load\n");
		}

		//printf("PID : %d; Type: %s; vAdd: %d; Value: %d\n", process_id, type, virtual_address, value);


		j++;
	}

	// printRam(ram);	
	remove("swapSpace.txt");
	free(input);
	free(param);

	return 0;
}