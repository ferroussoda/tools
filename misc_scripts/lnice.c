/*
 * lnice.c: Run tasks with userset latency_nice value
 *
 * Synopsis:
 * lnice -l 19 ./workload-cmd-here
 *
 * latency_nice valus can be in range [-20, 19]
 * To decrease the currnet latency_nice value to the a
 * lower value requires root priviledges.
 *
 * @author: Parth Shah <parths1229@gmail.com>
 */
#define _GNU_SOURCE
#include <unistd.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define SCHED_FLAG_RECLAIM		0x02
#define SCHED_FLAG_DL_OVERRUN		0x04
#define SCHED_FLAG_KEEP_POLICY		0x08
#define SCHED_FLAG_KEEP_PARAMS		0x10
#define SCHED_FLAG_UTIL_CLAMP_MIN	0x20
#define SCHED_FLAG_UTIL_CLAMP_MAX	0x40
#define SCHED_FLAG_LATENCY_NICE		0X80

struct sched_attr {
	__u32 size;
	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;
	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;
	/* SCHED_DEADLINE */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
	__u32 sched_util_min;
	__u32 sched_util_max;
	__s32 sched_latency_nice;
};

struct sched_attr sattr;
__s32 latency_nice = 0;
int pid = 0;
int sched_getattr = 0;
int sched_setattr = 0;
char ** cmd;

enum {
	HELP_LONG_OPT = 1,
};
char *option_string = "s:l:h";
static struct option long_options[] = {
	{"set", required_argument, 0, 's'},
	{"latencynice", required_argument, 0, 'l'},
	{"help", no_argument, 0, HELP_LONG_OPT},
	{0, 0, 0, 0}
};

static void print_usage(void)
{
	fprintf(stderr, "lnice [OPTION] COMMAND:\n"
			"\t -s(--set) VAL: set latency_nice value to VAL\n"
			"\t -l(--latencynice) VAL: same as --set\n"
	       );
	exit(1);
}

static void parse_options(int ac, char **av)
{
	int c;

	do {
		int option_index = 0;

		c = getopt_long(ac, av, option_string,
				long_options, &option_index);

		if (c == -1)
			break;

		switch(c) {
			case 's':
			case 'l':
				sscanf(optarg, "%d", &latency_nice);
				sattr.sched_latency_nice = latency_nice;
				sattr.sched_flags |= SCHED_FLAG_LATENCY_NICE;
				pid = syscall(SYS_gettid);
				if (syscall(SYS_sched_setattr, pid, &sattr, 0)) {
					printf("Failed to do set latency_nice for pid=%d\n", pid);
					exit(1);
				}
				break;
			case 'h':
			case '?':
			case HELP_LONG_OPT:
				print_usage();
				break;
			case '-':
			default:
				break;
		}
	} while (0);

	if (optind < ac) {
			cmd = &av[optind];
			execvp(cmd[0], &cmd[0]);
	}
}


int main(int argc, char** argv)
{
	int ret;

	sattr.size = sizeof(struct sched_attr);
	sattr.sched_flags = 0;

	parse_options(argc, argv);
	return 0;
}

