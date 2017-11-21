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

sem_t *sem_cont, *sem_free; // declaramos un puntero para el identificador de los semaforos

int main (int argc, char *argv[]) {
	pid_t hijo;
	int shmid;
	key_t key = 666;
	Mensaje mensaje;
	Mensaje *shm; 

	/* Ubica el segmento */

	if ((shmid = shmget(key, sizeof(mensaje)*BUFFER_SIZE, 0666)) < 0) {
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

		sem_close(sem_cont);
		sem_close(sem_free);
	
		exit(0);
	} else if (hijo == 0) {
		/*estamos en el hijo -> consumidor */
		printf("Soy el hijo (consumidor) con PID:%d\n", getpid());	
		sleep(1);
		for(int i = 0 ; i < BUFFER_SIZE; i++){
			sem_wait(sem_free);
			printf("Mensaje consumido: %s\n", shm[i].mensaje);	
			sem_post(sem_free);		
		}
		*shm->mensaje = '*';
		exit(0);
	}

	wait(0);

	/* libero semáforos */
	sem_close(sem_cont);
	sem_close(sem_free);

	printf("Soy el hijo y termino.....\n");

	exit(0);
}
