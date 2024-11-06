#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100
#define MAX_DATE_LEN 11 // dd-mm-yyyy

typedef struct Liver {
    char lastName[MAX_NAME_LEN];
    char firstName[MAX_NAME_LEN];
    char patronymic[MAX_NAME_LEN];
    char birthDate[MAX_DATE_LEN];
    char gender;
    double monthlyMoney;
    struct Liver* next;
} Liver;

typedef enum { ADD, MODIFY, DELETE } ActionType;

typedef struct Modification {
    ActionType action; // Тип действия
    Liver* liver; // Указатель на изменённого жителя (для добавления и изменения)
    struct Modification* next;
} Modification;

typedef struct UndoStack {
    Modification* top; // Верхняя модификация стека
    int count; // Количество модификаций
} UndoStack;

void initStack(UndoStack* stack) {
    stack->top = NULL; // Инициализация стека
    stack->count = 0;
}

void push(UndoStack* stack, Modification* mod) {
    mod->next = stack->top; // Помещаем модификацию на вершину стека
    stack->top = mod;
    stack->count++; // Увеличиваем счётчик
}

Modification* pop(UndoStack* stack) {
    if (stack->top == NULL) return NULL; // Если стек пуст, ничего не возвращаем
    Modification* mod = stack->top; // Сохраняем верхнюю модификацию
    stack->top = stack->top->next; // Убираем верхнюю модификацию из стека
    stack->count--; // Уменьшаем счётчик
    return mod;
}

// Создание нового структурного жителя
Liver* createLiver(char* lastName, char* firstName, char* patronymic,
                   char* birthDate, char gender, double monthlyMoney) {
    Liver* newLiver = (Liver*)malloc(sizeof(Liver));
    strcpy(newLiver->lastName, lastName);
    strcpy(newLiver->firstName, firstName);
    strcpy(newLiver->patronymic, patronymic ? patronymic : "");
    strcpy(newLiver->birthDate, birthDate);
    newLiver->gender = gender;
    newLiver->monthlyMoney = monthlyMoney;
    newLiver->next = NULL;
    return newLiver;
}

int isValidName(const char* name) {
    if (!isupper(name[0])) {
        return 0;
    }
    for (int i = 1; name[i] != '\0'; ++i) {
        if (!islower(name[i])) {
            return 0;
        }
    }
    return 1;
}

int isValidDate(const char* date) {
    if (strlen(date) != 10 || date[2] != '-' || date[5] != '-') {
        return 0;
    }
    int day = atoi(date);
    int month = atoi(date + 3);
    int year = atoi(date + 6);

    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 0 || year > 2024) {
        return 0;
    }
    return 1;
}

int isValidGender(char gender) {
    return gender == 'M' || gender == 'W';
}

int isValidMoney(double money) {
    return money >= 0;
}

// Добавление жителя в нужное место
void addLiver(Liver** head, char* lastName, char* firstName, char* patronymic,
              char* birthDate, char gender, double monthlyMoney, UndoStack* stack) {
    Liver* newLiver = createLiver(lastName, firstName, patronymic, birthDate, gender, monthlyMoney);
    if (*head == NULL || strcmp(birthDate, (*head)->birthDate) < 0) {
        newLiver->next = *head;
        *head = newLiver;
    } else {
        Liver* current = *head;
        while (current->next && strcmp(birthDate, current->next->birthDate) > 0) {
            current = current->next;
        }
        newLiver->next = current->next;
        current->next = newLiver;
    }

    // Логирование модификации
    Modification* mod = (Modification*)malloc(sizeof(Modification));
    mod->action = ADD;
    mod->liver = newLiver; // Сохраняем указатель на нового жителя
    push(stack, mod);
}

// Загружаем данные из файла
void loadFromFile(Liver** head, const char* filename, UndoStack* stack) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Не удалось открыть файл для чтения");
        return;
    }

    char lastName[MAX_NAME_LEN], firstName[MAX_NAME_LEN], patronymic[MAX_NAME_LEN];
    char birthDate[MAX_DATE_LEN], gender;
    double money;

    while (fscanf(file, "%s %s %s", lastName, firstName, patronymic) == 3) {
        if (isdigit(patronymic[0])) {
            strcpy(birthDate, patronymic);  //перекопируевам
            patronymic[0] = '\0';  //отчества не ма
        } else {
            fscanf(file, "%s", birthDate);
        }

        fscanf(file, " %c %lf", &gender, &money);

        if (!isValidName(lastName) || !isValidName(firstName) ||
            (strlen(patronymic) > 0 && !isValidName(patronymic)) ||
            !isValidDate(birthDate) || !isValidGender(gender) || !isValidMoney(money)) {
            printf("Ошибка валидации данных для: %s %s\n", lastName, firstName);
            continue;
        }

        addLiver(head, lastName, firstName, strlen(patronymic) > 0 ? patronymic : NULL,
                 birthDate, gender, money, stack);
    }
    fclose(file);
}

