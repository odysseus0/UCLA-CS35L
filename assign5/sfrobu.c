#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#define DEFAULT_BUFFER_SIZE 20
#define DELIMITER ' ' // space

// toUpper is an option of the program; by default, it is 0
int toUpperOpt= 0;

char decrypt(char c) {
  char ret = c ^ 42;
  if (toUpperOpt) {
    return toupper((unsigned) ret);
  }
  return ret;
}
int frobcmp (char const* s1, char const* s2) {
  char c1, c2;
  while (*s1 != DELIMITER && *s2 != DELIMITER) {
    c1 = decrypt(*s1);
    c2 = decrypt(*s2);
    if (c1 != c2) {
      return c1 - c2;
    }
    s1++;
    s2++;
  } 
  if(*s1 == DELIMITER && *s2 == DELIMITER) {
    return 0; 
  }
  else if (*s1 == DELIMITER) {
    return -1;
  }
  else if (*s2 == DELIMITER) {
    return 1;
  }
}

// This is a wrapper function to use frobcmp in qsort
int compar (const void *line1, const void *line2) {
  return frobcmp(*((const char **) line1), *((const char **) line2));
}

int getBufferSize(void) {
  // If the stdin is a file, try to get its size
	struct stat fileStat;

	if(fstat(0, &fileStat) < 0)
	{
		fprintf( stderr, "Error: cannot read data from stdin\n");
		exit(1);
	}
  // If the file is a regular file, set the buffer size to the file size
  if (S_ISREG(fileStat.st_mode)) {
    return fileStat.st_size;
  }
  else {
    return DEFAULT_BUFFER_SIZE;
  }
}
/*
This program reads frobnicated text lines from standard input, and writes a 
sorted version to standard output in frobnicated form.
*/
int main(int argc, const char *argv[])
{ 
  // Variable declarations
  char currentChar;
  char *inputBuffer, **lineBuffer;
  int inputCount = 0, inputBufferSize = getBufferSize(), lineNumber = 0;

  // Parse arguments
  if (argc == 2) {
    if (!strcmp(argv[1], "-f")) {
      toUpperOpt = 1;
    }
  }
  inputBuffer = (char*) malloc(sizeof(char) * inputBufferSize);
  if (inputBuffer == NULL) {
    fprintf(stderr, "Memory Allocation Error");
    exit(1);
  }
  // Reading from the standard input stream and store them into the 
  // inputBuffer, until we reach EOF or reads an error
  while (read(0, &currentChar, 1)) {
    if (ferror(stdin)) { // if input has an error
      fprintf(stderr, "IO Error");
      exit(1);
    }
    // ENSURE: currentChar is neither end of file nor error

    // Before we add the new character to the inputBuffer, we need to make sure
    // that there is still enough memory allocated
    if (inputCount == inputBufferSize) {
      // Double the memory allocated
      inputBufferSize *= 2;
      inputBuffer = (char*) realloc(inputBuffer, sizeof(char) *  
                                      inputBufferSize);
       
      if (inputBuffer == NULL) {
        fprintf(stderr, "Memory Allocation Error");
        exit(1);
      }
    }
    if (currentChar == DELIMITER) {
      lineNumber++;
    } 
    inputBuffer[inputCount++] = currentChar;
  }

  // After reaching the end of file
  // Make sure that if the file is nonempty, the output ends with a 
  // trailing space
  if (inputCount > 0) {
    // Check if the last input is a delimiter, that is, the end of a line
    if (inputBuffer[inputCount - 1] != DELIMITER) {
      inputBuffer[inputCount++] = DELIMITER;
      lineNumber++;
    }
  }

  // If the input is empty, exit the program
  if (inputCount == 0) {
    exit(0);
  }

  // ENSURE: inputBuffer stores all the input and ends with a space
  // Process the inputBuffer and store each frobnicated text as an element into
  // lineBuffer.
  lineBuffer = (char **) malloc(sizeof(char *) * lineNumber);
  if (inputBuffer == NULL) {
    fprintf(stderr, "Memory Allocation Error");
    exit(1);
  }
  char *line = inputBuffer;
  for (int i = 0, lineNumber = 0; i < inputCount; i++) {
    if (inputBuffer[i] == DELIMITER) {
      lineBuffer[lineNumber++] = line;
      line = inputBuffer + i + 1;
    }
  }
  free(inputBuffer);
 // ENSURE: lineBuffer is an array of pointer to frobnicated texts
  // sort the lineBuffer
  qsort(lineBuffer, lineNumber, sizeof(char *), &compar);
  // Output the result of qsort into stdout
  char delim = DELIMITER;
  for (int i = 0; i < lineNumber; i++) {
    for (int j = 0; lineBuffer[i][j] != DELIMITER; j++) {
      write(1, &lineBuffer[i][j], 1);
    }
    write(1, &delim, 1);
  }

  free(lineBuffer);
  // Exit the program
  exit(0);
}