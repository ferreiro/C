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
    
    while((totalBytes < nBytes) && ((readByte = getc(origin)) != EOF) && (ferror(origin) == 0)) {
        outputByte = putc(readByte, destination);
        totalBytes++;
    }
    
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
int
loadstr(FILE * file, char **buf)
{
    int valid = 0; // 0 = valid | 1 = not valid
    int filenameLength = 0; // len
    char bit;
    char *name; // array of characters. must be returned though buf
    char auxName[100];
    
    while((bit = getc(file)) != '\0') {
        filenameLength++;
        auxName[filenameLength] = bit; // save bit in aux array
    }
    
    name = malloc(sizeof(char) * filenameLength + 1); // 1 = last "\0"
    name = auxName;
    
    (*buf)=name; // (*buf)=<address>
    
	return valid;
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
    
    // Read the (pathname,size) pairs from tarFile and store them in the array
    for (index = 0; index < nr_files; index++) {
        int fileSize, nameReaded;

        nameReaded = loadstr(tarFile, &auxHeader->name); // Set axuHeader name - auxHeader->name = fileNames[index];
        
        fread( &fileSize, sizeof(int), 1, tarFile); // Size of the file the mytar file (after read the name)
        auxHeader->size = fileSize; // Set size of the file in the structure
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
    
    FILE * destination = fopen(tarName, "w"); // Tarbar file to write the header and data
    
    // OBJETIVE: write in the tarbar file the header (filename+size) and the data for each file
    // First calculate the size of the header (filename+size) for all the struct
    
    stHeaderEntry *stHeader = malloc(sizeof(stHeaderEntry) * nFiles);
    stHeaderEntry *auxHeader = stHeader; // Using for traverse the header
    
    for (index = 0; index < nFiles; index++) {
        
        // Now we have to copy each file content to the TarBar file
        // that's will tell me the size of each file (so we have to update struct size on heap)
        // Copy each origin file to destination file - copying byte by byte
        
        FILE * origin = fopen(fileNames[index], "r");
        
        copiedBytes = copynFile(origin, destination, INT_MAX); // destination is the tarbar file
        closedFile = fclose(origin); // closed the reading file
        
        if (closedFile == EOF) {
            created = EXIT_FAILURE;
        }
        
        // reserve space for each name field in the struct
        auxHeader->name = malloc(sizeof(fileNames[index]) + 1); // reserve space depending on filename size + 1 for the "\0"
        auxHeader->name = fileNames[index]; // Set the header name with the filename
        auxHeader->size = copiedBytes; // Size is not yet nown. We'll know it after reading the files
        
        // printf("auxHeader->name is %s\n", auxHeader->name);
        // printf("auxHeader->number is %d\n", auxHeader->size);
        // printf("auxHeader position is %p\n", auxHeader);
        
        auxHeader++; // Go to the next struct element - don't do anything after this line in the for
    }
    
    // Write headers
    auxHeader = stHeader; // The struct agains points to the beginning of the stHeader
    int success = fseek(destination, 0, SEEK_SET); // put the pointer at the beginning of the file to write the header
    
    
    if (success == 0) {
        
        fwrite(&nFiles, sizeof(int), 1, destination); // print number of files at beginning
        // Write header of each struct (stored in the heap)
        for (index = 0; index < nFiles; index++) {
            // Documentation: http://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
            fwrite(auxHeader->name, strlen(auxHeader->name), 1, destination);
            char endLine = '\0';
            fwrite(&endLine, sizeof(char), 1, destination);
            fwrite(&auxHeader->size, sizeof(int), 1, destination);
            auxHeader++;
        }
    }
    else {
        created = EXIT_FAILURE;
        // printf("We can't point the pointer to the start of the file");
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
	int result = EXIT_SUCCESS;



	return result;
}
