#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sched.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 10    // tamaño del buffer

typedef struct{
	char mensaje[20];
} Mensaje;

//int buffer[BUFFER_SIZE];
sem_t *sem_cont, *sem_free; // declaramos un puntero para el identificador de los semaforos

int main (int argc, char *argv[]) {
	pid_t hijo;
	int shmid;
    key_t key = 666;
    Mensaje mensaje;
	Mensaje *shm; 
 
	//Crear segmento de memoria compartida
	if ((shmid = shmget(key, sizeof(mensaje)*BUFFER_SIZE, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	//Se pega de la memoria compartida
    if ((shm = shmat(shmid, NULL, 0)) == (Mensaje *) -1) {
	    perror("shmat");
	    exit(1);
    }

	printf("Creando semaforos .....\n");

	/* comprueba si ya existe el semaforo del contador de productos sino lo crea inicializado(0)*/
	if((sem_cont = sem_open("/sem_cont", O_CREAT, 0666, 0)) == (sem_t *)-1) {
		perror("Error creando semaforo 1");
	}

	/* comprueba si ya existe el semaforo del espacio libre y sino lo crea inicializado (BUFFER_SIZE)*/
	if((sem_free = sem_open("/sem_free", O_CREAT, 0666, BUFFER_SIZE)) == (sem_t *)-1) {
		perror("Error creando semaforo 2");
	}

	printf("Creando proceso hijo .....\n");

	hijo = fork();

	if (hijo == -1) {
		printf("error creando proceso hijo\n");

		sem_unlink("/sem_cont");
		sem_unlink("/sem_free");

		exit(0);
	} else if (hijo == 0) {
		/*estamos en el padre -> productor */
		printf("Soy el padre (productor) con PID:%d\n", getpid());
		sleep(1);

		for(int i = 0 ; i < BUFFER_SIZE; i++){
			printf("Ingrese el mensaje %d: ", i+1);
			fgets(shm[i].mensaje, 20, stdin);			
		}

		while(*shm->mensaje != '*'){
			sleep(1);
		}

		if(shmctl(shmid, IPC_RMID, 0) == -1){
			fprintf(stderr, "shmctl(IPC_RMID) error\n");
		} else {
			printf("Memoria liberada!!!");
		}

		/* libero semáforos */
		sem_close(sem_cont);
		sem_close(sem_free);

		exit(0);
	}

	printf("Soy el padre espero que termine el hijo .....\n");

	wait(0); /* Esperar que acabe el hijo */

	printf("Soy el padre destruyo los semaforos y termino.....\n");

	sem_unlink("/sem_cont");
	sem_unlink("/sem_free");

	exit(0);
}
