#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

void append(struct Node **head, const char *str) {
    struct Node *new_node = malloc(sizeof(struct Node));
    if (!new_node) exit(EXIT_FAILURE);

    new_node->data = malloc(strlen(str) + 1);
    if (!new_node->data) exit(EXIT_FAILURE);
    
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

void free_list(struct Node **head) {
    struct Node *current = *head;
    while (current != NULL) {
        struct Node *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    *head = NULL;
}

int main() {
    struct Node *head = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.') {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (strlen(buffer) > 0) {
            append(&head, buffer);
        }
    }

    print_list(head);
    free_list(&head);

    return 0;
}
