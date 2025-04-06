/*
    BINPL (v2.0 / April 7, 2025)
    Copyright (c) 2025 Cyril John Magayaga
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper function to trim both leading and trailing whitespace.
void trim(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start))
        start++;
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

// Function to convert a binary (Bina) string with potential whitespace to an integer.
int binaryToInt(const char *bin) {
    char cleaned[128];
    int j = 0;
    for (int i = 0; bin[i] != '\0'; i++) {
        if (!isspace((unsigned char)bin[i])) {
            if (bin[i] != '0' && bin[i] != '1') {
                fprintf(stderr, "Invalid binary digit: %c\n", bin[i]);
                return -1;
            }
            cleaned[j++] = bin[i];
        }
    }
    cleaned[j] = '\0';
    
    int result = 0;
    for (int i = 0; cleaned[i] != '\0'; i++) {
        result = result * 2 + (cleaned[i] - '0');
    }
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <BINPL2 source file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    
    char line[256];
    int inProgram = 0;
    int foundBegin = 0;
    int foundEnd = 0;
    char programName[64] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline and trim whitespace.
        line[strcspn(line, "\n")] = '\0';
        trim(line);

        // Remove inline comments: If ";;" is found, truncate the line there.
        char *comment = strstr(line, ";;");
        if (comment) {
            *comment = '\0';
            trim(line); // re-trim in case spaces remain
        }
        
        // Skip empty lines.
        if (line[0] == '\0')
            continue;
        
        // Check for the start marker (::BEGIN or ::START).
        if ((strncmp(line, "::BEGIN", 7) == 0) || (strncmp(line, "::START", 7) == 0)) {
            if (inProgram) {
                fprintf(stderr, "Error: Nested start marker encountered.\n");
                fclose(fp);
                return EXIT_FAILURE;
            }
            foundBegin = 1;
            inProgram = 1;
            // Optionally process program name.
            char *token = strtok(line, " \t");
            token = strtok(NULL, " \t");
            if (token) {
                strncpy(programName, token, sizeof(programName) - 1);
                programName[sizeof(programName) - 1] = '\0';
                printf("Starting program: %s\n", programName);
            }
            continue;
        }
        
        // Check for the end marker ::END.
        if (strcmp(line, "::END") == 0) {
            if (!inProgram) {
                fprintf(stderr, "Error: ::END encountered without a matching start marker.\n");
                fclose(fp);
                return EXIT_FAILURE;
            }
            inProgram = 0;
            foundEnd = 1;
            break;
        }
        
        // Process instructions only if inside a valid program block.
        if (inProgram) {
            char type[16], value[128];
            if (sscanf(line, "%15s %127[^\n]", type, value) != 2) {
                fprintf(stderr, "Invalid instruction format: %s\n", line);
                continue;
            }
            
            int num = 0;
            if (strcmp(type, "Octa") == 0) {
                num = (int)strtol(value, NULL, 8);
            } else if (strcmp(type, "Deci") == 0) {
                num = (int)strtol(value, NULL, 10);
            } else if (strcmp(type, "Hexa") == 0) {
                // Validate that every character is a valid hexadecimal digit.
                for (int i = 0; value[i] != '\0'; i++) {
                    if (!isxdigit((unsigned char)value[i])) {
                        fprintf(stderr, "Invalid hexadecimal digit '%c' in value: %s\n", value[i], value);
                        num = -1;
                        break;
                    }
                }
                if (num == -1)
                    continue;
                num = (int)strtol(value, NULL, 16);
            } else if (strcmp(type, "Bina") == 0) {
                num = binaryToInt(value);
                if (num < 0)
                    continue;
            } else {
                fprintf(stderr, "Unknown numeral system: %s\n", type);
                continue;
            }
            // Output the corresponding ASCII character.
            putchar(num);
        }
    }
    
    fclose(fp);
    
    if (!foundBegin) {
        fprintf(stderr, "Error: Missing start marker (::BEGIN or ::START).\n");
        return EXIT_FAILURE;
    }
    if (!foundEnd) {
        fprintf(stderr, "Error: Missing ::END marker.\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
