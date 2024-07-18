#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define A 0
#define T 1
#define G 2
#define C 3
#define sair 12

#define maxSeq 1000
#define MAX_THREADS 8

char baseMapa[5] = {'A', 'T', 'G', 'C', '-'};
char seqMaior[maxSeq], seqMenor[maxSeq], alinhaMaior[MAX_THREADS][maxSeq], alinhaMenor[MAX_THREADS][maxSeq];
int matrizScores[maxSeq + 1][maxSeq + 1];
int tamSeqMaior, tamSeqMenor, tamAlinha[MAX_THREADS], penalGap, grauMuta, diagScore, linScore, colScore;
int matrizPesos[4][4] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
int num_threads = 4;

typedef struct
{
    int thread_id;
    int start_row;
} ThreadData;

typedef struct {
    int thread_id;
    int num_alignments;
    int tbLin;
    int tbCol;
    char* alinhaMaior;
    char* alinhaMenor;
    int* threadDatas;
} AlignmentThreadData;

void mostraAlinhamentoGlobal(int *ptr);
void *traceBackThread(void *arg);

int leTamMaior(void)
{
    printf("\nLeitura do Tamanho da Sequencia Maior:");
    do
    {
        printf("\nDigite 0 < valor < %d = ", maxSeq);
        scanf("%d", &tamSeqMaior);
    } while ((tamSeqMaior < 1) || (tamSeqMaior > maxSeq));
}

int leTamMenor(void)
{
    printf("\nLeitura do Tamanho da Sequencia Menor:");
    do
    {
        printf("\nDigite 0 < valor <= %d = ", tamSeqMaior);
        scanf("%d", &tamSeqMenor);
    } while ((tamSeqMenor < 1) || (tamSeqMenor > tamSeqMaior));
}

int lePenalidade(void)
{
    int penal;
    printf("\nLeitura da Penalidade de Gap:");
    do
    {
        printf("\nDigite valor >= 0 = ");
        scanf("%d", &penal);
    } while (penal < 0);
    return penal;
}

void leMatrizPesos()
{
    int i, j;
    printf("\nLeitura da Matriz de Pesos:\n");
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            printf("Digite valor %c x %c = ", baseMapa[i], baseMapa[j]);
            scanf("%d", &(matrizPesos[i][j]));
        }
        printf("\n");
    }
}

void mostraMatrizPesos(void)
{
    int i, j;
    printf("\nMatriz de Pesos Atual:");
    printf("\n%4c%4c%4c%4c%4c\n", ' ', 'A', 'T', 'G', 'C');
    for (i = 0; i < 4; i++)
    {
        printf("%4c", baseMapa[i]);
        for (j = 0; j < 4; j++)
            printf("%4d", matrizPesos[i][j]);
        printf("\n");
    }
}

int leGrauMutacao(void)
{
    int prob;
    printf("\nLeitura da Porcentagem Maxima de Mutacao Aleatoria:\n");
    do
    {
        printf("\nDigite 0 <= valor <= 100 = ");
        scanf("%d", &prob);
    } while ((prob < 0) || (prob > 100));
    return prob;
}

void leSequenciasDeArquivo(void)
{
    FILE *file;
    char filename[100];

    printf("\nDigite o nome do arquivo para a sequencia maior: ");
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    fscanf(file, "%s", seqMaior);
    tamSeqMaior = strlen(seqMaior);
    fclose(file);

    printf("\nDigite o nome do arquivo para a sequencia menor: ");
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo %s\n", filename);
        exit(1);
    }
    fscanf(file, "%s", seqMenor);
    tamSeqMenor = strlen(seqMenor);
    fclose(file);

    // Convertendo as sequências para os valores internos (0-3)
    for (int i = 0; i < tamSeqMaior; i++)
    {
        switch (seqMaior[i])
        {
        case 'A':
            seqMaior[i] = (char)A;
            break;
        case 'T':
            seqMaior[i] = (char)T;
            break;
        case 'G':
            seqMaior[i] = (char)G;
            break;
        case 'C':
            seqMaior[i] = (char)C;
            break;
        }
    }
    for (int i = 0; i < tamSeqMenor; i++)
    {
        switch (seqMenor[i])
        {
        case 'A':
            seqMenor[i] = (char)A;
            break;
        case 'T':
            seqMenor[i] = (char)T;
            break;
        case 'G':
            seqMenor[i] = (char)G;
            break;
        case 'C':
            seqMenor[i] = (char)C;
            break;
        }
    }
}

