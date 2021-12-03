
/* 

    Sintassi Usata:
        - MPI_Init = Inzializzazione MPI
        - MPI_Comm_size = Determina il numero di processi
        - MPI_Comm_rank = Determina il rank del processo
        - MPI_Finalize = Termina MPI
        
        - MPI_Isend = Invio "Messaggio" con i seguenti parametri
            - buffer = Messaggio da inviare
            - count = Numero di elementi da inviare [Dimensione (per le matrici o per i vettori necessita della dimensione)]
            - datatype = Tipo di dato da inviare [MPI_INT,MPI_DOUBLE,MPI_LONG]
            - dest = Processo destinatario (sarebbe il rank del processo)
            - tag = Tag del messaggio (per sincronizzare i messaggi in arrivo e in uscita) (il tag settato in send deve essere uguale in recive)
            - comm = Communicator (il communicator è una struttura che contiene i processi) (MPI_COMM_WORLD per interaggire con gli altri)
            - request = Risorsa che contiene le informazioni sul messaggio inviato (controllo errori)
        
        - MPI_Recv = Ricezione "Messaggio" con i seguenti parametri
            - buffer = Messaggio ricevuto
            - count = Numero di elementi da ricevere [Dimensione (per le matrici o per i vettori necessita della dimensione)]
            - datatype = Tipo di dato ricevuto [MPI_INT,MPI_DOUBLE,MPI_LONG]
            - source = Processo mittente (sarebbe il rank del processo) (di solito i worker ricevono messaggi solo dal master quindi viene settato a 0)
            - tag = Tag del messaggio (per sincronizzare i messaggi in arrivo e in uscita) (il tag settato in recive deve essere uguale in send)
            - comm = Communicator (il communicator è una struttura che contiene i processi) (MPI_COMM_WORLD per interaggire con gli altri)
            - status = Risorsa che contiene le informazioni sul messaggio ricevuto (controllo errori)
        
        - MPI_Bcast = Invia un messaggio a tutti i processi
            - buffer = Messaggio da inviare
            - count = Numero di elementi da inviare [Dimensione (per le matrici o per i vettori necessita della dimensione)]
            - datatype = Tipo di dato da inviare [MPI_INT,MPI_DOUBLE,MPI_LONG]
            - root = Processo che invia il messaggio (sarebbe il rank del processo)
            - comm = Communicator (il communicator è una struttura che contiene i processi) (MPI_COMM_WORLD per interaggire con gli altri)

        - L'algoritmo è capace di gestire una qualsiasi dimensione di matrice di tipo double

*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N_ROWS 400 // numero righe
#define N_COLS 400 // numero colonne

#define FROM_MASTER_TAG 2 // tag che identifica il messaggio da parte del master
#define FROM_WORKER_TAG 3 // tag che identifica il messaggio da parte del worker

int rank, n_workers; // rank e size sono gli identificatori del processo
size_t i,j,k; // indice per scorrere le matrici

double matrix_a[N_ROWS][N_COLS]; // matrice A
double matrix_b[N_ROWS][N_COLS]; // matrice B
double matrix_final[N_ROWS][N_COLS]; // matrice risultato

double start_time, end_time; // variabili per calcolare il tempo di esecuzione

int avg_rows_per_worker; // numero di righe da assegnare ad ogni worker
int start_row, end_row; // dove il worker deve iniziare e dove deve terminare la sua parte

MPI_Status status; // variabile per la ricezione
MPI_Request request; // variabile per l'invio 

// Funzione dedita al riempimento delle matrici
void Inserimento_Casuali() {

	for (int i=0;i<N_ROWS;i++)
	    for (int j=0;j<N_COLS;j++) {
	         matrix_a[i][j] = rand() % 10; // inserimento casuale in A
             matrix_b[i][j] = rand() % 10; // inserimento casuale in B
	    }

}

int main(int argc,char *argv[]) {

    // inizializzazione MPI - acquisizone del rank e del numero di processi 
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&n_workers);

    // il master ha il rank 0
    if (rank == 0) {

        // inizializzazione della matrice A e B
        Inserimento_Casuali();

        start_time = MPI_Wtime(); // tempo di inizio

        // ci prepariamo per l'invio dei messaggi ad ogni worker
        for (i= 1; i < n_workers; i++) {
            avg_rows_per_worker = N_ROWS / (n_workers-1); // quante righe assegnare ad ogni worker [Non Tenendo conto del nodo master]
            
            /* Tenendo conto che la matrice sia sempre equalmente divisibile :) */
            start_row = (i-1) * avg_rows_per_worker; // dove il nodo N deve iniziare

            /* 

                Se la dimensione della matrice è divisibile per il numero di nodi.. tutto tranquillo
                Se la dimensione della matrice è non divisibile per il numero di nodi.. l'ultimo nodo deve ricevere anche le righe rimanenti

            */
            if (((i+1) == n_workers) && (N_ROWS % (n_workers-1) != 0)) {
                end_row = start_row + (N_ROWS % (n_workers-1)); // l'ultimo nodo deve ricevere anche le righe rimanenti
            } else {
                end_row = start_row + avg_rows_per_worker; // è possibile suddividerela in parti uguali la matrice
            }

            
            // invio delle informazioni al worker
            MPI_Isend(&start_row, 1, MPI_INT, i, FROM_MASTER_TAG, MPI_COMM_WORLD, &request);
            MPI_Isend(&end_row, 1, MPI_INT, i, FROM_MASTER_TAG, MPI_COMM_WORLD, &request);
            MPI_Isend(&matrix_a[start_row][0], (end_row - start_row) * N_COLS , MPI_DOUBLE, i, FROM_MASTER_TAG, MPI_COMM_WORLD, &request);

        }
    }

    // Usiamo il Sistema Broadcast per inviare a tutti i workers la matrice B Completa
    MPI_Bcast(&matrix_b[0][0], N_ROWS * N_COLS, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    
    // il worker ha il rank diverso da 0 (Maggiore di 0)
    if (rank != 0) {
        // ricezione delle informazioni dal master
        MPI_Recv(&start_row, 1, MPI_INT, 0, FROM_MASTER_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&end_row, 1, MPI_INT, 0, FROM_MASTER_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&matrix_a[start_row][0], (end_row - start_row) * N_COLS , MPI_DOUBLE, 0, FROM_MASTER_TAG, MPI_COMM_WORLD, &status);

        // Eseguiamo la vera operazione di moltiplicazione tra le matrici 
        for (i=start_row; i < end_row; i++) {
            for (j=0; j < N_COLS; j++) {
                for (k=0; k < N_ROWS; k++) {
                    matrix_final[i][j] += (matrix_a[i][k] * matrix_b[k][j]);
                }
            }
        }

        // Ritorniamo il risultato al master [Lo start_row e l'end_row servono per che dimensione ha il vettore risultante (evitiamo di generare una nuova variabile)]
        MPI_Isend(&start_row, 1, MPI_INT, 0, FROM_WORKER_TAG, MPI_COMM_WORLD, &request);
        MPI_Isend(&end_row, 1, MPI_INT, 0, FROM_WORKER_TAG, MPI_COMM_WORLD, &request);
        MPI_Isend(&matrix_final[start_row][0], (end_row - start_row) * N_COLS , MPI_DOUBLE, 0, FROM_WORKER_TAG, MPI_COMM_WORLD, &request); 
    }
    
    // Ora il Master riceve i risultati dai worker e li deve elaborare
    if (rank == 0) {

        for (i= 1; i < n_workers; i++) {

            MPI_Recv(&start_row, 1, MPI_INT, i, FROM_WORKER_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&end_row, 1, MPI_INT, i, FROM_WORKER_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&matrix_final[start_row][0], (end_row - start_row) * N_COLS , MPI_DOUBLE, i, FROM_WORKER_TAG, MPI_COMM_WORLD, &status);

        }

        // Definiamo il Completamento del tempo di esecuzione
        end_time = MPI_Wtime();
        printf("Tempo di esecuzione: %f\n", end_time - start_time);

        // Stampiamo il risultato

        
        // chiediamo all'utente se vuole stampare la matrice risultante (1 = si, 0 = no)
        printf("Vuoi stampare la matrice risultante? (1 = si, 0 = no)\n");
        int print_matrix;
        scanf("%d", &print_matrix);

        if (print_matrix == 1) {
        printf("\n\n\n");
        for (i = 0; i < N_ROWS; i++) {
            printf("\n");
            for (j = 0; j < N_COLS; j++)
                printf("%8.2f  ", matrix_final[i][j]);
            } printf("\n\n");
            
        }
    
    }

    // Concludiamo l'esecuzione del programma
    MPI_Finalize();
    return 0;
}