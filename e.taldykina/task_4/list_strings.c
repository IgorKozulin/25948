 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

void append(struct Node **head, const char *str) {
    struct Node *new_node = malloc(sizeof(struct Node));
    new_node->data = malloc(strlen(str) + 1);
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

void free_list(struct Node *head) {
    struct Node *current = head;
    while (current != NULL) {
        struct Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}

int main() {
    struct Node *head = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.') {
            break;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) > 0) {
            append(&head, buffer);
        }
    }

    struct Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    free_list(head);
    return 0;
}