// Сохранение жителей в файлик
void saveToFile(Liver* head, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Не удалось открыть файл для записи");
        return;
    }
    while (head) {
        fprintf(file, "%s %s %s %s %c %.2f\n",
                head->lastName, head->firstName, head->patronymic, head->birthDate, head->gender, head->monthlyMoney);
        head = head->next;
    }
    fclose(file);
}

// Ищем жителей
void searchLivers(Liver* head, char* lastName, char* firstName) {
    int found = 0;
    while (head) {
        if (strcmp(head->lastName, lastName) == 0 && strcmp(head->firstName, firstName) == 0) {
            printf("Фамилия: %s, Имя: %s, Отчество: %s, Дата рождения: %s, Пол: %c, Доход: %.2f\n",
                   head->lastName, head->firstName,
                   strlen(head->patronymic) > 0 ? head->patronymic : "(нет)",
                   head->birthDate, head->gender, head->monthlyMoney);
            found = 1;
        }
        head = head->next;
    }
    if (!found) {
        printf("Жители с такими фамилией и именем не найдены.\n");
    }
}

// Меняем инфу жителя
void modifyLiver(Liver* liver, char* birthDate, char gender, double monthlyMoney, UndoStack* stack) {
    // Сохраняем старые данные для отмены
    Modification* mod = (Modification*)malloc(sizeof(Modification));
    mod->action = MODIFY;
    mod->liver = (Liver*)malloc(sizeof(Liver));
    memcpy(mod->liver, liver, sizeof(Liver)); // Сохраняем информацию о старом жителе
    push(stack, mod);
    
    strcpy(liver->birthDate, birthDate);
    liver->gender = gender;
    liver->monthlyMoney = monthlyMoney;
}

void modifyLiverInfo(Liver* head, char* lastName, char* firstName, char* patronymic, UndoStack* stack) {
    while (head) {
        if (strcmp(head->lastName, lastName) == 0 &&
            strcmp(head->firstName, firstName) == 0 &&
            (strlen(patronymic) == 0 || strcmp(head->patronymic, patronymic) == 0)) {
            
            printf("Найдена запись. Введите новые данные.\n");
            char birthDate[MAX_DATE_LEN];
            char gender;
            double monthlyMoney;

            printf("Введите новую дату рождения (dd-mm-yyyy): ");
            scanf("%s", birthDate);
            printf("Введите пол (M/W): ");
            scanf(" %c", &gender);
            printf("Введите средний доход: ");
            scanf("%lf", &monthlyMoney);

            // Обновляем данные
            modifyLiver(head, birthDate, gender, monthlyMoney, stack);
            printf("Информация о жителе обновлена.\n");
            return;
        }
        head = head->next;
    }
    printf("Житель не найден.\n");
}

