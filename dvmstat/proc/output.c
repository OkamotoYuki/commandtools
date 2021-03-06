// using for logpool

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "output.h"

ltrace_t *ltrace;

static ltrace_t *ltrace_open_string_data32(ltrace_t *parent)
{
	struct logpool_param_string param = {32, 1024};
	extern logapi_t STRING_API;
	return ltrace_open(parent, &STRING_API, (logpool_param_t *) &param);
}

static ltrace_t *ltrace_open_syslog_data32(ltrace_t *parent)
{
	struct logpool_param_syslog param = {32, 1024};
	extern logapi_t SYSLOG_API;
	return ltrace_open(parent, &SYSLOG_API, (logpool_param_t *) &param);
}

static ltrace_t *ltrace_open_file_data32(ltrace_t *parent, char *filename)
{
	struct logpool_param_file param = {32, 1024};
	param.fname = (const char *) filename;
	extern logapi_t FILE2_API;
	return ltrace_open(parent, &FILE2_API,  (struct logpool_param*) &param);
}

static ltrace_t *ltrace_open_memcache_data32(ltrace_t *parent, char *host, long ip)
{
	struct logpool_param_memcache param = {32, 1024};
	param.host = host;
	param.port = ip;
	extern logapi_t MEMCACHE_API;
	return ltrace_open(parent, &MEMCACHE_API, (struct logpool_param*) &param);
}

char prefix[MAX_PREFIX_LEN];

#define MAX_PID_LEN 5
#define MAX_HOSTNAME_LEN 26   // MAX_PREFIX_LEN - MAX_PID_LEN - 1

static void make_prefix(void) {
	char *env;

	if((env = getenv("LOGPOOL_PREFIX")) != NULL) {
		memcpy(prefix, "dvmstat+", 8);
		memcpy(prefix + 8, env, strlen(env));
		return;
	}

	char hostname[MAX_HOSTNAME_LEN];
	int pid = (int)getpid();
	gethostname(hostname, MAX_HOSTNAME_LEN);
	snprintf(prefix, MAX_PREFIX_LEN, "dvmstat+%s+%d", hostname, pid);
	return;
}

static char *get_server_ip(void) {
	char *ip;
	ip = getenv("LOGPOOL_SERVER_IP");
	return ip;
}


#define OPT_LOG_STDERR_LEN 11
#define OPT_LOG_SYSLOG_LEN 11
#define OPT_LOG_FILE_LEN 9
#define OPT_LOG_MEMCACHED_LEN 14
#define OPT_PREFIX_LEN 8

int configure_output(char *arg) {
	static int prefix_flag = 1;

	logpool_init(LOGPOOL_DEFAULT);
	if(arg == NULL) {
		if(prefix_flag) make_prefix();
		if(ltrace != NULL) return 0;
		prefix_flag = 0;
		return 1;
	}
	else {
		char opt_log_syslog[] = "-log=syslog";
		char opt_log_file[] = "-log=file";
		char opt_log_memcached[] = "-log=memcached";
		char opt_prefix[] = "-prefix=";

		if(strncmp(arg, opt_log_syslog, OPT_LOG_SYSLOG_LEN) == 0) {
			if(ltrace == NULL) ltrace = ltrace_open_syslog_data32(NULL);
		}
		else if(strncmp(arg, opt_log_file, OPT_LOG_FILE_LEN) == 0) {
			if(ltrace == NULL) ltrace = ltrace_open_file_data32(NULL, "LOG");
		}
		else if(strncmp(arg, opt_log_memcached, OPT_LOG_MEMCACHED_LEN) == 0) {
			if(ltrace == NULL) {
				char *ip = get_server_ip();
				char *host;
				int port;
				if(ip != NULL) {
					host = strtok(ip, ":");
					port = atoi(strtok(NULL, ":"));
				}
				else {
					host = "localhost";
					port = 11211;
				}
				ltrace = ltrace_open_memcache_data32(NULL, host, port);
			}
		}
		else if(strncmp(arg, opt_prefix, OPT_PREFIX_LEN) == 0) {
			arg += 8;
			if(prefix_flag) {
				memcpy(prefix, "dvmstat+", 8);
				memcpy(prefix + 8, arg, strlen(arg));
			}
			prefix_flag = 0;
		}
		else {
			return 1;
		}
	}
	return 0;
}
