#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
    int totalBytes = 0; // amount of bytes copied to file | -1 if any byte were copied or an error occurs
    int outputByte = 0; // Byte written to destination file.
    int readByte = 0;  // Byte readen from the origin file

    // Documentation http://www.tutorialspoint.com/c_standard_library/c_function_ferror.htm

    if (origin == NULL) { return -1; } // Origin file doesn't exit. 0 characters copied
    
    // In the loop we copied each byte from origin to destination.
    
    while((totalBytes < nBytes) && (readByte = getc(origin)) != EOF) { // && (ferror(origin) == 0)) {
        if ((ferror(origin) != 0)) { return -1; } // There is an erro
        outputByte = putc(readByte, destination);
        totalBytes++;
    }

    return totalBytes;
    
    if (totalBytes == 0 || ferror(origin) != 0) {
        totalBytes = -1; // automatically returns -1 cause we couldn't copy all the bytes or there was an error reading a specific byte
        if (ferror(origin) != 0) { // error reading from file
            clearerr(origin); // Clean the error occured when reading from file
        }
    }
    
    return totalBytes;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * buf: parameter to return the read string. Buf is a
 * string passed by reference. 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly "built" in memory, return the starting 
 * address of the string (pointer returned by malloc()) in the buf parameter
 * (*buf)=<address>;
 * 
 * Returns: 0 if success, -1 if error
 */
int loadstr(FILE * file, char **buf) {
    int filenameLength = 0, index = 0;
    char bit;
    char *name;

    while((bit = getc(file) != '\0')) {
        filenameLength++;
        if (bit == 0) {}
    }

    name =  malloc(sizeof(char) * (filenameLength + 1)); // +1 for the \0 character
    fseek(file, -(filenameLength + 1), SEEK_CUR);

    for (index = 0; index < filenameLength+1; index++) {
        name[index] = getc(file);
        putc(name[index],stdout);
    }    

    printf("The string is %s\n", name);
    (*buf)=name; // (*buf)=<address>
    return 0;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * header: output parameter. It is used to return the starting memory address
 * of an array that contains the (name,size) pairs read from the tar file
 * nFiles: output parameter. Used to return the number of files stored in
 * the tarball archive (first 4 bytes of the header)
 *
 * On success it returns EXIT_SUCCESS. Upon failure, EXIT_FAILURE is returned.
 * (both macros are defined in stdlib.h).
 */
int
readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles)
{
    int nr_files = 0, index = 0; // Return parameter
    stHeaderEntry* array=NULL; // header for creating heap space
    stHeaderEntry *auxHeader; // header for creating heap space
    
    fread( &nr_files, sizeof(int), 1, tarFile); // read the number of files of a mtar
    *nFiles = nr_files; // Return the number of files for the mtar

    /* Allocate memory for the array */
    array=malloc(sizeof(stHeaderEntry)*nr_files);
    auxHeader = array; // used for traverse the array of structs
    
    //char endLine;
    //fread(&endLine, sizeof(char), 1, tarFile); // get '/0'

    // Read the (pathname,size) pairs from tarFile and store them in the array
    for (index = 0; index < nr_files; index++) {
        int size = 0;
        int nameReaded = loadstr(tarFile, &auxHeader->name); // Set axuHeader name - auxHeader->name = fileNames[index];
    
        if (nameReaded == 0) { printf("File name: %s \n", auxHeader->name); }
        fread(&size, sizeof(unsigned int), 1, tarFile); // Set size of the file in the structure
        auxHeader->size = size;
        printf("Size %d\n", auxHeader->size);

        auxHeader++;
    }
    
    (*nFiles)=nr_files;
    (*header)=array;

    return (EXIT_SUCCESS);
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */

int
createTar(int nFiles, char *fileNames[], char tarName[])
{
    int created = EXIT_SUCCESS; // returned variable
    int index = 0, copiedBytes = 0, closedFile = 0;
    int nbytes;

    FILE * destination = fopen(tarName, "w"); // Tarbar file to write the header and data
    
    // OBJETIVE: write in the tarbar file the header (filename+size) and the data for each file
    // First calculate the size of the header (filename+size) for all the struct
    
    stHeaderEntry *stHeader = malloc(sizeof(stHeaderEntry) * nFiles); 
    
    nbytes=nFiles*sizeof(unsigned int)+sizeof(int);

    for (index=0; index < nFiles; index++)
        nbytes+=strlen(fileNames[index])+1;

    fseek(destination, nbytes, SEEK_SET); 


    // Now we have to copy each file content to the TarBar file
    // that's will tell me the size of each file (so we have to update struct size on heap)
    // Copy each origin file to destination file - copying byte by byte
    
    for (index=0; index < nFiles; index++) {
        
        FILE * origin = fopen(fileNames[index], "r");
        copiedBytes = copynFile(origin, destination, INT_MAX); // destination is the tarbar file

        stHeader[index].size = copiedBytes;
        stHeader[index].name = malloc(sizeof(fileNames[index]) + 1); // Heap space for the name char + '\0' character
        strcpy(stHeader[index].name, fileNames[index]); // Copy one string to the struct entry

        // printf("stHeader->name is %s\n", stHeader->name);
        // printf("stHeader->number is %d\n", stHeader->size);
        // printf("stHeader position is %p\n", stHeader);

        if (fclose(origin) == EOF) return EXIT_FAILURE; // Try to close the file
    }
        
    if (fseek(destination, 0, SEEK_SET) != 0) return EXIT_FAILURE;
    // else: // pointer at the beginning of the file to write the header

    fwrite(&nFiles, sizeof(int), 1, destination); // write number of files in .mtar
    
    for (index = 0; index < nFiles; index++) {
        // Write header for each struct
        // Documentation: http://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
        fwrite(stHeader[index].name, strlen(stHeader[index].name)+1, 1, destination);
        fwrite(&stHeader[index].size, sizeof(unsigned int), 1, destination);
        printf("%d\n", stHeader[index].size);
    }
  
    
    closedFile = fclose(destination); // closed the reading file
    
    if (closedFile == EOF) { 
        created = EXIT_FAILURE;
        // printf ("Error closing file"); 
    } 
    
    // Free the memory
    free(stHeader);
    
    return created;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
    int headerRead = -1, nr_files = 0, index = 0;
    FILE *tarFile = fopen(tarName, "r");
    stHeaderEntry * header;

    headerRead = readHeader(tarFile, &header, &nr_files);
    
    if (headerRead == -1) { return (EXIT_FAILURE); }

    int long offset = sizeof(stHeaderEntry)*nr_files;

    fseek(tarFile, offset, SEEK_SET); // skip the header on file

    for (index = 0; index < nr_files; index++) {
        int totalBytes = 0, nBytes = header->size, readByte;
        while((totalBytes < nBytes) && ((readByte = getc(tarFile)) != EOF)) {
            putc(readByte,stdout);
            totalBytes++;
        }
        header++;
    }

	return (EXIT_SUCCESS);
}
