/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <syslog.h>
#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "utils.h"
#include "mjpg_streamer.h"

//631.6.0730, 638.4.0915 hiroshi: include OpenCV header
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
//640.2.0927 hiroshi: include image_proc.h
#include "_imageproc/image_proc.h"
//704.4.0126 hiroshi: include IhsorihGoogLeNetObjectRecognizer.h
#include "_GoogleNet/IhsorihGoogLeNetObjectRecognizer.h"

//652.4.1222
#include "_i2c/i2cPiToArduino.h"

// 638.4.0915 hiroshi. for opencv
using namespace cv;

// 642.1.1010 hiroshi: init pthread_mutex_t, pthread_cond_t
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 652.4.1222 hiroshi: i2cPiToArduino
I2C i2c;




void error(const char *msg)
{
    perror(msg);
    exit(1);
}
// cv_thread function
static void *cv_proc(void *arg)
{
    return NULL;
}
// tcpSrv_thread function
static void *tcp_server(void *arg)
{
    int srv_socket , client_socket , c;
    struct sockaddr_in server , client;
//    char *message;
    char buffer[256];
    int n,m;

    //Create socket
    srv_socket = socket(AF_INET , SOCK_STREAM , 0);
    if (srv_socket == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 6969 );

    //Bind
    if( bind(srv_socket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
        return NULL;
    }
    puts("bind done");

    //Listen
    listen(srv_socket , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    client_socket = accept(srv_socket, (struct sockaddr *)&client, (socklen_t*)&c); // <-- "accept" will block to wait

    if (client_socket<0)
    {
        perror("accept failed");
        return NULL;
    }
    puts("Connection accepted");

    // 642.2.1011 hiroshi: receive data from client <-- block here untill read something from client
    while (n = read(client_socket, buffer, sizeof(buffer)) > 0)
    {
	printf("Here is the message: %s\n",buffer);
	printf("message lenght: %d\n", strlen(buffer));
	printf("message size: %d\n", sizeof(buffer));


	//652.4.1222 hiroshi: send reveived cmd to Ellum(arduino) via i2c
	if (strcmp(buffer, "up") == 0)
	    i2c.send("1");
	else if (strcmp(buffer, "down") == 0)
	    i2c.send("2");
	else if (strcmp(buffer, "left") == 0)
	    i2c.send("3");
	else if (strcmp(buffer, "right") == 0)
	    i2c.send("4");
	else if (strcmp(buffer, "UL") == 0)
	    i2c.send("5");
	else if (strcmp(buffer, "UR") == 0)
	    i2c.send("5");
	else if (strcmp(buffer, "DL") == 0)
	    i2c.send("5");
	else if (strcmp(buffer, "DR") == 0)
	    i2c.send("5");
	else
	    i2c.send("0");

	memset(buffer, 0x0, sizeof(buffer));

        //Reply to the client
//        message = "received msg: ";
//        m = write(client_socket , message , strlen(message));
//			if (m < 0) error("ERROR writing header to socket");

        m = write(client_socket , buffer , strlen(buffer));
			if (m < 0) error("ERROR writing content to socket");
    }

    if (n == 0)
    {
	puts("client disconnected");
	fflush(stdout);
    }
    if (n < 0)
	error("ERROR reading from socket");

    close(client_socket);
    close(srv_socket);

    return NULL;
}
// talk_thread function
static void *talk_proc(void *arg)
{
    return NULL;
}


/* globals */
static globals global;

/******************************************************************************
Description.: Display a help message
Input Value.: argv[0] is the program name and the parameter progname
Return Value: -
******************************************************************************/
static void help(char *progname)
{
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Usage: %s\n" \
            "  -i | --input \"<input-plugin.so> [parameters]\"\n" \
            "  -o | --output \"<output-plugin.so> [parameters]\"\n" \
            " [-h | --help ]........: display this help\n" \
            " [-v | --version ].....: display version information\n" \
            " [-b | --background]...: fork to the background, daemon mode\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #1:\n" \
            " To open an UVC webcam \"/dev/video1\" and stream it via HTTP:\n" \
            "  %s -i \"input_uvc.so -d /dev/video1\" -o \"output_http.so\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #2:\n" \
            " To open an UVC webcam and stream via HTTP port 8090:\n" \
            "  %s -i \"input_uvc.so\" -o \"output_http.so -p 8090\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #3:\n" \
            " To get help for a certain input plugin:\n" \
            "  %s -i \"input_uvc.so --help\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "In case the modules (=plugins) can not be found:\n" \
            " * Set the default search path for the modules with:\n" \
            "   export LD_LIBRARY_PATH=/path/to/plugins,\n" \
            " * or put the plugins into the \"/lib/\" or \"/usr/lib\" folder,\n" \
            " * or instead of just providing the plugin file name, use a complete\n" \
            "   path and filename:\n" \
            "   %s -i \"/path/to/modules/input_uvc.so\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
}

/******************************************************************************
Description.: pressing CTRL+C sends signals to this process instead of just
              killing it plugins can tidily shutdown and free allocated
              resources. The function prototype is defined by the system,
              because it is a callback function.
Input Value.: sig tells us which signal was received
Return Value: -
******************************************************************************/
static void signal_handler(int sig)
{
#ifdef _SHOW_DBG
    DBG("\n\n in signal_handler!\n\n");
#endif
    int i;

    // 652.4.1222 hiroshi: stop motor
    i2c.send("0");

    /* signal "stop" to threads */
    LOG("setting signal to stop\n");
    global.stop = 1;
    usleep(1000 * 1000);

    /* clean up threads */
    LOG("force cancellation of threads and cleanup resources\n");
    for(i = 0; i < global.incnt; i++) {
        global.in[i].stop(i);
        /*for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            if (global.in[i].param.argv[j] != NULL) {
                free(global.in[i].param.argv[j]);
            }
        }*/
    }

    for(i = 0; i < global.outcnt; i++) {
        global.out[i].stop(global.out[i].param.id);
        pthread_cond_destroy(&global.in[i].db_update);
        pthread_mutex_destroy(&global.in[i].db);
        /*for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            if (global.out[i].param.argv[j] != NULL)
                free(global.out[i].param.argv[j]);
        }*/
    }
    usleep(1000 * 1000);

    /* close handles of input plugins */
    for(i = 0; i < global.incnt; i++) {
        dlclose(global.in[i].handle);
    }

    for(i = 0; i < global.outcnt; i++) {
        int j, skip = 0;
        DBG("about to decrement usage counter for handle of %s, id #%02d, handle: %p\n", \
            global.out[i].plugin, global.out[i].param.id, global.out[i].handle);

        for(j=i+1; j<global.outcnt; j++) {
          if ( global.out[i].handle == global.out[j].handle ) {
            DBG("handles are pointing to the same destination (%p == %p)\n", global.out[i].handle, global.out[j].handle);
            skip = 1;
          }
        }
        if ( skip ) {
          continue;
        }

        DBG("closing handle %p\n", global.out[i].handle);

        dlclose(global.out[i].handle);
    }
    DBG("all plugin handles closed\n");

    LOG("done\n");

    closelog();
    exit(0);
    return;
}

static int split_parameters(char *parameter_string, int *argc, char **argv)
{
    int count = 1;
    argv[0] = NULL; // the plugin may set it to 'INPUT_PLUGIN_NAME'
    if(parameter_string != NULL && strlen(parameter_string) != 0) {
        char *arg = NULL, *saveptr = NULL, *token = NULL;

        arg = strdup(parameter_string);

        if(strchr(arg, ' ') != NULL) {
            token = strtok_r(arg, " ", &saveptr);
            if(token != NULL) {
                argv[count] = strdup(token);
                count++;
                while((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                    argv[count] = strdup(token);
                    count++;
                    if(count >= MAX_PLUGIN_ARGUMENTS) {
                        IPRINT("ERROR: too many arguments to input plugin\n");
                        return 0;
                    }
                }
            }
        }
        free(arg);
    }
    *argc = count;
    return 1;
}

/******************************************************************************
Description.:
Input Value.:
Return Value:
******************************************************************************/
int main(int argc, char *argv[])
{
    ///642.1.1010 hiroshi: init pthread///
    pthread_t cv_thread;
    pthread_t tcpSrv_thread;
    pthread_t talk_thread;

    //pthread create
    // create cv_thread
    if ( pthread_create( &cv_thread, NULL, cv_proc, NULL) ) {
	printf("error creating cv_thread.");
	abort();
    }
    // create tcpSrv_thread
    if ( pthread_create( &tcpSrv_thread, NULL, tcp_server, NULL) ) {
	printf("error creating tcpSrv_thread.");
	abort();
    }
    // create talk_thread
    if ( pthread_create( &talk_thread, NULL, talk_proc, NULL) ) {
	printf("error creating talk_thread.");
	abort();
    }


    //char *input  = "input_uvc.so --resolution 640x480 --fps 5 --device /dev/video0";
    char *input[MAX_INPUT_PLUGINS];
    char *output[MAX_OUTPUT_PLUGINS];
    int daemon = 0, i, j;
    size_t tmp = 0;

    output[0] = "output_http.so --port 8080";
    global.outcnt = 0;
    global.incnt = 0;

    // 640.2.0927 hiroshi: init image: face detection
    if (!initFaceDetection()) {
#ifdef _SHOW_DBG
        DBG("\n\n\n\ cannot init face detection! n\n\n");
#endif
	return 1;
    }

    // 704.4.0126 hiroshi: init GoogleNet Obj-rec object
    IhsorihGoogLeNetObjectRecognizer objRec;
    if (!objRec.init("_GoogleNet/bvlc_googlenet.prototxt",
                             "_GoogleNet/bvlc_googlenet.caffemodel",
                             "_GoogleNet/synset_words.txt")) {
#ifdef _SHOW_DBG
        DBG("\n\n\n\ cannot init GoogleNet Obj-rec! n\n\n");
#endif
        return 1;
    }

    /* parameter parsing */
    while(1) {
        int c = 0;
        static struct option long_options[] = {
            {"help", no_argument, NULL, 'h'},
            {"input", required_argument, NULL, 'i'},
            {"output", required_argument, NULL, 'o'},
            {"version", no_argument, NULL, 'v'},
            {"background", no_argument, NULL, 'b'},
            {NULL, 0, NULL, 0}
        };

        c = getopt_long(argc, argv, "hi:o:vb", long_options, NULL);

        /* no more options to parse */
        if(c == -1) break;

        switch(c) {
        case 'i':
            input[global.incnt++] = strdup(optarg);
            break;

        case 'o':
            output[global.outcnt++] = strdup(optarg);
            break;

        case 'v':
            printf("MJPG Streamer Version: %s\n" \
            "Compilation Date.....: %s\n" \
            "Compilation Time.....: %s\n",
#ifdef SVN_REV
            SVN_REV,
#else
            SOURCE_VERSION,
#endif
            __DATE__, __TIME__);
            return 0;
            break;

        case 'b':
            daemon = 1;
            break;

        case 'h': /* fall through */
        default:
            help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    openlog("MJPG-streamer ", LOG_PID | LOG_CONS, LOG_USER);
    //openlog("MJPG-streamer ", LOG_PID|LOG_CONS|LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "starting application");

    /* fork to the background */
    if(daemon) {
        LOG("enabling daemon mode");
        daemon_mode();
    }

    /* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets) */
    signal(SIGPIPE, SIG_IGN);

    /* register signal handler for <CTRL>+C in order to clean up */
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
#ifdef _SHOW_DBG
	DBG("\n\ncould not register signal handler\n\n");
#endif
        LOG("could not register signal handler\n");
        closelog();
        exit(EXIT_FAILURE);
    }
#ifdef _SHOW_DBG
    else
	DBG("\n\nregeister signal OK\n\n");
#endif

    /*
     * messages like the following will only be visible on your terminal
     * if not running in daemon mode
     */
#ifdef SVN_REV
    LOG("MJPG Streamer Version: svn rev: %s\n", SVN_REV);
#else
    LOG("MJPG Streamer Version.: %s\n", SOURCE_VERSION);
#endif

    /* check if at least one output plugin was selected */
    if(global.outcnt == 0) {
        /* no? Then use the default plugin instead */
        global.outcnt = 1;
    }

    // 638.4.0915 hiroshi
    // input
    typedef int (*myInit)(input_parameter*, int);
    typedef int (*myStop)(int);
    typedef int (*myRun)(int);
    typedef int (*myCmd)(int, unsigned int, unsigned int, int, char*);
    // output
    typedef int (*myOutputInit)(output_parameter*, int);

    /* open input plugin */
    for(i = 0; i < global.incnt; i++) {
        /* this mutex and the conditional variable are used to synchronize access to the global picture buffer */
        if(pthread_mutex_init(&global.in[i].db, NULL) != 0) {
            LOG("could not initialize mutex variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }
        if(pthread_cond_init(&global.in[i].db_update, NULL) != 0) {
            LOG("could not initialize condition variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }

        tmp = (size_t)(strchr(input[i], ' ') - input[i]);
        global.in[i].stop      = 0;
        global.in[i].context   = NULL;
        global.in[i].buf       = NULL;
        global.in[i].size      = 0;
        global.in[i].plugin = (tmp > 0) ? strndup(input[i], tmp) : strdup(input[i]);
        global.in[i].handle = dlopen(global.in[i].plugin, RTLD_LAZY);
        if(!global.in[i].handle) {
            LOG("ERROR: could not find input plugin\n");
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }

	global.in[i].init = (myInit)dlsym(global.in[i].handle, "input_init");
        if(global.in[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].stop = (myStop)dlsym(global.in[i].handle, "input_stop");
        if(global.in[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].run = (myRun)dlsym(global.in[i].handle, "input_run");
        if(global.in[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        /* try to find optional command */
        global.in[i].cmd = (myCmd)dlsym(global.in[i].handle, "input_cmd");

        global.in[i].param.parameters = strchr(input[i], ' ');

        for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            global.in[i].param.argv[j] = NULL;
        }

        split_parameters(global.in[i].param.parameters, &global.in[i].param.argc, global.in[i].param.argv);
        global.in[i].param.global = &global;
        global.in[i].param.id = i;

        if(global.in[i].init(&global.in[i].param, i)) {
            LOG("input_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
    }

    /* open output plugin */
    for(i = 0; i < global.outcnt; i++) {
        tmp = (size_t)(strchr(output[i], ' ') - output[i]);
        global.out[i].plugin = (tmp > 0) ? strndup(output[i], tmp) : strdup(output[i]);
        global.out[i].handle = dlopen(global.out[i].plugin, RTLD_LAZY);
        if(!global.out[i].handle) {
            LOG("ERROR: could not find output plugin %s\n", global.out[i].plugin);
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }
        global.out[i].init = (myOutputInit)dlsym(global.out[i].handle, "output_init");
        if(global.out[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].stop = (myStop)dlsym(global.out[i].handle, "output_stop");
        if(global.out[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].run = (myRun)dlsym(global.out[i].handle, "output_run");
        if(global.out[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        /* try to find optional command */
        global.out[i].cmd = (myCmd)dlsym(global.out[i].handle, "output_cmd");

        global.out[i].param.parameters = strchr(output[i], ' ');

        for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            global.out[i].param.argv[j] = NULL;
        }
        split_parameters(global.out[i].param.parameters, &global.out[i].param.argc, global.out[i].param.argv);

        global.out[i].param.global = &global;
        global.out[i].param.id = i;
        if(global.out[i].init(&global.out[i].param, i)) {
            LOG("output_init() return value signals to exit\n");
            closelog();
            exit(EXIT_FAILURE);
        }
    }

    /* start to read the input, push pictures into global buffer */
    DBG("starting %d input plugin\n", global.incnt);
    for(i = 0; i < global.incnt; i++) {
        syslog(LOG_INFO, "starting input plugin %s", global.in[i].plugin);
        if(global.in[i].run(i)) {
            LOG("can not run input plugin %d: %s\n", i, global.in[i].plugin);
            closelog();
            return 1;
        }
    }

    DBG("starting %d output plugin(s)\n", global.outcnt);
    for(i = 0; i < global.outcnt; i++) {
        syslog(LOG_INFO, "starting output plugin: %s (ID: %02d)", global.out[i].plugin, global.out[i].param.id);
        global.out[i].run(global.out[i].param.id);
    }





    // 636.6.0903 hiroshi: get frames
    Mat image(240, 320, CV_8UC3);

    char key;

    //set camera params
//    Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );

    while (!global.stop)
    {
        memcpy (image.ptr<uchar> ( 0 ), global.buf, global.size);
        cvtColor(image, image, COLOR_RGB2BGR);
        flip(image, image, 0);

        // 704.4.0126 hiroshi: GoogleNet Obj-rec
        objRec.setImage(image);
//        objRec.predict();
//        objRec.putProbBar(image);
        imshow("GoogleNet Obj-rec", image);

        // << opencv face detection
//        detectFace(image);
//        imshow("face-detection", image);
        // opencv face-detection >>

        cvtColor(image, image, COLOR_BGR2RGB);

        // copy processed frame image into buffer to encoder input port (http send to client)
        memcpy (global.buf2encoder_input_port, image.ptr<uchar> ( 0 ), global.size);
        key = waitKey(10);
    	if (char(key) == 27)
	    break;
    }


    ///642.1.1010 hiroshi:
    //// pthread join ///
    if ( pthread_join ( cv_thread, NULL ) )
    {
	printf("error joining cv_thread.");
	abort();
    }
    if ( pthread_join ( tcpSrv_thread, NULL ) )
    {
	printf("error joining tcpSrv_thread.");
	abort();
    }
    if ( pthread_join ( talk_thread, NULL ) )
    {
	printf("error joining talk_thread.");
	abort();
    }

    /* wait for signals */
    pause();

    return 0;
}
