#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_ELEMENTS 118

typedef struct Element {
    int atomic_number;
    char symbol[3];
    char name[30];
    double atomic_mass;
} Element;


typedef struct Node {
    int num_keys;
    Element **elements;
    struct Node **children;
    struct Node *next;
    int is_leaf;
} Node;

Node *root = NULL;

//Functions used:
void insert(Element *element);
Element *create_element(int atomic_number, const char *symbol, const char *name, double atomic_mass);
Node *create_node(int is_leaf);
void split_node(Node *parent, int index, Node *child);
Element *search(int atomic_number);
void delete_adjust(Node *parent);
void delete(int atomic_number);
void range_search(int lower, int upper);
void print_tree(Node *node);
void display_s_block_elements(Node *node);
void display_p_block_elements(Node *node);
void display_d_block_elements(Node *node);
void display_f_block_elements(Node *node);


void initialize_tree_from_file(const char *filename) {
    FILE *file = fopen("elements.txt", "r");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return;
    }

    int atomic_number;
    char symbol[3];
    char name[30];
    double atomic_mass;

    while (fscanf(file, "%d %s %s %lf", &atomic_number, symbol, name, &atomic_mass) == 4) {
        insert(create_element(atomic_number, symbol, name, atomic_mass));
    }

    fclose(file);
}


Element *create_element(int atomic_number, const char *symbol, const char *name, double atomic_mass) {
    Element *element = (Element *)malloc(sizeof(Element));
    element->atomic_number = atomic_number;
    strcpy(element->symbol, symbol);
    strcpy(element->name, name);
    element->atomic_mass = atomic_mass;
    return element;
}


Node *create_node(int is_leaf) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->num_keys = 0;
    node->elements = (Element **)malloc(MAX_ELEMENTS * sizeof(Element *));
    node->children = (Node **)malloc((MAX_ELEMENTS + 1) * sizeof(Node *));
    node->next = NULL;
    node->is_leaf = is_leaf;
    return node;
}


void split_node(Node *parent, int index, Node *child) {
    Node *new_node = create_node(child->is_leaf);
    new_node->num_keys = MAX_ELEMENTS / 2;
    
    for (int i = 0; i < new_node->num_keys; i++) {
        new_node->elements[i] = child->elements[MAX_ELEMENTS / 2 + i];
    }
    
    if (!child->is_leaf) {
        for (int i = 0; i <= new_node->num_keys; i++) {
            new_node->children[i] = child->children[MAX_ELEMENTS / 2 + i];
        }
    }
    
    child->num_keys = MAX_ELEMENTS / 2;
    
    for (int i = parent->num_keys; i >= index + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = new_node;
    
    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->elements[i + 1] = parent->elements[i];
    }
    parent->elements[index] = child->elements[MAX_ELEMENTS / 2 - 1];
    
    parent->num_keys++;
}


void insert(Element *element) {
    if (root == NULL) {
        root = create_node(1);
        root->elements[0] = element;
        root->num_keys = 1;
    } else {
        if (root->num_keys == MAX_ELEMENTS) {
            Node *new_root = create_node(0);
            new_root->children[0] = root;
            split_node(new_root, 0, root);
            root = new_root;
        }
        Node *cur = root;
        while (!cur->is_leaf) {
            int i = cur->num_keys - 1;
            while (i >= 0 && element->atomic_number < cur->elements[i]->atomic_number) {
                i--;
            }
            i++;
            if (cur->children[i]->num_keys == MAX_ELEMENTS) {
                split_node(cur, i, cur->children[i]);
                if (element->atomic_number > cur->elements[i]->atomic_number) {
                    i++;
                }
            }
            cur = cur->children[i];
        }
        int i = cur->num_keys - 1;
        while (i >= 0 && element->atomic_number < cur->elements[i]->atomic_number) {
            cur->elements[i + 1] = cur->elements[i];
            i--;
        }
        cur->elements[i + 1] = element;
        cur->num_keys++;
    }
}


