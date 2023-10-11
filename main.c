
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    size_t offset;
    uint8_t old;
    uint8_t new;
    struct difference *next;
} difference_t;


size_t getSizeFromFileStream(FILE *fp) {
    if (fp == NULL) {
        exit(1);
    }

    size_t size;

    fseek(fp, 0L, SEEK_END);

    size = (size_t)ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return size;

}

uint8_t *readFile(FILE *fp, size_t size) {
    if (fp == NULL) {
        exit(1);
    }

    uint8_t *buffer = (uint8_t*)malloc(size);

    fseek(fp, 0L, SEEK_SET);

    fread(buffer, size, 1, fp);

    return buffer;
}

difference_t *createDifference() {
    difference_t *diff = (difference_t*)malloc(sizeof(difference_t));
    diff->next = NULL;
    return diff;
}

void freeDifferences(difference_t *differences) {
    difference_t *pDiff;

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
            difference_t *diff = createDifference();

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
    FILE *fpPath = fopen(path, "w");

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
    FILE *fpOld = fopen(oldPath, "rb");
    FILE *fpNew = fopen(newPath, "rb");

    if (fpOld == NULL) {
        printf("Failed reading old file!\n");
        exit(1);
    }

    if (fpNew == NULL) {
        printf("Failed reading new file!\n");
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

    uint8_t *oldDataBuffer = readFile(fpOld, oldSize);
    uint8_t *newDataBuffer = readFile(fpNew, newSize);

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
