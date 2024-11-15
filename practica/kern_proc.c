// 4.2 Shell primitiva.
// Escriba un programa en C que simule el funcionamiento de una shell primitiva.
// ★ ¿Qu´e deber´ıa agregarse para poder encadenar dos comandos por las salidas y las entradas?
// ◆ Programar la shell junto con todas las validaciones necesarias y comentar el c´odigo explicando
// que sucede en cada paso.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char input[256];
    char *args1[10], *args2[10];
    int pipefd[2];
    pid_t pid;

    while (1) {
        // Mostrar el prompt
        printf("simple-shell> ");
        fgets(input, sizeof(input), stdin);

        // Si el usuario ingresa "exit", salir de la shell
        if (strcmp(input, "exit\n") == 0) break;

        // Buscar si hay un pipe "|"
        char *pipe_pos = strchr(input, '|');

        if (pipe_pos != NULL) {
            // Separar en dos comandos
            *pipe_pos = '\0'; // Reemplazar "|" por fin de cadena
            args1[0] = strtok(input, " \n");
            args2[0] = strtok(pipe_pos + 1, " \n");
            args1[1] = NULL;
            args2[1] = NULL;

            // Crear pipe y proceso hijo
            pipe(pipefd);
            if (fork() == 0) {
                // Primer comando: redirigir salida al pipe
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                execlp(args1[0], args1[0], NULL);
                exit(1);
            }
            if (fork() == 0) {
                // Segundo comando: redirigir entrada al pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                execlp(args2[0], args2[0], NULL);
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
            wait(NULL);
        } else {
            // Ejecutar un solo comando
            args1[0] = strtok(input, " \n");
            args1[1] = NULL;

            if (fork() == 0) {
                execlp(args1[0], args1[0], NULL);
                exit(1);
            }
            wait(NULL);
        }
    }

    return 0;
}

// 4.3 Ping Pong.
// Escriba un programa en C que permita jugar a dos procesos al ping pong, la pelota es
// un entero, cada vez que un proceso recibe la pelota debe incrementar en 1 su valor.
// ★ El programa debe cortar por overflow o cambio de signo.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

void ping_pong(int read_fd, int write_fd) {
    int ball;

    while (1) {
        read(read_fd, &ball, sizeof(ball));

        if (ball < 0) {
            break;
        }

        ball += 1;

        write(write_fd, &ball, sizeof(ball));
    }
}

int main() {
    int pipe1[2], pipe2[2]; // Cada proceso tiene que escribir y leer

    pipe(pipe1);
    pipe(pipe2);
    
    pid_t pid_f = fork();

    if (pid_f == 0) {
        // Proceso hijo: Jugador 2
        close(pipe1[1]);
        close(pipe2[0]);
        ping_pong(pipe1[0], pipe2[1]);
    } else {
        // Proceso padre: Jugador 1
        close(pipe1[0]);
        close(pipe2[1]);

        // Pasa la pelota para comenzar
        int ball = 0;
        write(pipe1[1], &ball, sizeof(ball));

        ping_pong(pipe2[0], pipe1[1]);
    }

    return 0;
}

// 4.4 Primes

void filter(int fds_lectura) {
	int x;

	read(fds_lectura, &x, sizeof(x));

	int fds_nuevo[2];

	int p_nuevo = pipe(fds_nuevo);

	pid_t f_nuevo = fork();

	if (f_nuevo == 0) {
		close(fds_lectura);
		close(fds_nuevo[1]);
		filter(fds_nuevo[0]);
		close(fds_nuevo[0]);
		exit(0);
	} else {
		close(fds_nuevo[0]);
		int y;
		while (read(fds_lectura, &y, sizeof(y)) > 0) {
			if (y % x != 0) {
				write(fds_nuevo[1], &y, sizeof(y));
			}
		}
		close(fds_nuevo[1]);
		wait(NULL);
	}
}


int main(int argc, char *argv[]) {
	int fds[2];

	int p = pipe(fds);

	pid_t f = fork();

	if (f == 0) {
		close(fds[1]);
		filter(fds[0]);
		close(fds[0]);
		exit(0);
	} else {
		close(fds[0]);
		for (int i = 2; i <= n; i++) {
			write(fds[1], &i, sizeof(i));
		}
		close(fds[1]);
		wait(NULL);
	}

    return 0;
}