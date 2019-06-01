/*
 * Intended to be program that converts images to grayscale.
 *
 * Copyright (c) 2019 Gaurav Kumar Yadav <gaurav712@protonmail.com>
 * for license and copyright information, see the LICENSE file distributed with this source
 *
 * It takes paths of "source" and "destination" images as input.
 *
 * NOTE :
 * This program depends on "ImageMagick(https://www.imagemagick.org)"
 */

#include <stdio.h>      /* for I/O */
#include <stdlib.h>     /* for exit() */
#include <sys/wait.h>   /* for wait() */
#include <unistd.h>     /* for fork() */
#include <string.h>     /* for strcpy() */

#define EXTMAX 10
#define BANDSIZE 25

/* Shows program's usage rules */
static void showHelp(void);
/* Explains itself */
static void convert(char *fileName);
/* Reads data from the source file */
static unsigned readEntry(FILE *fp, char delim);
/* Skip comment if any */
static void skipComment(FILE *fp);
/* Write basic stuff like width and height in the output file */
static void writeHeaders(FILE *fp, unsigned width, unsigned height);
/* Converts numbers to strings */
static void numToStr(char *dest, unsigned num);
/* Reverses a string */
static void strReverse(char *str, unsigned short len);

int main(int argc, char *argv[]) {

    pid_t childPID;
    int childExitStatus;

    if(argc != 3) {
        showHelp();
        exit(1);
    }

    /* Convert it to *.ppm */
    if((childPID = fork()) < 0) {
        perror("fork() failed!");
        exit(1);
    }

    if(!childPID) {
        /* It's the child */
        execl("/usr/bin/convert", "convert", argv[1], ".temp.ppm", NULL);
        exit(0);
    } else {
        /* It's the parent */
        if(wait(&childExitStatus) < 0) {
            perror("wait() failed!");
            exit(1);
        }

        if(WIFEXITED(childExitStatus)) {
            printf("ImageMagick converted the image succesfully, now proceeding\n");
            convert(".temp.ppm");
        } else {
            perror("ImageMagick had some error processing the image!");
            exit(1);
        }
    }

    /* Sketching is done, now export the image */
    if((childPID = fork()) < 0) {
        perror("fork() failed!");
        exit(1);
    }

    if(!childPID) {
        /* It's the child */
        execl("/usr/bin/convert", "convert", ".output.ppm", argv[2], NULL);
        exit(0);
    } else {
        /* It's the parent */
        if(wait(&childExitStatus) < 0) {
            perror("wait() failed!");
            exit(1);
        }

        if(WIFEXITED(childExitStatus)) {
            printf("Operation completed!\n");
        } else {
            perror("ImageMagick had some error exporting the output image!");
            exit(1);
        }
    }

    return 0;
}

void
showHelp(void) {

    printf("\nUSAGE :\n\nppm_test {PATH_TO_SOURCE} {PATH_TO_DESTINATION}\n\n");
}

void
convert(char *fileName) {

    FILE *read, *write;
    unsigned width, height, maxIntensity;
    char red, green, blue, colorToFill;

    if((read = fopen(fileName, "r")) == NULL) {
        perror("Can't access source file!");
        exit(1);
    }

    /* Skip "P6\n" */
    fseek(read, 3, SEEK_SET);

    skipComment(read);

    /* Get Width and Height */
    width = readEntry(read, ' ');
    height = readEntry(read, '\n');
    maxIntensity = readEntry(read, '\n');

    printf("Detected :-\nWidth = %u\nHeight = %u\nMax color intensity = %u\n", width, height, maxIntensity);

    /* Open file for writing */
    if((write = fopen(".output.ppm", "wb")) == NULL) {
        perror("Can't access output file!");
        exit(1);
    }

    writeHeaders(write, width, height);

    /* LET'S START THE GAME WE'RE ALL WAITIN FOR */
    for(unsigned row = 0; row < height; row++) {
        for(unsigned col = 0; col < width; col++) {
            fread(&red, 1, 1, read);
            fread(&green, 1, 1, read);
            fread(&blue, 1, 1, read);
            colorToFill = (red + green + blue)/3;

            /* Putting the colorToFill in the output file */
            fwrite(&colorToFill, 1, 1, write);
            fwrite(&colorToFill, 1, 1, write);
            fwrite(&colorToFill, 1, 1, write);
        }
    }

    fclose(read);
    fclose(write);
}

unsigned
readEntry(FILE *fp, char delim) {

    char *stream, ch;
    short count = 0;
    unsigned data;

    while(1) {
        fread(&ch, 1, 1, fp);
        if(ch == delim)
            break;
        count++;
    }

    stream = malloc(count+1);
    fseek(fp, -(count + 1), SEEK_CUR);
    fread(stream, 1, count, fp);
    /* Move pointer after that delimiter */
    fseek(fp, 1L, SEEK_CUR);
    data = atoi(stream);
    free(stream);
    return data;
}

void
skipComment(FILE *fp) {

    char ch;

    while(1) {
        fread(&ch, 1, 1, fp);
        if(ch == '#')
            while(ch != '\n')
                fread(&ch, 1, 1, fp);
        else {
            fseek(fp, -1L, SEEK_CUR);
            break;
        }
    }
}

void
writeHeaders(FILE *fp, unsigned width, unsigned height) {

    const char str[] = "P6\n# Created by ppm_test(Copyright (c) 2019 Gaurav Kumar Yadav)\n";
    char temp[20];
    unsigned maxIntensity = 255;

    fwrite(str, strlen(str), 1, fp);

    numToStr(temp, width);
    /* Why create a new variable just for a space */
    strcat(temp, " ");
    fwrite(temp, strlen(temp), 1, fp);

    numToStr(temp, height);
    strcat(temp, "\n");
    fwrite(temp, strlen(temp), 1, fp);

    numToStr(temp, maxIntensity);
    strcat(temp, "\n");
    fwrite(temp, strlen(temp), 1, fp);
}

void
numToStr(char *dest, unsigned num) {

    short count = 0;

    while(num) {
        dest[count] = (num % 10) + '0';
        num /= 10;
        count++;
    }
    dest[count] = '\0';
    strReverse(dest, strlen(dest));
}

void
strReverse(char *str, unsigned short len) {

    char temp[20];
    unsigned short lenBak = len, count;

    strcpy(temp, str);

    len--;
    for(count = 0; count < lenBak; count++) {
        temp[count] = str[len];
        len--;
    }
    temp[count] = '\0';
    strcpy(str, temp);
}

