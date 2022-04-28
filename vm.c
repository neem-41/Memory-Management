/**
 * Demonstration C program illustrating how to perform file I/O for vm assignment.
 *
 * Input file contains logical addresses
 * 
 * Backing Store represents the file being read from disk (the backing store.)
 *
 * We need to perform random input from the backing store using fseek() and fread()
 *
 * This program performs nothing of meaning, rather it is intended to illustrate the basics
 * of I/O for the vm assignment. Using this I/O, you will need to make the necessary adjustments
 * that implement the virtual memory manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// number of characters to read for each line from input file
#define BUFFER_SIZE         10

// number of bytes to read
#define CHUNK               256

FILE    *address_file;
FILE    *backing_store;

// how we store reads from input file
char    address[BUFFER_SIZE];

int     logical_address;

int msize = 128;
// the buffer containing reads from backing store
signed char     buffer[128][CHUNK];

// the value of the byte (signed char) in memory
signed char     value;

int pageTable[256];
int TLB[16][2];

int main(int argc, char *argv[])
{
	int i = 0;
	int k  = 0;
	int buffer_check = 0;
	int offset;
	int page_number;

	int tlb_hit = 0;
	int tlb_miss = 0;

	int page_miss = 0;	

	const int mask = 0xff;
	
	// set page table to -1 at start
	for(int j = 0; j < 256; j++) {
		pageTable[j] = -1;
	}
	
	// set TLB to -1 at start
	for(int j = 0; j < 16; j++) {
		TLB[j][0] = -1;
	}

    // perform basic error checking
    if (argc != 3) {
        fprintf(stderr,"Usage: ./vm [backing store] [input file]\n");
        return -1;
    }

    // open the file containing the backing store
    backing_store = fopen(argv[1], "rb");
    
    if (backing_store == NULL) { 
        fprintf(stderr, "Error opening %s\n",argv[1]);
        return -1;
    }

    // open the file containing the logical addresses
    address_file = fopen(argv[2], "r");

    if (address_file == NULL) {
        fprintf(stderr, "Error opening %s\n",argv[2]);
        return -1;
    }

    // read through the input file and output each logical address
    while ( fgets(address, BUFFER_SIZE, address_file) != NULL) {
	// Getting page numbers and offset.
	logical_address = atoi(address);
        int la = logical_address;
	offset = logical_address & mask;
	logical_address = logical_address >> 8;
	page_number = logical_address & mask;

	//boolean for TLB.
	int check = 0;
	int frame;

	//printf("pn: %d, of: %d\n", page_number, offset);
	
	//check TLB first.
	for (int j = 0; j < 16; j++) {
		if (TLB[j][0] == page_number) {
			check = 1;
			tlb_hit++;
			frame = TLB[j][1];
			break;
		}
	}

	// Look for page fault only if not found in TLB.
	if (check != 1) {
		tlb_miss++;
		if (pageTable[page_number] == -1) {
			page_miss ++;
        		// first seek to byte CHUNK in the backing store
        		// SEEK_SET in fseek() seeks from the beginning of the file
        		if (fseek(backing_store, CHUNK * page_number, SEEK_SET) != 0) {
            			fprintf(stderr, "Error seeking in backing store\n");
       	     			return -1;
       		 	}
	
      		 	// now read CHUNK bytes from the backing store to the buffer
       		 	if (fread(buffer[i], sizeof(signed char), CHUNK, backing_store) == 0) {
		            fprintf(stderr, "Error reading from backing store\n");
		            return -1;
			}
			else {
				if (buffer_check > msize) {
					for (int p=0; p < 256; p++) {
						if (pageTable[p] == i) {
							pageTable[p] = -1;
						}
					}
					for (int p =0; p < 16; p++) {
						if (TLB[p][1] == i) {
							TLB[p][0] = -1;
						}

					}
				}
				pageTable[page_number] = i;
				i = (i + 1) % msize;
				buffer_check++;
			}

	        }
	
		frame = pageTable[page_number];
		TLB[k][0] = page_number;
		TLB[k][1] = frame;
		k = (k + 1) % 16;
	}

	value = buffer[frame][offset];


         
        printf("Virtual address: %d Physical Address: %d Value: %d\n", la, (frame << 8)+offset, value);
    }

  	printf("tlb_hit_rate: %f\n", (double)tlb_hit * 100/(tlb_hit+tlb_miss));
	printf("page_fault_rate: %f\n", (double)page_miss * 100/1000);	

    fclose(address_file);
    fclose(backing_store);

    return 0;
}


