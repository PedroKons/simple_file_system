# 📁 Sistema de Arquivos Virtual com Árvore B

Este projeto é uma implementação de um sistema de arquivos virtual simplificado, desenvolvido como parte da disciplina de **Sistemas Operacionais**. Ele utiliza uma estrutura de dados de **Árvore B** para gerenciar de forma eficiente uma hierarquia de diretórios e arquivos em memória.

## 📜 Sobre o Projeto

O objetivo principal é simular o comportamento básico de um sistema de arquivos de linha de comando, permitindo ao usuário criar e remover arquivos e diretórios, navegar entre eles e listar seus conteúdos. A escolha da Árvore B como estrutura de dados central garante que todas as operações (busca, inserção e remoção) sejam performáticas, com complexidade de tempo logarítmica, mesmo para diretórios com um grande número de entradas.

### Estrutura Principal
- **Hierarquia:** Cada diretório no sistema possui sua própria Árvore B.
- **Nós:** Os nós da árvore contêm ponteiros para arquivos ou outros diretórios.
- **Ordenação:** Todos os itens dentro de um diretório são mantidos em ordem alfabética automaticamente pela Árvore B.

## 🚀 Como Usar

### 1. Pré-requisitos

Para compilar e executar o projeto, você precisa de um compilador C, como o **GCC**.

- Para instalar o GCC em sistemas baseados em Debian/Ubuntu:
  ```bash
  sudo apt-get update
  sudo apt-get install build-essential
  ```

### 2. Compilação

Com o compilador instalado, navegue até a pasta raiz do projeto e execute o seguinte comando para compilar o código. As flags `-Wall` e `-Wextra` são usadas para habilitar avisos que ajudam a garantir a qualidade do código.

```bash
gcc -o fs filesystem.c main_fs.c -Wall -Wextra
```

Este comando irá gerar um arquivo executável chamado `fs`.

### 3. Execução

Para iniciar o sistema de arquivos virtual, execute o arquivo recém-criado:

```bash
./fs
```

Você será recebido por uma mensagem de boas-vindas e uma lista de comandos disponíveis. O prompt de comando mostrará o seu diretório atual, começando pela raiz (`/`).

```
🗂️  Sistema de Arquivos Virtual Iniciado!

=== SISTEMA DE ARQUIVOS VIRTUAL ===
Comandos disponíveis:
  help          - Mostra esta ajuda
  ls            - Lista conteúdo do diretório atual
  mkdir <nome>  - Cria um novo diretório
  ...
=====================================

/$
```

## ⌨️ Comandos Disponíveis

Aqui estão todos os comandos que você pode usar no shell interativo:

| Comando                               | Descrição                                                     | Exemplo de Uso                                    |
| ------------------------------------- | ------------------------------------------------------------- | ------------------------------------------------- |
| `help`                                | Mostra a lista de todos os comandos disponíveis.              | `/$ help`                                         |
| `ls`                                  | Lista os arquivos e subdiretórios no diretório atual.         | `/$ ls`                                           |
| `mkdir <nome>`                        | Cria um novo diretório com o `<nome>` especificado.           | `/$ mkdir Documentos`                             |
| `rmdir <nome>`                        | Remove um diretório, **somente se ele estiver vazio**.          | `/$ rmdir Imagens`                                |
| `touch <nome>`                        | Cria um arquivo de texto vazio com o `<nome>` especificado.   | `/$ touch notas.txt`                              |
| `echo <nome> "<conteudo>"`            | Cria um arquivo de texto com `<nome>` e insere o `<conteudo>`. | `/$ echo hello.txt "Olá, mundo!"`                 |
| `rm <nome>`                           | Remove um arquivo de texto.                                   | `/$ rm notas.txt`                                 |
| `cat <nome>`                          | Exibe o conteúdo de um arquivo de texto.                      | `/$ cat hello.txt`                                |
| `pwd`                                 | Mostra o caminho absoluto do diretório atual.                 | `/Documentos$ pwd`                                |
| `cd <nome>`                           | Navega para o subdiretório `<nome>`.                          | `/$ cd Documentos`                                |
| `cd ..`                               | Volta para o diretório pai.                                   | `/Documentos$ cd ..`                              |
| `save`                                | Salva o estado atual do sistema de arquivos em `fs.img`.      | `/$ save`                                         |
| `exit`                                | Salva o estado atual e encerra o programa.                    | `/$ exit`                                         |

---
Feito com ❤️ por `Pedro Kons e Guilherme Thommy`. 