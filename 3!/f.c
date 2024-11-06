#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>


#define PATH_M 1024

typedef struct Employee {
    unsigned int id;
    char firstName[50];
    char lastName[50];
    double salary;
} Employee;

typedef enum kOpts {
    OPT_A,
    OPT_D
} kOpts;

//че за хуйня с указателями
int GetOpts(int argc, char** argv, kOpts *option, char *inputPath, char *outputPath) {
    if (argc != 4) {
        return 1;
    }

    const char* opt = argv[1];

    if (opt[0] == '/' || opt[0] == '-') {
        switch (opt[1]) {
        case 'a':
            *option = OPT_A;
            break;
        case 'd':
            *option = OPT_D;
            break;
        default:
            return 1;
        }
    } else {
        return 1;
    }

    // Преобразуем входной и выходной пути в абсолютные
    if (realpath(argv[2], inputPath) == NULL) {
        perror("Wrong input file path");
        return 1;
    }

    // Для выходного пути используем относительный путь, если файл еще не существует
    if (realpath(argv[3], outputPath) == NULL) {
        strncpy(outputPath, argv[3], PATH_M);
    }

    if (strcmp(inputPath, outputPath) == 0) {
        return 1;
    }
    return 0;
}

int compareEmployees(const void *a, const void *b) {
    Employee *empA = (Employee *)a;
    Employee *empB = (Employee *)b;

    if (empA->salary != empB->salary) {
        return (empA->salary > empB->salary) ? 1 : -1;
    }
    int lastNameCompare = strcmp(empA->lastName, empB->lastName);
    if (lastNameCompare != 0) {
        return lastNameCompare;
    }
    int firstNameCompare = strcmp(empA->firstName, empB->firstName);
    if (firstNameCompare != 0) {
        return firstNameCompare;
    }
    return (empA->id > empB->id) ? 1 : -1;
}

void readEmployees(const char *inputPath, Employee **employees, int *count) {
    FILE *file = fopen(inputPath, "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    *count = 0;
    *employees = NULL;

    while (!feof(file)) {
        *employees = realloc(*employees, (*count + 1) * sizeof(Employee));
        fscanf(file, "%u %49s %49s %lf", &(*employees)[*count].id, (*employees)[*count].firstName,
               (*employees)[*count].lastName, &(*employees)[*count].salary);
        (*count)++;
    }

    fclose(file);
}

void writeEmployees(const char *outputPath, Employee *employees, int count) {
    FILE *file = fopen(outputPath, "w");
    if (!file) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%u %s %s %.2lf\n", employees[i].id, employees[i].firstName,
                employees[i].lastName, employees[i].salary);
    }

    fclose(file);
}

void HandlerOptA(const char *inputPath, const char *outputPath) {
    Employee *employees;
    int count;

    readEmployees(inputPath, &employees, &count);
    qsort(employees, count, sizeof(Employee), compareEmployees);
    writeEmployees(outputPath, employees, count);
    free(employees);
}

void HandlerOptD(const char *inputPath, const char *outputPath) {
    Employee *employees;
    int count;

    readEmployees(inputPath, &employees, &count);
    qsort(employees, count, sizeof(Employee), compareEmployees);
    for (int i = 0; i < count / 2; i++) {
        Employee temp = employees[i];
        employees[i] = employees[count - i - 1];
        employees[count - i - 1] = temp;
    }
    writeEmployees(outputPath, employees, count);
    free(employees);
}

int main(int argc, char** argv) {
    kOpts opt;
    char inputPath[PATH_M];
    char outputPath[PATH_M];

    if (GetOpts(argc, argv, &opt, inputPath, outputPath)) {
        printf("%s\n", "Incorrect option or path or path of input and output file are the same");
        return 1;
    }

    if (opt == OPT_A) {
        HandlerOptA(inputPath, outputPath);
    } else {
        HandlerOptD(inputPath, outputPath);
    }

    return 0;
}