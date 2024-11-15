#include <pthread.h>
#include <stdio.h>

// Crear cinco threads que en simultaneo sumen 7 a una variable compartida hasta llegar a 1000.
// Hipotesis: todas las syscalls son exitosas.

pthread_mutex_t mutex_var;

void* incrementar(void* var){
	int* int_var = var;

	while (*var < 1000){
		pthread_mutex_lock(&mutex_var);
		*int_var += 7;
		pthread_mutex_unlock(&mutex_var);
	}

	return NULL;
}

int main(){
	pthread_t threads[5];
	int variable_compartida = 0;

	pthread_mutex_init(&mutex_var, NULL);

	for (size_t i = 0; i < 5; ++i){
		pthread_create(&threads[i], NULL, incrementar, &variable_compartida);
    }

	for (size_t i = 0; i < 5; ++i){
		pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_var);

    return 0;
}

// Crear cuatro threads que en simultáneo sumen 10 a una variable compartida, 
// pero cada thread solo puede realizar 10 incrementos en total.
// El objetivo es alcanzar un valor de 400 en la variable compartida.
// Hipótesis: todas las syscalls son exitosas.

pthread_mutex_t mutex_var;

void* incrementar(void* var) {
    int* int_var = (int*) var;

    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutex_var);
        *int_var += 10;
        pthread_mutex_unlock(&mutex_var);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[4];
    int variable_compartida = 0;

    pthread_mutex_init(&mutex_var, NULL);

    for (int i = 0; i < 4; ++i) {
        pthread_create(&threads[i], NULL, incrementar, &variable_compartida);
    }

    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_var);

    return 0;
}

// Crear dos threads que actualicen una variable compartida desde el valor 0.
// El primer thread incrementa en 2 cada vez (solo cuando la variable es par), mientras que el segundo incrementa en 3 cada vez (solo cuando la variable es impar).
// El objetivo es llegar al valor 500.
// Hipótesis: todas las syscalls son exitosas.

pthread_mutex_t mutex_var;

void* increment_even(void* var) {
    int* shared_variable = (int*) var;

    while (1) {
        pthread_mutex_lock(&mutex_var);

        if (shared_variable >= 500) {
            pthread_mutex_unlock(&mutex_var);
            break;
        }

        if (shared_variable % 2 == 0) {
            shared_variable += 2;
        }

        pthread_mutex_unlock(&mutex_var);
    }

    return NULL;
}

void* increment_odd(void* var) {
    int* shared_variable = (int*) var;
    
    while (1) {
        pthread_mutex_lock(&mutex_var);

        if (shared_variable >= 500) {
            pthread_mutex_unlock(&mutex_var);
            break;
        }

        if (shared_variable % 2 != 0) {
            shared_variable += 3;
        }

        pthread_mutex_unlock(&mutex_var);
    }

    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int variable_compartida = 0; 

    pthread_mutex_init(&mutex_var, NULL);

    pthread_create(&thread1, NULL, increment_even, &variable_compartida);
    pthread_create(&thread2, NULL, increment_odd, &variable_compartida);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex_var);
    
    return 0;
}