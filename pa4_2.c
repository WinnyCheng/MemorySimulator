#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

int getvpn(int virtual_address){
	int vpn = (virtual_address & 48) >> 4; // 48 = 110000
	return vpn;
}

// return 1 if there is an empty page else return 0
int isEmpty(int* freelist){
	int full = 0;
	//CHECK FOR FREE SPACE OR NOT
	for(int i = 0; i < 4; i++){
		if(freelist[i] == 1)
			full++;
	}
	if(full < 4)
		return 1;
	return 0;
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
	if(ram[ramindex + vpn] == INT_MIN)
		return -1;
	return ram[ramindex + vpn] /10;
}

// if process not mapped return true aka 1 else return 0
int noProcess(int process_id, int *registers){
	if(registers[process_id] == -1)
		return 1;
	return 0;
}

int store(int process_id, int virtual_address, int value, int* ram, int* registers){
	if(noProcess(process_id, registers)){
		printf("Error: Process %d not mapped; no Page Table\n", process_id);
		return 0;
	}
	if(getpfn(process_id, virtual_address, ram, registers) == -1){
		printf("Error: page not mapped to memory\n");
		return 1;
	}
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

int load(int process_id, int virtual_address, int* ram, int* registers){
	if(noProcess(process_id, registers)){
		printf("Error: Process %d not mapped; no Page Table\n", process_id);
		return 0;
	}
	if(getpfn(process_id, virtual_address, ram, registers) == -1){
		printf("Error: page not mapped to memory\n");
		return 1;
	}
	int offset = (virtual_address & 15); //15 = 1111
	int physical = (getpfn(process_id, virtual_address, ram, registers) << 4) | offset;
	printf("The value %d is virtual address %d (physical address %d)\n", ram[physical], virtual_address, physical);
	return 0;
}

int map(int process_id, int virtual_address, int value, int* ram, int* freelist, int* registers){

	//check
	if(!isEmpty(freelist)){
		printf("Error: not enough memory\n");
		return 0;
	}


	int pfnP;
	// make page table
	if(registers[process_id] == -1){
		// no page table allocated for this process
		for(int pfn = 0; pfn < 4; pfn++){
			if (freelist[pfn] == 0){
				freelist[pfn] = 1;
				registers[process_id] = pfn;
				printf("Put page table for PID %d into physical frame %d\n", process_id, pfn);
				break;
			}
		}
	}

	//check
	if(!isEmpty(freelist)){
		printf("Error: not enough memory\n");
		return 0;
	}

	int pfnT = registers[process_id];
	// Stores process table info
	int ramindex = pfnT * 16;
	int vpn = getvpn(virtual_address);

	//check page table
	if(ram[ramindex + vpn] != INT_MIN){
	//if there - edit
		pfnP = ram[ramindex + vpn] / 10;
	}
	else{
	//if not there add page and store
		//store page of process to memory
		for(int i = 0; i < 4; i++){
			if(freelist[i] == 0){
				freelist[i] = 1;
				pfnP = i;		
				break;
			}
		}
	}
	
	ram[ramindex + vpn] = pfnP *10 + value; // page that we are trying to allocate
	printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtual_address, vpn, pfnP);

	return 0;
}

int main(){

	int ram[64];

	for(int j = 0; j < 64; j++){
		ram[j] = INT_MIN;
	}

	int freelist[4] = {0}; //marks at each physical page to see if its valid or not
	// 0 means is free, 1 means its used 
	int registers[4] = {-1, -1, -1, -1}; // points at each process PageTable, is indexed by process_id

	char *input, *tok;
	char **param;
	input = (char *)malloc(sizeof(char) * 100);
	param = (char **)malloc(sizeof(char *) * 4);

	int j = 0;
	int loop = 1;

	while(loop){

		printf("Insruction? (e to exit): \n");
		fgets(input, 100, stdin);

		if(strcmp(input, "e\n") == 0 || feof(stdin)){ //exit or end of file
			return 0;
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

		//printf("PID : %d; Type: %s; vAdd: %d; Value: %d\n", process_id, type, virtual_address, value);

		if(strcmp(type, "map") == 0)
			map(process_id, virtual_address, value, ram, freelist, registers);
		else if(strcmp(type, "store") == 0)
			store(process_id, virtual_address, value, ram, registers);
		else if(strcmp(type, "load") == 0)
			load(process_id, virtual_address, ram, registers);
		else
			printf("Error: Wrong type of instruction. Instruction types: map, store, load\n");

		j++;
	}


	// // map(process_id, virtual_address, value, ram, freelist, registers);
	// map(0, 50, 1, ram, freelist, registers);
	// printf("ram is %d\n", ram[3]);
	// map(0, 50, 0, ram, freelist, registers); // pfn 1, 111
	// printf("ram is %d\n", ram[3]);
	// map(1, 12, 1, ram, freelist, registers);
	// printf("ram is %d\n", ram[32]);
	// map(1, 12, 1, ram, freelist, registers);

	// // printf("freelist\n");
	// // for(int i = 0; i < 4; i++){
	// // 	printf("%d, ", freelist[i]);
	// // }		
	// printf("\n");
	// printf("\n");
	// printf("\n");

	// store(1, 12, 24, ram, registers);
	// load(1, 12, ram, registers);

	//example from intruction text
	//map(0, 0, 1, ram, freelist, registers);
	//store(0, 12, 24, ram, registers);
	//load(0, 12, ram, registers);



	// printf("registers is %d\n", registers[0]);
	// printf("registers is %d\n", registers[1]);
	// printf("ram is %d\n", ram[2]);
	// printf("ram is %d\n", ram[18]);	

	free(input);
	free(param);

	return 0;
}