#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024

struct Node {
    char *data;        
    struct Node *next;  
};

// добавление строки
void append(struct Node **head, const char *str) {
    // выделяем память под узел 
    struct Node *new_node = malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // память
    new_node->data = malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->data, str);

    // новый узел пока последний
    new_node->next = NULL;

    // если список пуст новый узел становится головой
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // иначе ищем последний узел и присоединяем
    struct Node *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new_node;
}

// освобождаем память, сначала строку, потом узел
void free_list(struct Node *head) {
    struct Node *cur = head;
    while (cur != NULL) {
        struct Node *next = cur->next;  /* сохраняем next */
        free(cur->data);                /* сначала строку */
        free(cur);                      /* потом узел */
        cur = next;
    }
}

//Печатаем все строки списка 
void print_list(struct Node *head) {
    struct Node *cur = head;
    while (cur != NULL) {
        printf("%s\n", cur->data);
        cur = cur->next;
    }
}

int main(void) {
    char buffer[BUFSIZE];
    struct Node *head = NULL;

    printf("Enter lines (end with '.' on a new line):\n");

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;  // EOF
        }

        // удаляем \n
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // если начинается с . завершаем
        if (buffer[0] == '.') {
            break;
        }

        // добавляем строку
        append(&head, buffer);
    }

    printf("\nList contents\n");
    print_list(head);
    free_list(head);

    return 0;
}