void deleteLiver(Liver** head, char* lastName, char* firstName, char* patronymic, UndoStack* stack) {
    Liver* current = *head;
    Liver* prev = NULL;

    while (current) {
        if (strcmp(current->lastName, lastName) == 0 &&
            strcmp(current->firstName, firstName) == 0 &&
            (strlen(patronymic) == 0 || strcmp(current->patronymic, patronymic) == 0)) {
            
            if (prev) {
                prev->next = current->next;
            } else {
                *head = current->next;
            }

            // Логирование модификации
            Modification* mod = (Modification*)malloc(sizeof(Modification));
            mod->action = DELETE;
            mod->liver = current; // Сохраняем указатель на удаляемого жителя
            push(stack, mod);

            free(current); // Освобождаем память
            printf("Житель успешно удалён.\n");
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Житель не найден.\n");
}

void printList(Liver* head) {
    while (head) {
        printf("Фамилия: %s, Имя: %s, Отчество: %s, Дата рождения: %s, Пол: %c, Доход: %.2f\n",
               head->lastName, head->firstName,
               strlen(head->patronymic) > 0 ? head->patronymic : "(нет)",
               head->birthDate, head->gender, head->monthlyMoney);
        head = head->next;
    }
}

// Освобождаем память от нашего списка
void freeList(Liver* head) {
    while (head) {
        Liver* temp = head;
        head = head->next;
        free(temp);
    }
}

void undoModifications(UndoStack* stack, Liver** head) {
    int N = stack->count;
    int undoCount = N / 2;
    for (int i = 0; i < undoCount; i++) {
        Modification* mod = pop(stack);
        if (mod) {
            switch (mod->action) {
                case ADD:
                    deleteLiver(head, mod->liver->lastName, mod->liver->firstName, mod->liver->patronymic, NULL);
                    break;
                case DELETE:
                    addLiver(head, mod->liver->lastName, mod->liver->firstName, mod->liver->patronymic,
                             mod->liver->birthDate, mod->liver->gender, mod->liver->monthlyMoney, NULL);
                    break;
                case MODIFY:
                    // Реализуем отмену для изменения: восстанавливаем предыдущее состояние
                    modifyLiver(mod->liver, mod->liver->birthDate, mod->liver->gender, mod->liver->monthlyMoney, NULL);
                    break;
            }
            free(mod->liver); // Освобождаем память
            free(mod);
        }
    }
    printf("Отменены последние %d модификаций.\n", undoCount);
}

int main() {
    Liver* head = NULL;
    //UndoStack stack = {.top = -1};
    int choice;
    char filename[MAX_NAME_LEN];
    UndoStack stack;
    initStack(&stack);

    do {
        printf("\nМеню:\n");
        printf("1. Загрузить данные из файла\n");
        printf("2. Сохранить данные в файл\n");
        printf("3. Добавить жителя\n");
        printf("4. Поиск жителя\n");
        printf("5. Изменить информацию о жителе\n");
        printf("6. Удалить жителя\n");
        printf("7. Печать списка жителей\n");
        printf("8. Отмена последних действий\n");
        printf("9. Выйти\n");
        printf("Выберите действие: ");
        if (scanf("%d", &choice) != 1) {
            printf("Некорректный ввод. Пожалуйста, введите число.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                printf("Введите путь к файлу: ");
                scanf("%s", filename);
                loadFromFile(&head, filename, &stack);
                break;
            case 2:
                printf("Введите путь к файлу: ");
                scanf("%s", filename);
                saveToFile(head, filename);
                break;
            case 3: {
                char lastName[MAX_NAME_LEN], firstName[MAX_NAME_LEN], patronymic[MAX_NAME_LEN] = "";
                char birthDate[MAX_DATE_LEN], gender;
                double monthlyMoney;

                printf("Введите фамилию: ");
                scanf("%s", lastName);
                if (!isValidName(lastName)) {
                    printf("Некорректная фамилия.\n");
                    break;
                }

                printf("Введите имя: ");
                scanf("%s", firstName);
                if (!isValidName(firstName)) {
                    printf("Некорректное имя.\n");
                    break;
                }

                getchar();
                printf("Введите отчество (если нет, нажмите Enter): ");
                fgets(patronymic, MAX_NAME_LEN, stdin);
                patronymic[strcspn(patronymic, "\n")] = '\0';  // Меняем \n на \0

                if (strlen(patronymic) > 0 && !isValidName(patronymic)) {
                    printf("Некорректное отчество.\n");
                    break;
                }

                printf("Введите дату рождения (в формате dd-mm-yyyy): ");
                scanf("%s", birthDate);
                if (!isValidDate(birthDate)) {
                    printf("Некорректная дата рождения.\n");
                    break;
                }

                printf("Введите пол (M/W): ");
                scanf(" %c", &gender);
                if (!isValidGender(gender)) {
                    printf("Некорректный пол.\n");
                    break;
                }

                printf("Введите средний доход: ");
                scanf("%lf", &monthlyMoney);
                if (!isValidMoney(monthlyMoney)) {
                    printf("Некорректный доход.\n");
                    break;
                }
                addLiver(&head, lastName, firstName, strlen(patronymic) > 0 ? patronymic : NULL,
                        birthDate, gender, monthlyMoney, &stack);
                printf("Житель добавлен.\n");
                break;
            }
            case 4: {
                char lastName[MAX_NAME_LEN], firstName[MAX_NAME_LEN];
                printf("Введите фамилию: ");
                scanf("%s", lastName);
                printf("Введите имя: ");
                scanf("%s", firstName);
                searchLivers(head, lastName, firstName);
                break;
            }
            case 5: {
                char lastName[MAX_NAME_LEN], firstName[MAX_NAME_LEN], patronymic[MAX_NAME_LEN] = "";

                printf("Введите фамилию: ");
                scanf("%s", lastName);
                printf("Введите имя: ");
                scanf("%s", firstName);

                getchar();
                printf("Введите отчество (если нет, нажмите Enter): ");
                fgets(patronymic, MAX_NAME_LEN, stdin);
                patronymic[strcspn(patronymic, "\n")] = '\0';

                modifyLiverInfo(head, lastName, firstName, patronymic, &stack);
                break;
            }

            case 6: {
                char lastName[MAX_NAME_LEN], firstName[MAX_NAME_LEN], patronymic[MAX_NAME_LEN] = "";

                printf("Введите фамилию: ");
                scanf("%s", lastName);
                printf("Введите имя: ");
                scanf("%s", firstName);

                getchar();
                printf("Введите отчество (если нет, нажмите Enter): ");
                fgets(patronymic, MAX_NAME_LEN, stdin);
                patronymic[strcspn(patronymic, "\n")] = '\0';

                deleteLiver(&head, lastName, firstName, patronymic, &stack);
                break;
            }
            case 7:
                printList(head);
                break;
            case 8:
                undoModifications(&stack, &head);
                break;
            case 9:
                freeList(head);
                printf("Выход из программы.\n");
                break;
            default:
                printf("Некорректный выбор. Попробуйте снова.\n");
        }
    } while (choice != 9);

    return 0;
}