#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN_PAGE_SIZE 2
#define MAX_PAGE_SIZE 64
#define MIN_MEMORY_SIZE 128
#define MAX_MEMORY_SIZE 16384

void logexit(const char* msg){
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}

char *algorithm, *logFileName;
int numPages,pageSize,memorySize,operations,reads,writes,hits,faults,s,validPages,dirtyWritten;

typedef struct {
    int page;
    int valid;
    int dirty;
} PageTableEntry;
PageTableEntry* pageTable;

typedef struct {
    int id; //id representa a posição na tabela pageTable
    int page;
    Node* next;
    Node* prev;
} Node;

typedef struct {
    Node* first;
    Node* last;
} List;
List replace_list;

int findPage(page){
    for(int i = 0; i < numPages; i++){
        if(pageTable[i].page == page){
            return i;
        }
    }
    return -1;
}

void updateReplaceList(int page,int id){
    if(strcmp(algorithm,"fifo") == 0){
        //atualiza fila(adiciona nó no final)
        Node* node = (Node*) malloc(sizeof(Node));
        node->next = NULL;
        node->prev = replace_list.last;
        node->page = page;
        node->id = id;
        if(replace_list.last != NULL){
            replace_list.last->next = node;
        }
        replace_list.last = node;
        if(replace_list.first == NULL){
            replace_list.first = node;
        }
    }
    else if(strcmp(algorithm,"lru") == 0){
        //atualizar fila
        //primeiro verifica se o nó está na fila e remove
        //depois adiciona nó no início)
        Node* iter = replace_list.first;
        Node* toDelete = NULL;
        while(iter != NULL){
            if(iter->page == page){
                if(iter->prev == NULL){ //se iter é o primeiro nó
                    break;
                }
                else if(iter->next == NULL){ //se iter é o último nó
                    iter->prev->next = NULL;
                    replace_list.last = iter->prev;
                    toDelete = iter;
                }
                else{ //se iter é um nó do meio
                    iter->prev->next = iter->next;
                    iter->next->prev = iter->prev;
                    toDelete = iter;
                }
                if (toDelete) {
                    free(toDelete);
                    toDelete = NULL;
                }
            }
        }
        Node* node = (Node*) malloc(sizeof(Node));
        node->next = replace_list.first;
        node->prev = NULL;
        node->page = page;
        node->id = id;
        if(replace_list.first != NULL){
            replace_list.first->prev = node;
        }
        replace_list.first = node;
        if(replace_list.last == NULL){
            replace_list.last = node;
        }
    }
}

void removeFromList(){
    if(strcmp(algorithm,"fifo") == 0){
        //remove first
    }
    else if(strcmp(algorithm,"lru") == 0){
        //remove first
    }
}

void accessPage(int page,char rw){
    int dirty;
    if(rw == 'W' || rw == 'w'){
        dirty = 1;
    }
    else{
        dirty = 0;
    }
    int inMemory = findPage(page);
    if(inMemory == -1){
        faults++;
        if(validPages < numPages){
            //adiciona página na tabela na posição validPages
            pageTable[validPages].page = page;
            pageTable[validPages].valid = 1;
            pageTable[validPages].dirty = dirty;
            updateReplaceList(page,validPages);
            validPages++;
        }
        else{
            faults++;
            //adiciona página na tabela/replace usando o algoritmo escolhido
            if(strcmp(algorithm,"fifo") == 0){
                //replace_fifo(page);
                if(pageTable[replace_list.first->id].dirty == 1){
                    dirtyWritten++;
                }
                pageTable[replace_list.first->id].page = page;
                pageTable[replace_list.first->id].dirty = dirty;
                pageTable[replace_list.first->id].valid = 1;
                removeFromList();
            }
            else if(strcmp(algorithm,"2a") == 0){
                replace_2a(page);
            }
            else if(strcmp(algorithm,"lru") == 0){
                replace_lru(page);
            }
            else if(strcmp(algorithm,"random") == 0){
                replace_random(page);
            }
        }
    }else{
        hits++;
        if(strcmp(algorithm,"lru") == 0){
            updateReplaceList(page,inMemory);
        }
    }
    
}

int main(int argc, char** argv){
    operations = 0;
    reads = 0;
    writes = 0;
    hits = 0;
    faults = 0;
    s = 0;
    validPages = 0;
    dirtyWritten = 0;
    replace_list.first = NULL;
    replace_list.last = NULL;

    if (argc != 5) {
        logexit("Usage: tp2virtual <algorithm> <arquivo.log> <pageSize> <memorySize>\n");
    }
    
    algorithm = argv[1];
    if (strcmp(algorithm, "lru") != 0 && strcmp(algorithm, "2a") != 0 &&
        strcmp(algorithm, "fifo") != 0 && strcmp(algorithm, "random") != 0){
        logexit("Invalid algorithm. Please use 'lru', '2a', 'fifo' or 'random'.\n");
    }
    logFileName = argv[2];
    pageSize = atoi(argv[3]);
    if (pageSize < MIN_PAGE_SIZE || pageSize > MAX_PAGE_SIZE) {
        logexit("Invalid page size. Please use a value between 2 and 64.\n");
    }
    memorySize = atoi(argv[4]);
    if (memorySize < MIN_MEMORY_SIZE || memorySize > MAX_MEMORY_SIZE) {
        logexit("Invalid memory size. Please use a value between 128 and 16384.\n");
    }
    numPages = memorySize / pageSize;

    FILE* file = fopen(logFileName, "r");
    if (file == NULL) {
        logexit("Error opening file.\n");
    }

    /* Derivar o valor de s: */
    int tmp = pageSize;
    while (tmp>1) {
        tmp = tmp>>1;
        s++;
    }

    pageTable = malloc(sizeof(PageTableEntry) * numPages);
    for(int i = 0; i < numPages; i++){
        pageTable[i].valid = 0;
        pageTable[i].dirty = 0;
    }

    unsigned int addr, page;
    char rw;
    while(fscanf(file, "%x %c", &addr, &rw) != EOF) {
        operations++;
        page = addr >> s;
        accessPage(page,rw);


        /*if(rw == 'W' || rw == 'w'){
            writePage(page);
        }
        else if(rw == 'R' || rw == 'r'){
            //read
        }
        else{
            logexit("Error. invalid entry format.");
        }*/
    }
    fclose(file);

    printf("\nExecutando o simulador...\n");
    printf("Arquivo de entrada: %s\n", logFileName);
	printf("Tamanho da memória: %i KB\n", memorySize);
	printf("Tamanho das páginas: %i KB\n", pageSize);
	printf("Técnica de reposição: %s\n", algorithm);
    printf("Número de páginas: %i\n", numPages);
	printf("Número total de acessos à memória contidos no arquivo %i\n", operations);
	printf("Número de operações de leitura: %i\n", reads);
	printf("Número de operações de escrita: %i\n", writes);
    printf("Número de páginas sujas escritas de volta no disco: %i\n", dirtyWritten);
	printf("Número de page hits: %i\n", hits);
	printf("Numero de page faults: %f%% \n", faults);
}