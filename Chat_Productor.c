/*=============================================================
Nombre --------- Chat_Productor.c
Autor ---------- Juan David Arias & Daniel Rodriguez
Materia -------- Sistemas Operativos
Semestre ------- 2017-2
Compilación ---- gcc -Wall -o Chat_Productor.out Chat_Productor.c -lpthread
Ejecución ------ ./Chat_Productor.out
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

	/*Se declaran las variables*/
	pid_t hijo;
	int shmid;
    key_t key = 666;
    Mensaje mensaje;
	Mensaje *shm; 
 
	//Se crea el segmento de memoria compartida
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

		sem_unlink("/sem_cont");
		sem_unlink("/sem_free");

		exit(0);
	} else if (hijo == 0) {

		/*estamos en el padre -> productor */
		printf("Soy el padre (productor) con PID:%d\n", getpid());
		sleep(1);

		/*El productor produce mensajes*/
		for(int i = 0 ; i < BUFFER_SIZE; i++){
			sem_wait(sem_free);
			printf("Ingrese el mensaje %d: ", i+1);
			fgets(shm[i].mensaje, 20, stdin);	
			sem_post(sem_cont);		
		}

		/*Espero que el consumidor termine de consumir*/
		while(*shm->mensaje != '*'){
			sleep(1);
		}

		/*Se liberan los espacios de memoria*/
		if(shmctl(shmid, IPC_RMID, 0) == -1){
			fprintf(stderr, "shmctl(IPC_RMID) error\n");
		} else {
			printf("Memoria liberada!!!");
		}

		/*Se liberan los semáforos */
		sem_close(sem_cont);
		sem_close(sem_free);

		exit(0);
	}

	printf("Soy el padre espero que termine el hijo .....\n");

	/*Se espera a que el hijo termine*/
	wait(0); 

	printf("Soy el padre destruyo los semaforos y termino.....\n");

	sem_unlink("/sem_cont");
	sem_unlink("/sem_free");

	exit(0);
}
