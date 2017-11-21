/*=============================================================
Nombre --------- Chat_Consumidor.c
Autor ---------- Juan David Arias & Daniel Rodriguez
Materia -------- Sistemas Operativos
Semestre ------- 2017-2
Compilación ---- gcc -Wall -o Chat_Consumidor.out Chat_Consumidor.c -lpthread
Ejecución ------ ./Chat_Consumidor.out
=============================================================*/

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

/*Se define BUFFER_SIZE como constante*/
#define BUFFER_SIZE 10  

/*Se declara una estructura llamada Mensajes*/
typedef struct{
	char mensaje[20];
} Mensaje;

/*Se declara un puntero para el identificador de los semaforos*/
sem_t *sem_cont, *sem_free;

int main (int argc, char *argv[]) {

	/*Se definen las variables*/
	pid_t hijo;
	int shmid;
	key_t key = 666;
	Mensaje mensaje;
	Mensaje *shm; 

	/*Se ubica el segmento de memoria compartida*/
	if ((shmid = shmget(key, sizeof(mensaje)*BUFFER_SIZE, 0666)) < 0) {
		perror("shmget");
		exit(1);
	}

	/*Se adhiere el consumidor a la memoria compartida*/
    if ((shm = shmat(shmid, NULL, 0)) == (Mensaje *) -1) {
	    perror("shmat");
	    exit(1);
    }

	printf("Creando semaforos .....\n");

	/*Se comprueba si ya existe el semaforo del contador de productos sino lo crea inicializado(0)*/
	if((sem_cont = sem_open("/sem_cont", O_CREAT, 0666, 0)) == (sem_t *)-1) {
		perror("Error creando semaforo 1");
	}

	/*Se comprueba si ya existe el semaforo del espacio libre y sino lo crea inicializado (BUFFER_SIZE)*/
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

		/*El consumidor consume los mensajes*/
		for(int i = 0 ; i < BUFFER_SIZE; i++){
			sem_wait(sem_cont);
			printf("Mensaje consumido: %s", shm[i].mensaje);	
			sem_post(sem_free);		
		}

		/*El consumidor indica que ha terminado de consumir*/
		*shm->mensaje = '*';
		exit(0);
	}

	wait(0);

	/*Se liberan los semáforos */
	sem_close(sem_cont);
	sem_close(sem_free);

	printf("Soy el hijo y termino.....\n");

	exit(0);
}
