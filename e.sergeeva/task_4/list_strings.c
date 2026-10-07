#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Ошибка выделения памяти для узла");
        exit(EXIT_FAILURE);
    }

    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("Ошибка выделения памяти для строки");
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

void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next_node = current->next;
        free(current->data);
        free(current);
        current = next_node;
    }
}

int main() {
    Node *head = NULL;
    char buffer[1024];

    printf("Введите строки (точка '.' в начале строки для завершения):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        if (buffer[0] == '.') {
            break;
        }

        if (strlen(buffer) > 0) {
            append(&head, buffer);
        }
    }

    printf("\n--- Вывод списка ---\n");
    Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    free_list(head);

    return 0;
}
