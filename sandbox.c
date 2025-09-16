#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

void fun ( void ) {
	printf("wow\n");
	while (1);
}
int sandbox (void (*f)(void), unsigned int time, bool verbose);

int main ( int ac, char** av ) {
	int exit_status = 0;

	exit_status = sandbox(fun, (unsigned int)1, true);
	printf("exit status %d \n", exit_status);

	return exit_status;
};

pid_t exec_fun ( void (*f)(void), unsigned int time ) {
	
	pid_t id = fork();
	if ( id == -1 ) return id;

	if ( id == 0 ) {
		
		alarm(time);
		f();
		exit(0); // we always suppose it a good function

	}

	return id;
}

void set_signal ( void ) {
	struct  sigaction sa;
	
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &sa, NULL);
}

int monitor ( pid_t id ) {
	int status;
	pid_t tag = waitpid( id, &status, 0);
	
	if ( tag == -1 && errno == EINTR ) {
		kill ( id, SIGKILL );
		waitpid ( id, 0, 0 );
		return -1;
	}

	if ( WIFEXITED(status) ) {
		if ( WEXITSTATUS(status) == 0 ) 			
			return 1;
		else
			return 0;
	}

	if ( WIFSIGNALED(status) ) {
		if ( WTERMSIG(status) == SIGALRM )
			return 0;
		else
			return -1;
	}

	return -1;
}

int sandbox (void (*f)(void), unsigned int time, bool verbose) {
	
	// setup child with alarm
	pid_t id = exec_fun ( f, time );

	// setup signal handling
	set_signal () ;

	//monitor
	alarm ( time );
	int status = monitor( id );

	return status;
}
