	/* 
 * This is a demonstration of periodic threads using POSIX timers and signals.
 * Each periodic thread is allocated a signal between SIGRTMIN to SIGRTMAX: we
 * assume that there are no other uses for these signals.
 * * All RT signals must be blocked in all threads before calling make_periodic()
 */

#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

//signal and period of activation
struct periodic_info
{
	int sig;
	sigset_t alarm_sig;
};


//sets the signal to activate every period
//returns 0 on success
static int make_periodic(int unsigned period_us, struct periodic_info *info);

//sets the period in periodic_info structure
static void wait_period(struct periodic_info *info);