#include "filesystem.h"

static BTreeNode *btree_create_node(int leaf) {
    BTreeNode *node = (BTreeNode *)calloc(1, sizeof(BTreeNode));
    node->leaf = leaf;
    return node;
}

static int compare_names(const char *a, const char *b) {
    return strcmp(a, b);
}

static void btree_split_child(BTreeNode *x, int idx) {
    BTreeNode *y = x->children[idx];
    int t = BTREE_ORDER;

    BTreeNode *z = btree_create_node(y->leaf);
    z->num_keys = t - 1;

    for (int j = 0; j < t - 1; ++j)
        z->keys[j] = y->keys[j + t];

    if (!y->leaf) {
        for (int j = 0; j < t; ++j)
            z->children[j] = y->children[j + t];
    }

    y->num_keys = t - 1;

    for (int j = x->num_keys; j >= idx + 1; --j)
        x->children[j + 1] = x->children[j];
    x->children[idx + 1] = z;

    for (int j = x->num_keys - 1; j >= idx; --j)
        x->keys[j + 1] = x->keys[j];

    x->keys[idx] = y->keys[t - 1];
    x->num_keys += 1;
}

static void btree_insert_nonfull(BTreeNode *x, TreeNode *k) {
    int i = x->num_keys - 1;
    if (x->leaf) {
        while (i >= 0 && compare_names(k->name, x->keys[i]->name) < 0) {
            x->keys[i + 1] = x->keys[i];
            --i;
        }
        x->keys[i + 1] = k;
        x->num_keys += 1;
    } else {
        while (i >= 0 && compare_names(k->name, x->keys[i]->name) < 0)
            --i;
        ++i;
        if (x->children[i]->num_keys == 2 * BTREE_ORDER - 1) {
            btree_split_child(x, i);
            if (compare_names(k->name, x->keys[i]->name) > 0)
                ++i;
        }
        btree_insert_nonfull(x->children[i], k);
    }
}

static TreeNode *btree_search_node(BTreeNode *x, const char *name) {
    int i = 0;
    while (i < x->num_keys && compare_names(name, x->keys[i]->name) > 0)
        ++i;

    if (i < x->num_keys && compare_names(name, x->keys[i]->name) == 0)
        return x->keys[i];

    if (x->leaf)
        return NULL;
    return btree_search_node(x->children[i], name);
}

static int btree_find_key(BTreeNode *x, const char *name) {
    int idx = 0;
    while (idx < x->num_keys && compare_names(x->keys[idx]->name, name) < 0)
        ++idx;
    return idx;
}

static void btree_remove_from_leaf(BTreeNode *x, int idx) {
    for (int i = idx + 1; i < x->num_keys; ++i)
        x->keys[i - 1] = x->keys[i];
    x->num_keys -= 1;
}

static int btree_delete_node(BTreeNode *x, const char *name) {
    int idx = btree_find_key(x, name);

    if (idx < x->num_keys && compare_names(x->keys[idx]->name, name) == 0) {
        if (x->leaf) {
            btree_remove_from_leaf(x, idx);
            return 1;
        } else {
            fprintf(stderr, "Remoção de diretórios com filhos ainda não suportada (somente folhas).\n");
            return 0;
        }
    }

    if (x->leaf) return 0;
    return btree_delete_node(x->children[idx], name);
}

static void btree_traverse_node(BTreeNode *x, int depth) {
    if (!x) return;
    for (int i = 0; i < x->num_keys; ++i) {
        if (!x->leaf) btree_traverse_node(x->children[i], depth + 1);
        for (int d = 0; d < depth; ++d) printf("    ");
        printf("%s%s\n", x->keys[i]->name,
               x->keys[i]->type == DIRECTORY_TYPE ? "/" : "");
    }
    if (!x->leaf) btree_traverse_node(x->children[x->num_keys], depth + 1);
}

BTree *btree_create() {
    BTree *tree = (BTree *)calloc(1, sizeof(BTree));
    tree->root = btree_create_node(1);
    return tree;
}

