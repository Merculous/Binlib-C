
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct difference
{
    size_t offset;
    uint8_t old;
    uint8_t new;
    struct difference *next;
} difference_t;

size_t getSizeFromFileStream(FILE *fp) {
    if (fp == NULL) {
        printf("Failed getting size!\n");
        exit(1);
    }

    size_t size = 0;

    fseek(fp, 0L, SEEK_END);

    size = (size_t)ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return size;

}

uint8_t *readDataFromFD(FILE *fp) {
    if (fp == NULL) {
        printf("Failed opening fd for data gathering!\n");
        exit(1);
    }

    size_t size = getSizeFromFileStream(fp);

    uint8_t *buffer = malloc(size);

    fseek(fp, 0L, SEEK_SET);

    fread(buffer, size, 1, fp);

    return buffer;
}

difference_t *allocateDifference() {
    difference_t *diff = NULL;

    diff = malloc(sizeof(difference_t));

    if (diff == NULL) {
        printf("Failed allocating difference!\n");
    }

    return diff;
}

void freeDifferences(difference_t *differences) {
    difference_t *pDiff = NULL;

    while (differences != NULL) {
        pDiff = differences;
        differences = differences->next;
        free(pDiff);
    }
}

difference_t *diff(uint8_t *oldData, uint8_t *newData, size_t size) {
    difference_t *head = NULL;

    for (size_t i = 0; i < size; i++) {
        if (oldData[i] != newData[i]) {
            difference_t *diff = allocateDifference();

            diff->offset = i;
            diff->old = oldData[i];
            diff->new = newData[i];

            diff->next = head;

            head = diff;
        }
    }

    return head;
}

void writeDifferencesToPath(const char *path, difference_t *differences) {
    FILE *fpPath = fopen(path, "wt");

    if (fpPath == NULL) {
        printf("Failed to open %s for writing!\n", path);
        exit(1);
    }

    for (difference_t *pDiff = differences; pDiff->next != NULL; pDiff = pDiff->next) {
        fprintf(fpPath, "Offset: %zu\n", pDiff->offset);
        fprintf(fpPath, "Old: %x\n", pDiff->old);
        fprintf(fpPath, "New: %x\n", pDiff->new);
    }

    fclose(fpPath);
}

difference_t *diffFilesFromPath(const char *oldPath, const char *newPath, const char *diffPath) {
    FILE *fpOld = NULL;

    fpOld = fopen(oldPath, "rb");

    if (fpOld == NULL) {
        printf("Failed opening old path!\n");
        fclose(fpOld);
        exit(1);
    }

    FILE *fpNew = NULL;

    fpNew = fopen(newPath, "rb");

    if (fpNew == NULL) {
        printf("Failed opening new path!\n");
        fclose(fpNew);
        exit(1);
    }

    size_t oldSize = getSizeFromFileStream(fpOld);
    size_t newSize = getSizeFromFileStream(fpNew);

    if (oldSize != newSize) {
        printf("Input file sizes differ!\n");

        fclose(fpOld);
        fclose(fpNew);

        exit(1);
    }

    uint8_t *oldDataBuffer = readDataFromFD(fpOld);
    uint8_t *newDataBuffer = readDataFromFD(fpNew);

    difference_t *differences = diff(oldDataBuffer, newDataBuffer, oldSize);

    if (differences != NULL) {
        for (difference_t *pDiff = differences; pDiff->next != NULL; pDiff = pDiff->next) {
            printf("Found difference at offset: %x\n", pDiff->offset);
            printf("Old: %x\n", pDiff->old);
            printf("New: %x\n", pDiff->new);
        }
    }

    writeDifferencesToPath(diffPath, differences);

    freeDifferences(differences);

    free(oldDataBuffer);
    free(newDataBuffer);

    fclose(fpOld);
    fclose(fpNew);

    return differences;
}

int main(int argc, char **argv) {
    if (argc == 4) {
        diffFilesFromPath(argv[1], argv[2], "diff.txt");
        return 0;
    } else {
        printf("Usage: <old> <new>\n");
        return 1;
    }
}
