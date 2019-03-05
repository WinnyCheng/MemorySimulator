Ching Wing Cheng - ccheng2
Jose Li - jaliquiel 
PROJECT 4

1- How to run
Go into the folder and do "make". Then do "./pa4".
The program will wait for instructions formated in the following way:
"0,map,7,1"

The first parameter would be the process id. 
The second parameter would be the instruction you want to run. Which are:
	- map (allocate in memory a virtual page)
	-load (get a value from a page)
	-store (store inside a page)
The third parameter will be the virtual address. 
The fourth parameter will be the value.


2- Files
The files included are:
- pa4.c
- Makefile

- input(.txt) files
These files contain test cases.
Input.txt contains the example provided in phase 2.

- output(.txt) files
These files contain output from our three test cases.
output.txt parameters: input.txt
output2.txt parameters: input2.txt
output3.txt parameters: input3.txt

- swapSpace(.txt) files
These files contain our swap Space file.


3- Page table entries
The first 4 bits of the page are used, each respectively represent the virtual page they represent.
In our page table we represent each page as a three digit number: (ex. 111)
The 100th digit represents our present bit which can be 1 (in memory) or 0 (in swap space).
The 10th digit represents our pfn.
The 1th digit represents our protection bit. (if 1 page can be edited, and 0 page cannot be edited)

4- Swap Space storage
We are implementing a text file as our swap space. Our funtions, swapIn() takes a page out of memory 
and stores it in the swap space and swapOut() takes a page out of swap spaces and stores it in memory.
We store each page as a string with its values. At the beginnin of our string we add an ID each page.

ID for Page Tables: two digit integer. 
The 10th digit is always 4.
The 1th digit is the process id, the table corresponds to.

ID for Pages: three digit integer.
The 100th digit is always 1.
The 10th digit is the process id.
The 1th digit is the vpn.

5- Swap Space Inserting and Deleting
Our swap space pages are represented by lines. When we swap in a page, we remove the line and
we shift the rest of the pages so that there are no empty spaces between the data.

6 - Freelist
We have an array called freelist[4] which contains information of what is stored in physical memory.
For example if pfn 1 contains a Page table of process 1, the free list in space [1] will contain the number
"41". If the pfn 3 contains a Page with vpn 1 of process 0, the free list in space [3] will contain the following
ID number "101"
An empty spot is represented with an 0.

7- Registers
// everything is -1
Our registers is an array of [4] each one respectevely representing each process' page table. If there is no page table
in physical memory for an current process, the value will be -1.

8 - Eviction Policy
Our eviction policy is Round Robin. We have the exception that when we do not evict the current process' page table, but 
any other page table can be evicted.