void leSequencias(void)
{
    int i, erro;
    printf("\nLeitura das Sequencias:\n");

    do
    {
        printf("\nPara a Sequencia Maior,");
        printf("\nDigite apenas caracteres 'A', 'T', 'G' e 'C'");
        do
        {
            printf("\n> ");
            fgets(seqMaior, maxSeq, stdin);
            tamSeqMaior = strlen(seqMaior) - 1; // remove o enter
        } while (tamSeqMaior < 1);
        printf("\ntamSeqMaior = %d\n", tamSeqMaior);
        i = 0;
        erro = 0;
        do
        {
            switch (seqMaior[i])
            {
            case 'A':
                seqMaior[i] = (char)A;
                break;
            case 'T':
                seqMaior[i] = (char)T;
                break;
            case 'G':
                seqMaior[i] = (char)G;
                break;
            case 'C':
                seqMaior[i] = (char)C;
                break;
            default:
                erro = 1;
            }
            i++;
        } while ((erro == 0) && (i < tamSeqMaior));
    } while (erro == 1);

    do
    {
        printf("\nPara a Sequencia Menor, ");
        printf("\nDigite apenas caracteres 'A', 'T', 'G' e 'C'");
        do
        {
            printf("\n> ");
            fgets(seqMenor, maxSeq, stdin);
            tamSeqMenor = strlen(seqMenor) - 1; // remove o enter
        } while ((tamSeqMenor < 1) || (tamSeqMenor > tamSeqMaior));
        printf("\ntamSeqMenor = %d\n", tamSeqMenor);

        i = 0;
        erro = 0;
        do
        {
            switch (seqMenor[i])
            {
            case 'A':
                seqMenor[i] = (char)A;
                break;
            case 'T':
                seqMenor[i] = (char)T;
                break;
            case 'G':
                seqMenor[i] = (char)G;
                break;
            case 'C':
                seqMenor[i] = (char)C;
                break;
            default:
                erro = 1;
            }
            i++;
        } while ((erro == 0) && (i < tamSeqMenor));
    } while (erro == 1);
}

void geraSequencias(void)
{
    int i, dif, probAux, ind, nTrocas;
    char base;
    srand(time(NULL));
    printf("\nGeracao Aleatoria das Sequencias:\n");

    for (i = 0; i < tamSeqMaior; i++)
    {
        base = (char)(rand() % 4);
        seqMaior[i] = base;
    }

    dif = tamSeqMaior - tamSeqMenor;
    ind = 0;
    if (dif > 0)
        ind = rand() % dif;

    for (i = 0; i < tamSeqMenor; i++)
        seqMenor[i] = seqMaior[ind + i];

    i = 0;
    nTrocas = 0;
    while ((i < tamSeqMenor) && (nTrocas < grauMuta))
    {
        probAux = rand() % 100;
        if (probAux < grauMuta)
        {
            seqMenor[i] = (seqMenor[i] + (rand() % 3) + 1) % 4;
            nTrocas++;
        }
        i++;
    }

    printf("\nSequencias Geradas, Dif = %d Ind = %d\n", dif, ind);
}

void mostraSequencias(void)
{
    int i;
    printf("\nSequencias Atuais:\n");
    printf("\nSequencia Maior, Tam = %d\n", tamSeqMaior);
    printf("%c", baseMapa[(int)seqMaior[0]]);
    for (i = 1; i < tamSeqMaior; i++)
        printf("%c", baseMapa[(int)seqMaior[i]]);
    printf("\n");

    printf("\nSequencia Menor, Tam = %d\n", tamSeqMenor);
    printf("%c", baseMapa[(int)seqMenor[0]]);
    for (i = 1; i < tamSeqMenor; i++)
        printf("%c", baseMapa[(int)seqMenor[i]]);
    printf("\n");
}

void *geraMatrizScoresThread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int lin, col, peso, diagScore, linScore, colScore;

    for (lin = data->start_row; lin <= tamSeqMenor; lin += num_threads)
    {
        for (col = 1; col <= tamSeqMaior; col++)
        {
            peso = matrizPesos[(int)(seqMenor[lin - 1])][(int)(seqMaior[col - 1])];
            diagScore = matrizScores[lin - 1][col - 1] + peso;
            linScore = matrizScores[lin][col - 1] - penalGap;
            colScore = matrizScores[lin - 1][col] - penalGap;

            if ((diagScore >= linScore) && (diagScore >= colScore))
            {
                matrizScores[lin][col] = diagScore;
            }
            else if (linScore > colScore)
            {
                matrizScores[lin][col] = linScore;
            }
            else
            {
                matrizScores[lin][col] = colScore;
            }
        }
    }

    pthread_exit(NULL);
}

void geraMatrizScores(void)
{
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int i, rc;

    printf("\nGeracao da Matriz de Scores:\n");

    for (i = 0; i < num_threads; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].start_row = i + 1; // Start at different rows to make them equidistant

        rc = pthread_create(&threads[i], NULL, geraMatrizScoresThread, (void *)&thread_data[i]);
        if (rc)
        {
            printf("Erro ao criar thread %d: %d\n", i, rc);
            exit(-1);
        }
    }

    for (i = 0; i < num_threads; i++)
    {
        rc = pthread_join(threads[i], NULL);
        if (rc)
        {
            printf("Erro ao esperar thread %d: %d\n", i, rc);
            exit(-1);
        }
    }

    printf("\nMatriz de Scores Gerada.");
}

