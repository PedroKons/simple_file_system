#include "filesystem.h"

// Variável global para diretório atual
Directory* current_dir = NULL;
Directory* root_dir = NULL;
char current_path[1024] = "/";

// Stack para navegação (implementação simples)
typedef struct DirStack {
    Directory* dir;
    char path[1024];
    struct DirStack* parent;
} DirStack;

DirStack* dir_stack = NULL;

void print_help() {
    printf("\n=== SISTEMA DE ARQUIVOS VIRTUAL ===\n");
    printf("Comandos disponíveis:\n");
    printf("  help          - Mostra esta ajuda\n");
    printf("  ls            - Lista conteúdo do diretório atual\n");
    printf("  mkdir <nome>  - Cria um novo diretório\n");
    printf("  rmdir <nome>  - Remove um diretório vazio\n");
    printf("  touch <nome>  - Cria um arquivo vazio\n");
    printf("  echo <nome> <conteudo> - Cria arquivo com conteúdo\n");
    printf("  rm <nome>     - Remove um arquivo\n");
    printf("  cd <nome>     - Navega para um diretório\n");
    printf("  cd ..         - Volta para o diretório anterior\n");
    printf("  pwd           - Mostra diretório atual\n");
    printf("  cat <nome>    - Mostra conteúdo de um arquivo\n");
    printf("  save          - Salva sistema em fs.img\n");
    printf("  exit          - Sair do programa\n");
    printf("=====================================\n\n");
}

// Função recursiva para salvar a árvore
void save_tree_recursive(FILE* file, BTreeNode* node, int depth) {
    if (!node) return;
    
    for (int i = 0; i < node->num_keys; ++i) {
        if (!node->leaf && i < node->num_keys) {
            save_tree_recursive(file, node->children[i], depth + 1);
        }
        
        // Imprime indentação
        for (int d = 0; d < depth; ++d) {
            fprintf(file, "    ");
        }
        
        TreeNode* item = node->keys[i];
        if (item->type == DIRECTORY_TYPE) {
            fprintf(file, "├── %s/\n", item->name);
            // Salva conteúdo do diretório recursivamente
            save_tree_recursive(file, item->data.directory->tree->root, depth + 1);
        } else {
            fprintf(file, "├── %s (%zu bytes)\n", item->name, item->data.file->size);
            // Opcionalmente, pode salvar o conteúdo do arquivo
            if (strlen(item->data.file->content) > 0) {
                for (int d = 0; d <= depth; ++d) {
                    fprintf(file, "    ");
                }
                fprintf(file, "│   Conteúdo: %s\n", item->data.file->content);
            }
        }
    }
    
    if (!node->leaf && node->num_keys > 0) {
        save_tree_recursive(file, node->children[node->num_keys], depth + 1);
    }
}

void save_filesystem(Directory* root) {
    FILE* img = fopen("fs.img", "w");
    if (img) {
        fprintf(img, "=== SISTEMA DE ARQUIVOS VIRTUAL ===\n");
        fprintf(img, "Estrutura do sistema:\n\n");
        fprintf(img, "ROOT/\n");
        save_tree_recursive(img, root->tree->root, 1);
        fclose(img);
        printf("Sistema de arquivos salvo em fs.img\n");
    } else {
        perror("Erro ao criar fs.img");
    }
}

// Funções para gerenciar stack de diretórios
void push_directory(Directory* dir, const char* path) {
    DirStack* new_entry = (DirStack*)malloc(sizeof(DirStack));
    new_entry->dir = dir;
    strcpy(new_entry->path, path);
    new_entry->parent = dir_stack;
    dir_stack = new_entry;
}

int pop_directory() {
    if (!dir_stack) return 0;
    
    DirStack* top = dir_stack;
    current_dir = top->dir;
    strcpy(current_path, top->path);
    dir_stack = top->parent;
    free(top);
    return 1;
}

void show_file_content(const char* filename) {
    TreeNode* node = btree_search(current_dir->tree, filename);
    if (!node) {
        printf("Arquivo '%s' não encontrado.\n", filename);
        return;
    }
    if (node->type != FILE_TYPE) {
        printf("'%s' não é um arquivo.\n", filename);
        return;
    }
    printf("Conteúdo de %s:\n", filename);
    printf("─────────────────────\n");
    printf("%s\n", node->data.file->content);
    printf("─────────────────────\n");
    printf("Tamanho: %zu bytes\n", node->data.file->size);
}

