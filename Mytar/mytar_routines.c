#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes from the origin file to the destination file.
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
        // DEBUG: putc(name[index],stdout);
    }    

    // printf("The string is %s\n", name);
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
    int nr_files = 0, index = 0, size = 0; // Return parameter
    stHeaderEntry *stHeader=NULL; // header for creating heap space

    if (fread( &nr_files, sizeof(int), 1, tarFile) == 0) { // read "number of files" compress in the .mtar
        printf("Wasn't possible to read the mtar file\n");
        return EXIT_FAILURE; // wans't possible to read the mtar file
    }

    stHeader=malloc(sizeof(stHeaderEntry)*nr_files); // Allocate memory for the stHeader
   
    for (index = 0; index < nr_files; index++) {

        if (loadstr(tarFile, &stHeader[index].name) != 0) {
            printf("Wasn't possible to read the mtar file\n");
            return EXIT_FAILURE;
        }

        fread(&size, sizeof(unsigned int), 1, tarFile); // Set size of the file in the structure
        stHeader[index].size = size;
    }

    (*nFiles)=nr_files; // Return the number of files for the mtar
    (*header)=stHeader;

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

// OBJETIVE: write in the tarbar file the header (filename+size) and the data for each file
// First calculate the size of the header (filename+size) for all the struct

int
createTar(int nFiles, char *fileNames[], char tarName[])
{
    FILE * inputFile; // Used for reading each .txt file
    FILE * outputFile; // Used for writing in the output file.

    int copiedBytes = 0, stHeaderBytes = 0, index = 0;
    stHeaderEntry *stHeader; // Pointer to the program header struct.

    // Create a stHeader in the heap with the correct size

    stHeader      =  malloc(sizeof(stHeaderEntry) * nFiles); // Allocate memory for an array of stHeader Structs
    stHeaderBytes += sizeof(int); // One integer for the number of files (written at the beginning of the file)
    stHeaderBytes += nFiles*sizeof(unsigned int); // other int for each struct to store the bytes (aka size) of the file

    for (index=0; index < nFiles; index++) {
        stHeaderBytes+=strlen(fileNames[index])+1; // Sum the bytes for each filename (+1 for the '\0' character)
    }

    outputFile =  fopen(tarName, "w"); // Open the name.mtar file for writing the header and data of the files.
    fseek(outputFile, stHeaderBytes, SEEK_SET); // Move Outputfile pointer to the "data" section (leaving space for the header we'll write at the end of this function)

    for (index=0; index < nFiles; index++) {

        if ((inputFile = fopen(fileNames[index], "r")) == NULL) {
            printf("Yo! File: %s doesn't exist\n", fileNames[index]);
            return (EXIT_FAILURE);
        } // Try to open the file. EXIT_FAILURE will be returned if errors

        copiedBytes = copynFile(inputFile, outputFile, INT_MAX); // Copy N bytes to the output file (INT_MAX == huge number to ensure you copy all the character)

        stHeader[index].size = copiedBytes; // Set the size from the copiedBytes for each struct.
        stHeader[index].name = malloc(sizeof(fileNames[index]) + 1); // Heap space for the name char + '\0' character
        strcpy(stHeader[index].name, fileNames[index]); // Copy one string to the struct entry

        if (fclose(inputFile) == EOF) return EXIT_FAILURE; // Wasn't possible to close the file. Return and error
    }
     
    // Now move the file pointer to the beggining of the file in order to write: number of files + the header for each file

    if (fseek(outputFile, 0, SEEK_SET) != 0)
        return (EXIT_FAILURE); // wasn't possible to move the pointer to the beginning of the file. Returns and error
    else
        fwrite(&nFiles, sizeof(int), 1, outputFile); // write Number of Files in the output .tar
    
    // Write header for each struct
    // Documentation: http://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm

    for (index = 0; index < nFiles; index++) {
        fwrite(stHeader[index].name, strlen(stHeader[index].name)+1, 1, outputFile); // Important add one when saving the length for the name (for the '\0' character)
        fwrite(&stHeader[index].size, sizeof(unsigned int), 1, outputFile); // Write one byte 
        // printf("%d\n", stHeader[index].size);
    }

    // FREE MEMORY. First, delete the char *names pointer.
    // Then erase all the structs.
    
    for (index=0; index < nFiles; index++) {
        free(stHeader[index].name); // Free the filenames array of characters
    }

    free(stHeader); // Free the struct created in heap

    if (fclose(outputFile) == EOF) { return (EXIT_FAILURE); } // Try close file. Return an error if wasn't possible

    return (EXIT_SUCCESS);
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
    FILE *tarFile = NULL; // file manager
    FILE *destinationFile = NULL;
    stHeaderEntry *stHeader; // Array of structs for header
    int nr_files = 0, index = 0, copiedBytes = 0;

    if((tarFile = fopen(tarName, "r") ) == NULL) {
        printf("Yo! %s file doesn't exit \n", tarName);
        return (EXIT_FAILURE); // File doesn't exit or there was a problem
    }

    if (readHeader(tarFile, &stHeader, &nr_files) == -1) {
        printf("We couldn't load the header \n");
        return (EXIT_FAILURE); 
    } // Returns in stHeader the array of struct (name/size) for each file and in nr_files the total num of files in the .mtar

    // Write the content to the outputFile

    for (index = 0; index < nr_files; index++) {

        if ((destinationFile = fopen(stHeader[index].name, "w")) == NULL) { return EXIT_FAILURE; } // Try to open the output file | return error if we couldn't create the file
        else {
            copiedBytes = copynFile(tarFile, destinationFile, stHeader[index].size); // Write nBytes to the output file (where nBytes is obtained when reading the .mtar header)
        }  // copied nBytes to output file
        
        if(fclose(destinationFile) != 0) { return EXIT_FAILURE; } // close the file
    }
    
    // DEBUG STUFF
    printf("Number of files is %d\n", nr_files);
    for (index = 0; index <nr_files;index++) {
        printf("Filename is %s and size is %u \n", stHeader[index].name, stHeader[index].size);
    }
    
    // FREE MEMORY

    for (index = 0; index <nr_files; index++) {
        free(stHeader[index].name);
    }

    free(stHeader); // Delete main struct

	return (EXIT_SUCCESS);
}
