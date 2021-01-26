#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pulse/simple.h>

#define BUFSIZE 1024
volatile sig_atomic_t flag_do = 1;

void handler(int s)
{
	signal(SIGINT, handler);
	flag_do = 0;
}

int loop_write(int fd, const void *data, size_t size)
{
    int ret = 0;

    while(size > 0) {
        int r;

        if((r = write(fd, data, size)) < 0)
            return r;

        if(r == 0)
            break;

        ret += r;
        data = (const uint8_t *) data + r;
        size -= (size_t) r;
    }

    return ret;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, handler);
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.channels = 2;
    ss.rate = 44100;

    pa_simple *s;
    s = pa_simple_new(NULL,
                    argv[0],
                    PA_STREAM_RECORD,
                    NULL,
                    "record",
                    &ss,
                    NULL,
                    NULL,
                    NULL);
    if(!s) {
        fprintf(stderr, "[Error] pa_simple_new() failed\n");
        return 1;
    }

	fprintf(stderr, "[Message] Use \"Ctrl + C\" for exit\n");
   	while(flag_do) {
        uint8_t buf[BUFSIZE];

        // Record some data
        if(pa_simple_read(s, buf, sizeof(buf), NULL) < 0) {
            fprintf(stderr, "[Error] pa_simple_read() failed\n");
            return 1;
        }

        // And write it to STDOUT */
        if(loop_write(STDOUT_FILENO, buf, sizeof(buf)) != sizeof(buf)) {
            fprintf(stderr, "[Error] write() failed\n");
            return 1;
        }
    }

    if(s)
    	pa_simple_free(s);

    return 0;
}