int main() {
    // Inicialização do sistema
    Directory* root = get_root_directory();
    current_dir = root;
    root_dir = root;
    
    printf("🗂️  Sistema de Arquivos Virtual Iniciado!\n");
    print_help();
    
    char input[256];
    char command[64], arg1[128], arg2[512];
    
    while (1) {
        printf("%s$ ", current_path);
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        // Parse do comando
        memset(command, 0, sizeof(command));
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        
        if (sscanf(input, "%s %s %[^\n]", command, arg1, arg2) < 1) {
            continue;
        }
        
        // Processamento dos comandos
        if (strcmp(command, "help") == 0) {
            print_help();
        }
        else if (strcmp(command, "ls") == 0) {
            printf("Conteúdo de %s:\n", current_path);
            if (current_dir->tree->root->num_keys == 0) {
                printf("  (diretório vazio)\n");
            } else {
                list_directory_contents(current_dir);
            }
            printf("\n");
        }
        else if (strcmp(command, "pwd") == 0) {
            printf("%s\n", current_path);
        }
        else if (strcmp(command, "mkdir") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: mkdir <nome_do_diretorio>\n");
                continue;
            }
            
            // Verifica se já existe
            if (btree_search(current_dir->tree, arg1)) {
                printf("'%s' já existe.\n", arg1);
                continue;
            }
            
            TreeNode* new_dir = create_directory(arg1);
            btree_insert(current_dir->tree, new_dir);
            printf("Diretório '%s' criado com sucesso.\n", arg1);
        }
        else if (strcmp(command, "rmdir") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: rmdir <nome_do_diretorio>\n");
                continue;
            }
            delete_directory(current_dir->tree, arg1);
        }
        else if (strcmp(command, "touch") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: touch <nome_do_arquivo>\n");
                continue;
            }
            
            // Verifica se já existe
            if (btree_search(current_dir->tree, arg1)) {
                printf("'%s' já existe.\n", arg1);
                continue;
            }
            
            TreeNode* new_file = create_txt_file(arg1, "");
            btree_insert(current_dir->tree, new_file);
            printf("Arquivo '%s' criado com sucesso.\n", arg1);
        }
        else if (strcmp(command, "echo") == 0) {
            if (strlen(arg1) == 0 || strlen(arg2) == 0) {
                printf("Uso: echo <nome_do_arquivo> <conteudo>\n");
                continue;
            }
            
            // Verifica se já existe
            if (btree_search(current_dir->tree, arg1)) {
                printf("'%s' já existe.\n", arg1);
                continue;
            }
            
            TreeNode* new_file = create_txt_file(arg1, arg2);
            btree_insert(current_dir->tree, new_file);
            printf("Arquivo '%s' criado com conteúdo.\n", arg1);
        }
        else if (strcmp(command, "rm") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: rm <nome_do_arquivo>\n");
                continue;
            }
            delete_txt_file(current_dir->tree, arg1);
        }
        else if (strcmp(command, "cat") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: cat <nome_do_arquivo>\n");
                continue;
            }
            show_file_content(arg1);
        }
        else if (strcmp(command, "cd") == 0) {
            if (strlen(arg1) == 0) {
                printf("Uso: cd <nome_do_diretorio> ou cd ..\n");
                continue;
            }
            
            if (strcmp(arg1, "..") == 0) {
                if (pop_directory()) {
                    printf("Voltou para %s\n", current_path);
                } else {
                    printf("Já está no diretório raiz.\n");
                }
                continue;
            }
            
            TreeNode* target = btree_search(current_dir->tree, arg1);
            if (!target || target->type != DIRECTORY_TYPE) {
                printf("Diretório '%s' não encontrado.\n", arg1);
                continue;
            }
            
            // Salva diretório atual na stack antes de navegar
            push_directory(current_dir, current_path);
            
            current_dir = target->data.directory;
            // Atualiza o path (simplificado)
            char new_path[1024];
            if (strcmp(current_path, "/") == 0) {
                snprintf(new_path, sizeof(new_path), "/%s", arg1);
            } else {
                snprintf(new_path, sizeof(new_path), "%s/%s", current_path, arg1);
            }
            strcpy(current_path, new_path);
            printf("Navegou para %s\n", current_path);
        }
        else if (strcmp(command, "save") == 0) {
            save_filesystem(root);
        }
        else if (strcmp(command, "exit") == 0) {
            printf("Salvando sistema de arquivos...\n");
            save_filesystem(root);
            printf("Saindo do sistema de arquivos virtual. Até logo! 👋\n");
            break;
        }
        else if (strlen(command) > 0) {
            printf("Comando '%s' não reconhecido. Digite 'help' para ver os comandos disponíveis.\n", command);
        }
    }
    
    return 0;
}