void mostraMatrizScores()
{
    int i, lin, col;
    printf("\nMatriz de Scores Atual:\n");

    printf("%4c%4c", ' ', ' ');
    for (i = 0; i <= tamSeqMaior; i++)
        printf("%4d", i);
    printf("\n");

    printf("%4c%4c%4c", ' ', ' ', '-');
    for (i = 0; i < tamSeqMaior; i++)
        printf("%4c", baseMapa[(int)(seqMaior[i])]);
    printf("\n");

    printf("%4c%4c", '0', '-');
    for (col = 0; col <= tamSeqMaior; col++)
        printf("%4d", matrizScores[0][col]);
    printf("\n");

    for (lin = 1; lin <= tamSeqMenor; lin++)
    {
        printf("%4d%4c", lin, baseMapa[(int)(seqMenor[lin - 1])]);
        for (col = 0; col <= tamSeqMaior; col++)
        {
            printf("%4d", matrizScores[lin][col]);
        }
        printf("\n");
    }
}

void *traceBackThread(void *arg)
{
    AlignmentThreadData *data = (AlignmentThreadData *)arg;
    int tbLin = data->tbLin, tbCol = data->tbCol;
    int pos = 0, peso, diagScore, linScore, colScore;
    int thread_id = data->thread_id;

    while ((tbLin != 0) && (tbCol != 0))
    {
        peso = matrizPesos[(int)(seqMenor[tbLin - 1])][(int)(seqMaior[tbCol - 1])];
        diagScore = matrizScores[tbLin - 1][tbCol - 1] + peso;
        linScore = matrizScores[tbLin][tbCol - 1] - penalGap;
        colScore = matrizScores[tbLin - 1][tbCol] - penalGap;

        if ((diagScore >= linScore) && (diagScore >= colScore))
        {
            data->alinhaMenor[pos] = seqMenor[tbLin - 1];
            data->alinhaMaior[pos] = seqMaior[tbCol - 1];
            tbLin--;
            tbCol--;
        }
        else if (linScore > colScore)
        {
            data->alinhaMenor[pos] = (char)4;
            data->alinhaMaior[pos] = seqMaior[tbCol - 1];
            tbCol--;
        }
        else
        {
            data->alinhaMenor[pos] = seqMenor[tbLin - 1];
            data->alinhaMaior[pos] = (char)4;
            tbLin--;
        }
        pos++;
    }

    while (tbLin > 0)
    {
        data->alinhaMenor[pos] = seqMenor[tbLin - 1];
        data->alinhaMaior[pos] = (char)4;
        tbLin--;
        pos++;
    }

    while (tbCol > 0)
    {
        data->alinhaMenor[pos] = (char)4;
        data->alinhaMaior[pos] = seqMaior[tbCol - 1];
        tbCol--;
        pos++;
    }

    for (int i = 0; i < (pos / 2); i++)
    {
        int aux = data->alinhaMenor[i];
        data->alinhaMenor[i] = data->alinhaMenor[pos - i - 1];
        data->alinhaMenor[pos - i - 1] = aux;

        aux = data->alinhaMaior[i];
        data->alinhaMaior[i] = data->alinhaMaior[pos - i - 1];
        data->alinhaMaior[pos - i - 1] = aux;
    }

    data->threadDatas[thread_id] = &data;

    pthread_exit(NULL);
}

void traceBack(int k)
{
    pthread_t threads[MAX_THREADS];
    AlignmentThreadData thread_data[k];
    int i, rc;
    int countThreads, countData;

    printf("\nGeracao do Alinhamento Global:\n");

    thread_data[0].thread_id = i;
    thread_data[0].tbLin = tamSeqMenor;
    thread_data[0].tbCol = tamSeqMaior;
    thread_data[0].threadDatas = &thread_data;

    rc = pthread_create(&threads[0], NULL, traceBackThread, (void *)&thread_data[0]);
    if (rc)
    {
        printf("Erro ao criar thread %d: %d\n", 0, rc);
        exit(-1);
    }

    countThreads++;
    countData++;

    for (i = 0; i < k; i++)
    {
        rc = pthread_join(threads[i], NULL);
        if (rc)
        {
            printf("Erro ao esperar thread %d: %d\n", i, rc);
            exit(-1);
        }
    }

    printf("\nAlinhamentos Globais Gerados.");
}