void btree_insert(BTree *tree, TreeNode *node) {
    if (!tree || !node) return;
    BTreeNode *r = tree->root;
    if (r->num_keys == 2 * BTREE_ORDER - 1) {
        BTreeNode *s = btree_create_node(0);
        tree->root = s;
        s->children[0] = r;
        btree_split_child(s, 0);
        btree_insert_nonfull(s, node);
    } else {
        btree_insert_nonfull(r, node);
    }
}

TreeNode *btree_search(BTree *tree, const char *name) {
    if (!tree || !tree->root) return NULL;
    return btree_search_node(tree->root, name);
}

void btree_delete(BTree *tree, const char *name) {
    if (!tree) return;
    int removed = btree_delete_node(tree->root, name);
    if (!removed)
        printf("Item '%s' não encontrado para remoção.\n", name);
    if (tree->root->num_keys == 0 && !tree->root->leaf) {
        BTreeNode *old_root = tree->root;
        tree->root = old_root->children[0];
        free(old_root);
    }
}

void btree_traverse(BTree *tree) {
    if (!tree || !tree->root) return;
    btree_traverse_node(tree->root, 0);
}

TreeNode *create_txt_file(const char *name, const char *content) {
    File *file = (File *)calloc(1, sizeof(File));
    file->name = strdup(name);
    file->content = strdup(content);
    file->size = strlen(content);

    TreeNode *node = (TreeNode *)calloc(1, sizeof(TreeNode));
    node->name = strdup(name);
    node->type = FILE_TYPE;
    node->data.file = file;
    return node;
}

TreeNode *create_directory(const char *name) {
    Directory *dir = (Directory *)calloc(1, sizeof(Directory));
    dir->tree = btree_create();

    TreeNode *node = (TreeNode *)calloc(1, sizeof(TreeNode));
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;
    node->data.directory = dir;
    return node;
}

static void free_tree_node(TreeNode *node) {
    if (!node) return;
    if (node->type == FILE_TYPE) {
        free(node->data.file->content);
        free(node->data.file->name);
        free(node->data.file);
    } else {
        btree_traverse(node->data.directory->tree);
    }
    free(node->name);
    free(node);
}

void delete_txt_file(BTree *tree, const char *name) {
    TreeNode *target = btree_search(tree, name);
    if (!target || target->type != FILE_TYPE) {
        printf("Arquivo '%s' não encontrado.\n", name);
        return;
    }
    btree_delete(tree, name);
    free_tree_node(target);
    printf("Arquivo '%s' removido com sucesso.\n", name);
}

void delete_directory(BTree *tree, const char *name) {
    TreeNode *target = btree_search(tree, name);
    if (!target || target->type != DIRECTORY_TYPE) {
        printf("Diretório '%s' não encontrado.\n", name);
        return;
    }
    if (target->data.directory->tree->root->num_keys > 0) {
        printf("Diretório '%s' não está vazio – remova conteúdo primeiro.\n", name);
        return;
    }
    btree_delete(tree, name);
    free_tree_node(target);
    printf("Diretório '%s' removido com sucesso.\n", name);
}

Directory *get_root_directory() {
    Directory *root = (Directory *)calloc(1, sizeof(Directory));
    root->tree = btree_create();
    return root;
}

void change_directory(Directory **current, const char *path) {
    if (!current || !*current) return;
    if (strcmp(path, ".") == 0) {
        return;
    }
    if (strcmp(path, "..") == 0) {
        printf("cd .. não suportado nesta versão.\n");
        return;
    }
    TreeNode *dest = btree_search((*current)->tree, path);
    if (!dest || dest->type != DIRECTORY_TYPE) {
        printf("Diretório '%s' não encontrado.\n", path);
        return;
    }
    *current = dest->data.directory;
}

void list_directory_contents(Directory *dir) {
    if (!dir) return;
    btree_traverse(dir->tree);
}
