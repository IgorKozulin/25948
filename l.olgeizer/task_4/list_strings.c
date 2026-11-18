#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }

    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc data");
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

void print_first_and_last(const Node *head) {
    if (head == NULL) {
        return;
    }

    printf("%s\n", head->data);

    const Node *current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    if (current != head) {
        printf("%s\n", current->data);
    }
}

void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}

int main(void) {
    char buffer[BUFFER_SIZE];
    Node *head = NULL;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (buffer[0] == '.') {
            break;
        }

        append(&head, buffer);
    }

    print_first_and_last(head);
    free_list(head);

    return EXIT_SUCCESS;
}