void mostraAlinhamentoGlobal(int *ptr)
{
    int i, j;

    printf("\nAlinhamentos Globais:\n");

    for (j = 0; j < num_threads; j++)
    {
        printf("\nAlinhamento %d:\n", j + 1);

        printf("Sequencia Maior: ");
        for (i = 0; i < tamAlinha[j]; i++)
        {
            printf("%c", baseMapa[(int)ptr->alinhaMaior[j][i]]);
        }
        printf("\n");

        printf("Sequencia Menor: ");
        for (i = 0; i < tamAlinha[j]; i++)
        {
            printf("%c", baseMapa[(int)ptr->alinhaMenor[j][i]]);
        }
        printf("\n");
    }
}

void gravaMatrizScores(void)
{
    FILE *file;
    char filename[100];

    printf("\nDigite o nome do arquivo para salvar a matriz de scores: ");
    scanf("%s", filename);
    file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo %s para escrita\n", filename);
        exit(1);
    }

    fprintf(file, "\nMatriz de Scores Atual:\n");
    fprintf(file, "%4c%4c", ' ', ' ');
    for (int i = 0; i <= tamSeqMaior; i++)
        fprintf(file, "%4d", i);
    fprintf(file, "\n");

    fprintf(file, "%4c%4c%4c", ' ', ' ', '-');
    for (int i = 0; i < tamSeqMaior; i++)
        fprintf(file, "%4c", baseMapa[(int)(seqMaior[i])]);
    fprintf(file, "\n");

    fprintf(file, "%4c%4c", '0', '-');
    for (int col = 0; col <= tamSeqMaior; col++)
        fprintf(file, "%4d", matrizScores[0][col]);
    fprintf(file, "\n");

    for (int lin = 1; lin <= tamSeqMenor; lin++)
    {
        fprintf(file, "%4d%4c", lin, baseMapa[(int)(seqMenor[lin - 1])]);
        for (int col = 0; col <= tamSeqMaior; col++)
        {
            fprintf(file, "%4d", matrizScores[lin][col]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Matriz de Scores salva em %s\n", filename);
}

int leNumThreads(void)
{
    int num;
    printf("\nLeitura do Numero de Threads:");
    do
    {
        printf("\nDigite 1 <= valor <= %d = ", MAX_THREADS);
        scanf("%d", &num);
    } while ((num < 1) || (num > MAX_THREADS));
    return num;
}

int menuOpcao(void)
{
    int op;
    char enter;

    do
    {
        printf("\nMenu de Opcao:");
        printf("\n<01> Ler Matriz de Pesos");
        printf("\n<02> Mostrar Matriz de Pesos");
        printf("\n<03> Ler Penalidade de Gap");
        printf("\n<04> Mostrar Penalidade");
        printf("\n<05> Definir Sequencias Genomicas");
        printf("\n<06> Mostrar Sequencias");
        printf("\n<07> Gerar Matriz de Scores");
        printf("\n<08> Mostrar Matriz de Scores");
        printf("\n<09> Gerar Alinhamento Global");
        printf("\n<10> Mostrar Alinhamento Global");
        printf("\n<11> Salvar Matriz de Scores em Arquivo");
        printf("\n<12> Sair");
        printf("\nDigite a opcao => ");
        scanf("%d", &op);
        scanf("%c", &enter);
    } while ((op < 1) || (op > sair));

    return (op);
}

void trataOpcao(int op)
{
    int resp, k;
    char enter;

    switch (op)
    {
    case 1:
        leMatrizPesos();
        break;
    case 2:
        mostraMatrizPesos();
        break;
    case 3:
        penalGap = lePenalidade();
        break;
    case 4:
        printf("\nPenalidade = %d", penalGap);
        break;
    case 5:
        printf("\nDeseja Definicao: <1>MANUAL, <2>ALEATORIA ou <3>DE ARQUIVO? = ");
        scanf("%d", &resp);
        scanf("%c", &enter); // remove o enter
        if (resp == 1)
        {
            leSequencias();
        }
        else if (resp == 2)
        {
            leTamMaior();
            leTamMenor();
            grauMuta = leGrauMutacao();
            geraSequencias();
        }
        else
        {
            leSequenciasDeArquivo();
        }
        break;
    case 6:
        mostraSequencias();
        break;
    case 7:
        num_threads = leNumThreads();
        geraMatrizScores();
        break;
    case 8:
        mostraMatrizScores();
        break;
    case 9:
        printf("\nDigite o numero de alinhamentos (k) = ");
        scanf("%d", &k);
        traceBack(k);
        break;
    case 10:
        mostraAlinhamentoGlobal(&resp);
        break;
    case 11:
        gravaMatrizScores();
        break;
    case 12:
        exit(0);
        break;
    }
}

void main(void)
{
    int opcao;

    do
    {
        printf("\n\nPrograma Needleman-Wunsch Sequencial\n");
        opcao = menuOpcao();
        trataOpcao(opcao);
    } while (opcao != sair);
}