# Matrix Parallel Multiply


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