Element *search(int atomic_number) {
    Node *cur = root;
    while (cur != NULL) {
        int i = 0;
        while (i < cur->num_keys && atomic_number > cur->elements[i]->atomic_number) {
            i++;
        }
        if (i < cur->num_keys && atomic_number == cur->elements[i]->atomic_number) {
            return cur->elements[i];
        }
        if (cur->is_leaf) {
            return NULL;
        }
        cur = cur->children[i];
    }
    return NULL;
}


void delete_adjust(Node *parent) {
    Node *cur = root;
    Node *left_sibling = NULL;
    Node *right_sibling = NULL;
    int index = 0;

    
    while (index <= parent->num_keys && parent->children[index] != NULL) {
        if (parent->children[index] == parent) {
            break;
        }
        index++;
    }
    
    if (index > 0) {
        left_sibling = parent->children[index - 1];
    }

    
    if (index < parent->num_keys) {
        right_sibling = parent->children[index + 1];
    }

    // Try redistributing keys from left sibling
    if (left_sibling && left_sibling->num_keys > (MAX_ELEMENTS + 1) / 2) {
        for (int i = cur->num_keys; i > 0; i--) {
            cur->elements[i] = cur->elements[i - 1];
        }
        cur->elements[0] = left_sibling->elements[left_sibling->num_keys - 1];
        if (!cur->is_leaf) {
            for (int i = cur->num_keys + 1; i > 0; i--) {
                cur->children[i] = cur->children[i - 1];
            }
            cur->children[0] = left_sibling->children[left_sibling->num_keys];
        }
        cur->num_keys++;
        left_sibling->num_keys--;
        parent->elements[index - 1] = cur->elements[0];
    }
    
    else if (right_sibling && right_sibling->num_keys > (MAX_ELEMENTS + 1) / 2) {
        cur->elements[cur->num_keys] = right_sibling->elements[0];
        if (!cur->is_leaf) {
            cur->children[cur->num_keys + 1] = right_sibling->children[0];
        }
        cur->num_keys++;
        right_sibling->num_keys--;
        parent->elements[index] = right_sibling->elements[0];
        for (int i = 0; i < right_sibling->num_keys; i++) {
            right_sibling->elements[i] = right_sibling->elements[i + 1];
        }
        if (!right_sibling->is_leaf) {
            for (int i = 0; i <= right_sibling->num_keys; i++) {
                right_sibling->children[i] = right_sibling->children[i + 1];
            }
        }
    }
    
    else if (left_sibling) {
        left_sibling->elements[left_sibling->num_keys] = cur->elements[0];
        for (int i = 0; i < cur->num_keys; i++) {
            left_sibling->elements[left_sibling->num_keys + 1 + i] = cur->elements[i + 1];
        }
        if (!cur->is_leaf) {
            for (int i = 0; i <= cur->num_keys; i++) {
                left_sibling->children[left_sibling->num_keys + 1 + i] = cur->children[i];
            }
        }
        left_sibling->num_keys += (1 + cur->num_keys);
        for (int i = index; i < parent->num_keys - 1; i++) {
            parent->elements[i] = parent->elements[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->num_keys--;
        free(cur);
    }
    
    else if (right_sibling) {
        cur->elements[cur->num_keys] = right_sibling->elements[0];
        for (int i = 0; i < right_sibling->num_keys; i++) {
            cur->elements[cur->num_keys + 1 + i] = right_sibling->elements[i + 1];
        }
        if (!right_sibling->is_leaf) {
            for (int i = 0; i <= right_sibling->num_keys; i++) {
                cur->children[cur->num_keys + 1 + i] = right_sibling->children[i];
            }
        }
        cur->num_keys += (1 + right_sibling->num_keys);
        for (int i = index; i < parent->num_keys - 1; i++) {
            parent->elements[i] = parent->elements[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->num_keys--;
        free(right_sibling);
    }

    
    if (parent != root && parent->num_keys < (MAX_ELEMENTS + 1) / 2) {
        delete_adjust(parent);
    }
}


void delete(int atomic_number) {
    if (root == NULL) {
        printf("Tree is empty. Cannot delete.\n");
        return;
    }

    
    Node *cur = root;
    Node *parent = NULL;
    int index;
    while (cur != NULL) {
        index = 0;
        while (index < cur->num_keys && atomic_number > cur->elements[index]->atomic_number) {
            index++;
        }
        if (index < cur->num_keys && atomic_number == cur->elements[index]->atomic_number) {
            break;
        }
        parent = cur;
        cur = cur->is_leaf ? NULL : cur->children[index];
    }

    if (cur == NULL) {
        printf("Element with atomic number %d not found. Cannot delete.\n", atomic_number);
        return;
    }

    
    for (int i = index; i < cur->num_keys - 1; i++) {
        cur->elements[i] = cur->elements[i + 1];
    }
    cur->num_keys--;

    
    if (cur == root || cur->num_keys >= (MAX_ELEMENTS + 1) / 2) {
        return;
    }

    
    Node *left_sibling = NULL;
    if (index > 0) {
        left_sibling = parent->children[index - 1];
    }

    
    Node *right_sibling = NULL;
    if (index < parent->num_keys) {
        right_sibling = parent->children[index + 1];
    }

    
    if (left_sibling && left_sibling->num_keys > (MAX_ELEMENTS + 1) / 2) {
        for (int i = cur->num_keys; i > 0; i--) {
            cur->elements[i] = cur->elements[i - 1];
        }
        cur->elements[0] = left_sibling->elements[left_sibling->num_keys - 1];
        if (!cur->is_leaf) {
            for (int i = cur->num_keys + 1; i > 0; i--) {
                cur->children[i] = cur->children[i - 1];
            }
            cur->children[0] = left_sibling->children[left_sibling->num_keys];
        }
        cur->num_keys++;
        left_sibling->num_keys--;
        parent->elements[index - 1] = cur->elements[0];
    }
    
    else if (right_sibling && right_sibling->num_keys > (MAX_ELEMENTS + 1) / 2) {
        cur->elements[cur->num_keys] = right_sibling->elements[0];
        if (!cur->is_leaf) {
            cur->children[cur->num_keys + 1] = right_sibling->children[0];
        }
        cur->num_keys++;
        right_sibling->num_keys--;
        parent->elements[index] = right_sibling->elements[0];
        for (int i = 0; i < right_sibling->num_keys; i++) {
            right_sibling->elements[i] = right_sibling->elements[i + 1];
        }
        if (!right_sibling->is_leaf) {
            for (int i = 0; i <= right_sibling->num_keys; i++) {
                right_sibling->children[i] = right_sibling->children[i + 1];
            }
        }
    }
    
    else if (left_sibling) {
        left_sibling->elements[left_sibling->num_keys] = cur->elements[0];
        for (int i = 0; i < cur->num_keys; i++) {
            left_sibling->elements[left_sibling->num_keys + 1 + i] = cur->elements[i + 1];
        }
        if (!cur->is_leaf) {
            for (int i = 0; i <= cur->num_keys; i++) {
                left_sibling->children[left_sibling->num_keys + 1 + i] = cur->children[i];
            }
        }
        left_sibling->num_keys += (1 + cur->num_keys);
        for (int i = index; i < parent->num_keys - 1; i++) {
            parent->elements[i] = parent->elements[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->num_keys--;
        free(cur);
    }
    
    else if (right_sibling) {
        cur->elements[cur->num_keys] = right_sibling->elements[0];
        for (int i = 0; i < right_sibling->num_keys; i++) {
            cur->elements[cur->num_keys + 1 + i] = right_sibling->elements[i + 1];
        }
        if (!right_sibling->is_leaf) {
            for (int i = 0; i <= right_sibling->num_keys; i++) {
                cur->children[cur->num_keys + 1 + i] = right_sibling->children[i];
            }
        }
        cur->num_keys += (1 + right_sibling->num_keys);
        for (int i = index; i < parent->num_keys - 1; i++) {
            parent->elements[i] = parent->elements[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->num_keys--;
        free(right_sibling);
    }

    
    if (parent != root && parent->num_keys < (MAX_ELEMENTS + 1) / 2) {
        delete_adjust(parent);
    }
}


void range_search(int lower, int upper) {
    if (root == NULL) {
        printf("Tree is empty. No elements to search.\n");
        return;
    }
    

    Node *cur = root;
    while (!cur->is_leaf) {
        int i = 0;
        while (i < cur->num_keys && lower >= cur->elements[i]->atomic_number) {
            i++;
        }
        cur = cur->children[i];
    }
    
    
    int found = 0;
    while (cur != NULL) {
        for (int i = 0; i < cur->num_keys; i++) {
            if (cur->elements[i]->atomic_number >= lower && cur->elements[i]->atomic_number <= upper) {
                printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n", cur->elements[i]->name,
                       cur->elements[i]->symbol, cur->elements[i]->atomic_number, cur->elements[i]->atomic_mass);
                found = 1;
            }
        }
        cur = cur->next;
    }
    
    if (!found) {
        printf("No elements found in the specified range.\n");
    }
}


void print_tree(Node *node) {
    if (node != NULL) {
        for (int i = 0; i < node->num_keys; i++) {
            printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n", node->elements[i]->name,
                   node->elements[i]->symbol, node->elements[i]->atomic_number, node->elements[i]->atomic_mass);
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                print_tree(node->children[i]);
            }
        }
    }
}


void display_s_block_elements(Node *node) {
    if (node != NULL) {
        for (int i = 0; i < node->num_keys; i++) {
            int atomic_number = node->elements[i]->atomic_number;
            if ((atomic_number >= 1 && atomic_number <= 2) ||  // Hydrogen and Helium
                (atomic_number >= 3 && atomic_number <= 4) ||  // Lithium and Beryllium
                (atomic_number >= 11 && atomic_number <= 12) || // Sodium and Magnesium
                (atomic_number >= 19 && atomic_number <= 20) || // Potassium and Calcium
                (atomic_number >= 37 && atomic_number <= 38) || // Rubidium and Strontium
                (atomic_number >= 55 && atomic_number <= 56) || // Cesium and Barium
                (atomic_number >= 87 && atomic_number <= 88)) { // Francium and Radium
                printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n", 
                       node->elements[i]->name, node->elements[i]->symbol, 
                       node->elements[i]->atomic_number, node->elements[i]->atomic_mass);
            }
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                display_s_block_elements(node->children[i]);
            }
        }
    }
}


void display_p_block_elements(Node *node) {
    if (node != NULL) {
        for (int i = 0; i < node->num_keys; i++) {
            int atomic_number = node->elements[i]->atomic_number;
            if ((atomic_number >=5 && atomic_number<=10) ||         // Boron to neons
                (atomic_number >= 13 && atomic_number <= 18) ||    // Aluminium to Argon
                (atomic_number >= 31 && atomic_number <= 36) ||    // Gallium to Krypton
                (atomic_number >= 49 && atomic_number <= 54) ||    // Indium to Xenon
                (atomic_number >= 81 && atomic_number <= 86) ||    // Thallium to Radon
                (atomic_number >= 113 && atomic_number <= 118)) {  // Nihonium to Oganesson
                printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n",
                       node->elements[i]->name, node->elements[i]->symbol,
                       node->elements[i]->atomic_number, node->elements[i]->atomic_mass);
            }
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                display_p_block_elements(node->children[i]);
            }
        }
    }
}


void display_d_block_elements(Node *node) {
    if (node != NULL) {
        for (int i = 0; i < node->num_keys; i++) {
            int atomic_number = node->elements[i]->atomic_number;
            if ((atomic_number >= 21 && atomic_number <= 30) ||    // Scandium to Zinc
                (atomic_number >= 39 && atomic_number <= 48) ||    // Yttrium to Cadmium
                (atomic_number >=72  && atomic_number<=80) ||   // Lanthanum to Mercury
                (atomic_number >= 104 && atomic_number <= 112)) {  // Dubnium to Copernicium
                printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n",
                       node->elements[i]->name, node->elements[i]->symbol,
                       node->elements[i]->atomic_number, node->elements[i]->atomic_mass);
            }
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                display_d_block_elements(node->children[i]);
            }
        }
    }
}


