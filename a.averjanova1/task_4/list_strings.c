#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

void append(struct Node **head, const char *str) {
    struct Node *new_node = malloc(sizeof(struct Node));

    if (new_node == NULL) {
        exit(EXIT_FAILURE);
    }

    new_node->data = malloc(strlen(str) + 1);

    if (new_node->data == NULL) {
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        struct Node *current = *head;

        while (current->next != NULL) {
            current = current->next;
        }

        current->next = new_node;
    }
}

void print_list(struct Node *head) {
    struct Node *current = head;

    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
}

void free_list(struct Node *head) {
    while (head != NULL) {
        struct Node *next = head->next;

        free(head->data);
        free(head);

        head = next;
    }
}

int main() {
    struct Node *head = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.') {
            break;
        }

        size_t length = strlen(buffer);

        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        }

        append(&head, buffer);
    }

    print_list(head);
    free_list(head);

    return 0;
}
