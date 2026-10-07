#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

/* Описание структуры узла односвязного списка */
typedef struct Node {
    char *data;
    struct Node *next;
} Node;

/* Функция добавления строки в конец списка */
void append(Node **head, const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Ошибка выделения памяти под узел");
        exit(EXIT_FAILURE);
    }

    /* Выделяем память под строку: strlen(str) + 1 байт для терминального нуля '\0' */
    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("Ошибка выделения памяти под строку");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

/* Функция вывода всех элементов списка */
void print_list(const Node *head) {
    const Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
}

/* Функция полного освобождения памяти */
void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->data); // 1. Освобождаем строку
        free(current);       // 2. Освобождаем сам узел
        current = next;
    }
}

int main(void) {
    char buffer[BUFFER_SIZE];
    Node *head = NULL;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        /* Удаляем завершающий символ перевода строки '\n' */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        /* Завершение ввода, если в начале строки введена точка */
        if (buffer[0] == '.') {
            break;
        }

        append(&head, buffer);
    }

    printf("\n--- Результат ---\n");
    print_list(head);

    free_list(head);
    return EXIT_SUCCESS;
}