void display_f_block_elements(Node *node) {
    if (node != NULL) {
        for (int i = 0; i < node->num_keys; i++) {
            int atomic_number = node->elements[i]->atomic_number;
            if ((atomic_number >= 57 && atomic_number <= 71) ||    // Lanthanum to Lutetium
                (atomic_number >= 89 && atomic_number <= 103))   // Actinium to Lawrencium
              {  // Nihonium to Oganesson
                printf("%s (%s) - Atomic Number: %d, Atomic Mass: %.2f\n",
                       node->elements[i]->name, node->elements[i]->symbol,
                       node->elements[i]->atomic_number, node->elements[i]->atomic_mass);
            }
        }
        if (!node->is_leaf) {
            for (int i = 0; i <= node->num_keys; i++) {
                display_f_block_elements(node->children[i]);
            }
        }
    }
}


int main() {
    int choice;
    int atomic_number;
    char symbol[3];
    char name[30];
    double atomic_mass;

    initialize_tree_from_file("elements.txt");
    
    while (1) {
        printf("\nMenu:\n");
        printf("1. Insert an element\n");
        printf("2. Delete an element\n");
        printf("3. Search for an element\n");
        printf("4. Range search for elements\n");
        printf("5. Display S-Block Elements\n");
        printf("6. Display P-Block Elements\n");
        printf("7. Display D-Block Elements\n");
        printf("8. Display F-Block Elements\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                printf("Enter atomic number: ");
                scanf("%d", &atomic_number);
                printf("Enter symbol: ");
                scanf("%s", symbol);
                printf("Enter name: ");
                scanf("%s", name);
                printf("Enter atomic mass: ");
                scanf("%lf", &atomic_mass);
                insert(create_element(atomic_number, symbol, name, atomic_mass));
                printf("Element inserted successfully.\n");
                break;
            case 2:
                printf("Enter the atomic number of the element to delete: ");
                scanf("%d", &atomic_number);
                delete(atomic_number);
                printf("Element deleted successfully.\n");
                break;
            case 3:
                printf("Enter the atomic number of the element to search: ");
                scanf("%d", &atomic_number);
                Element *result = search(atomic_number);
                if (result != NULL) {
                    printf("Element found:\n");
                    printf("Name: %s\n", result->name);
                    printf("Symbol: %s\n", result->symbol);
                    printf("Atomic Mass: %.2f\n", result->atomic_mass);
                } else {
                    printf("Element with atomic number %d not found.\n", atomic_number);
                }
                break;
            case 4:
                printf("Enter the range of atomic numbers (lower and upper bounds): ");
                int lower, upper;
                scanf("%d %d", &lower, &upper);
                range_search(lower, upper);
                break;
            case 5:
                printf("Displaying s-block elements:\n");
                display_s_block_elements(root);
                break;
            case 6:
                printf("Displaying p-block elements:\n");
                display_p_block_elements(root);
                break;
            case 7:
                printf("Displaying d-block elements:\n");
                display_d_block_elements(root);
                break;
            case 8:
                printf("Displaying f-block elements:\n");
                display_f_block_elements(root);
                break;
            case 9:
                printf("Exiting program...\n");
                exit(0);
            default:
                printf("Invalid choice. Please enter a number between 1 and 5.\n");
        }
    }

    return 0;
}

