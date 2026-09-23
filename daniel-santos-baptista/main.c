#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define TAM_MEMORIA (16 * 1024)

_Alignas(max_align_t)
static unsigned char memoria[TAM_MEMORIA];

typedef struct Bloco {
    size_t tamanho;
    int livre;
    struct Bloco *proximo;
} Bloco;

static Bloco *inicioMemoria = NULL;

static size_t alinhar(size_t tamanho) {
    size_t alinhamento = _Alignof(max_align_t);

    return ((tamanho + alinhamento - 1) / alinhamento) * alinhamento;
}

void inicializaMemoria(void) {
    inicioMemoria = (Bloco *) memoria;

    inicioMemoria->tamanho = TAM_MEMORIA - sizeof(Bloco);
    inicioMemoria->livre = 1;
    inicioMemoria->proximo = NULL;
}

void *aloca(size_t tamanho) {
    if (tamanho == 0) {
        return NULL;
    }

    tamanho = alinhar(tamanho);

    Bloco *atual = inicioMemoria;

    while (atual != NULL) {

        if (atual->livre && atual->tamanho >= tamanho) {

            /*
             * Se houver espaço suficiente,
             * divide o bloco atual em dois.
             */
            if (atual->tamanho >= tamanho + sizeof(Bloco) + 1) {

                Bloco *novoBloco =
                    (Bloco *)((unsigned char *)(atual + 1) + tamanho);

                novoBloco->tamanho =
                    atual->tamanho - tamanho - sizeof(Bloco);

                novoBloco->livre = 1;
                novoBloco->proximo = atual->proximo;

                atual->tamanho = tamanho;
                atual->proximo = novoBloco;
            }

            atual->livre = 0;

            /*
             * Retorna o endereço logo depois
             * do cabeçalho do bloco.
             */
            return (void *)(atual + 1);
        }

        atual = atual->proximo;
    }

    return NULL;
}

void libera(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    /*
     * O cabeçalho está imediatamente antes
     * da memória entregue ao usuário.
     */
    Bloco *bloco = ((Bloco *)ptr) - 1;

    bloco->livre = 1;

    /*
     * Junta blocos livres vizinhos.
     */
    Bloco *atual = inicioMemoria;

    while (atual != NULL && atual->proximo != NULL) {

        if (atual->livre && atual->proximo->livre) {

            atual->tamanho +=
                sizeof(Bloco) + atual->proximo->tamanho;

            atual->proximo = atual->proximo->proximo;

        } else {
            atual = atual->proximo;
        }
    }
}

typedef struct No {
    int valor;
    struct No *anterior;
    struct No *proximo;
} No;

void inserirFim(No **inicio, int valor) {
    No *novo = aloca(sizeof(No));

    if (novo == NULL) {
        printf("Erro ao alocar memoria.\n");
        exit(1);
    }

    novo->valor = valor;
    novo->proximo = NULL;
    novo->anterior = NULL;

    if (*inicio == NULL) {
        *inicio = novo;
        return;
    }

    No *atual = *inicio;

    while (atual->proximo != NULL) {
        atual = atual->proximo;
    }

    atual->proximo = novo;
    novo->anterior = atual;
}

void mostrarInicioFim(No *inicio) {
    No *atual = inicio;

    printf("Lista do inicio para o fim: ");

    while (atual != NULL) {
        printf("%d ", atual->valor);
        atual = atual->proximo;
    }

    printf("\n");
}

void mostrarFimInicio(No *inicio) {
    if (inicio == NULL) {
        return;
    }

    No *atual = inicio;

    while (atual->proximo != NULL) {
        atual = atual->proximo;
    }

    printf("Lista do fim para o inicio: ");

    while (atual != NULL) {
        printf("%d ", atual->valor);
        atual = atual->anterior;
    }

    printf("\n");
}

void liberarLista(No *inicio) {
    No *atual = inicio;

    while (atual != NULL) {
        No *proximo = atual->proximo;
        libera(atual);
        atual = proximo;
    }
}

int main(void) {
	inicializaMemoria();
    No *inicio = NULL;

    inserirFim(&inicio, 10);
    inserirFim(&inicio, 20);
    inserirFim(&inicio, 30);
    inserirFim(&inicio, 40);

    mostrarInicioFim(inicio);
    mostrarFimInicio(inicio);

    liberarLista(inicio);

    return 0;
}
