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
#include <syslog.h>
#include <time.h>

#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "../../mjpg_streamer.h"
#include "../../utils.h"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "mmal/mmal.h"
#include "mmal/util/mmal_default_components.h"
#include "mmal/util/mmal_connection.h"

#include "RaspiCamControl.c"

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2
// Stills format information
#define STILLS_FRAME_RATE_NUM 3
#define STILLS_FRAME_RATE_DEN 1
/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

#define INPUT_PLUGIN_NAME "raspicam input plugin"

// Layer that preview window should be displayed on
#define PREVIEW_LAYER      2
// Frames rates of 0 implies variable, but denominator needs to be 1 to prevent div by 0
#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1

/* private functions and variables to this plugin */
static pthread_t   worker;
static globals     *pglobal;
static pthread_mutex_t controls_mutex;
static int plugin_number;

void *worker_thread(void *);
void worker_cleanup(void *);
void help(void);

static int fps = 5;
static int width = 640;
static int height = 480;
static int quality = 85;
static int usestills = 0;
static int wantPreview = 0;
static RASPICAM_CAMERA_PARAMETERS c_params;

///////////////////////////////
//****** jack's defined *****//
// 635.2.0823 hiroshi
#define _camera_callback
//***************************//
///////////////////////////////


/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
  FILE *file_handle; /// File handle to write buffer data to.
  VCOS_SEMAPHORE_T complete_semaphore; /// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
  MMAL_POOL_T *pool; /// pointer to our state in case required in callback
  uint32_t offset;
} PORT_USERDATA;

/*** plugin interface functions ***/

/******************************************************************************
  Description.: parse input parameters
  Input Value.: param contains the command line string and a pointer to globals
  Return Value: 0 if everything is ok
 ******************************************************************************/
int input_init(input_parameter *param, int plugin_no)
{
  int i;
  if (pthread_mutex_init(&controls_mutex, NULL) != 0)
  {
    IPRINT("could not initialize mutex variable\n");
    exit(EXIT_FAILURE);
  }

  param->argv[0] = INPUT_PLUGIN_NAME;
  plugin_number = plugin_no;

  //setup the camera control st
  raspicamcontrol_set_defaults(&c_params);

  /* show all parameters for DBG purposes */
  for (i = 0; i < param->argc; i++)
  {
    DBG("argv[%d]=%s\n", i, param->argv[i]);
  }

  reset_getopt();
  while(1) {
    int option_index = 0, c = 0;
    static struct option long_options[] = {
      {"h", no_argument, 0, 0},
      {"help", no_argument, 0, 0},
      {"x", required_argument, 0, 0},
      {"width", required_argument, 0, 0},
      {"y", required_argument, 0, 0},
      {"height", required_argument, 0, 0},
      {"fps", required_argument, 0, 0},
      {"framerate", required_argument, 0, 0},
      {"sh", required_argument, 0, 0},
      {"co", required_argument, 0, 0},
      {"br", required_argument, 0, 0},
      {"sa", required_argument, 0, 0},
      {"ISO", required_argument, 0, 0},
      {"vs", no_argument, 0, 0},
      {"ev", required_argument, 0, 0},
      {"ex", required_argument, 0, 0},
      {"awb", required_argument, 0, 0},
      {"ifx", required_argument, 0, 0},
      {"cfx", required_argument, 0, 0},
      {"mm", required_argument, 0, 0},
      {"rot", required_argument, 0, 0},
      {"hf", no_argument, 0, 0},
      {"vf", no_argument, 0, 0},
      {"quality", required_argument, 0, 0},
      {"usestills", no_argument, 0, 0},
      {"preview", no_argument, 0, 0},
      {0, 0, 0, 0}
    };

    c = getopt_long_only(param->argc, param->argv, "", long_options, &option_index);

    /* no more options to parse */
    if(c == -1)
      break;

    /* unrecognized option */
    if (c == '?')
    {
      help();
      return 1;
    }

    switch(option_index) {
      /* h, help */
      case 0:
      case 1:
        DBG("case 0,1\n");
        help();
        return 1;
        break;
        /* width */
      case 2:
      case 3:
        DBG("case 2,3\n");
        width = atoi(optarg);
        break;
        /* height */
      case 4:
      case 5:
        DBG("case 4,5\n");
        height = atoi(optarg);
        break;
        /* fps */
      case 6:
      case 7:
        DBG("case 6, 7\n");
        fps = atoi(optarg);
        break;
      case 8:
        //sharpness
        sscanf(optarg, "%d", &c_params.sharpness);
        break;
      case 9:
        //contrast
        sscanf(optarg, "%d", &c_params.contrast);
        break;
      case 10:
        //brightness
        sscanf(optarg, "%d", &c_params.brightness);
        break;
      case 11:
        //saturation
        sscanf(optarg, "%d", &c_params.saturation);
        break;
      case 12:
        //ISO
        sscanf(optarg, "%d", &c_params.ISO);
        break;
      case 13:
        //video stabilisation
        c_params.videoStabilisation = 1;
        break;
      case 14:
        //ev
        sscanf(optarg, "%d", &c_params.exposureCompensation);
        break;
      case 15:
        //exposure
        c_params.exposureMode = exposure_mode_from_string(optarg);
        break;
      case 16:
        //awb mode
        c_params.awbMode = awb_mode_from_string(optarg);
        break;
      case 17:
        //img effect
        c_params.imageEffect = imagefx_mode_from_string(optarg);
        break;
      case 18:
        //color effects
        sscanf(optarg, "%d:%d", &c_params.colourEffects.u, &c_params.colourEffects.u);
        c_params.colourEffects.enable = 1;
        break;
      case 19:
        //metering mode
        c_params.exposureMeterMode = metering_mode_from_string(optarg);
        break;
      case 20:
        //rotation
        sscanf(optarg, "%d", &c_params.rotation);
        break;
      case 21:
        //hflip
        c_params.hflip  = 1;
        break;
      case 22:
        //vflip
        c_params.vflip = 1;
        break;
      case 23:
        //quality
        quality = atoi(optarg);
        break;
      case 24:
        //use stills
        usestills = 1;
        break;
      case 25:
        //display preview
        wantPreview = 1;
        break;
      default:
        DBG("default case\n");
        help();
        return 1;
    }
  }

  pglobal = param->global;

  IPRINT("fps.............: %i\n", fps);
  IPRINT("resolution........: %i x %i\n", width, height);
  IPRINT("camera parameters..............:\n\n");
  raspicamcontrol_dump_parameters(&c_params);

  return 0;
}

/******************************************************************************
  Description.: stops the execution of the worker thread
  Input Value.: -
  Return Value: 0
 ******************************************************************************/
int input_stop(int id)
{
  DBG("will cancel input thread\n");
  pthread_cancel(worker);

  return 0;
}

/**************************************************
  Print which status
 **************************************************/
void print_mmal_status(MMAL_STATUS_T status)
{
  if (status != MMAL_SUCCESS)
  {
    switch (status)
    {
      case MMAL_ENOMEM : DBG("Out of memory\n"); break;
      case MMAL_ENOSPC : DBG("Out of resources (other than memory)\n"); break;
      case MMAL_EINVAL: DBG("Argument is invalid\n"); break;
      case MMAL_ENOSYS : DBG("Function not implemented\n"); break;
      case MMAL_ENOENT : DBG("No such file or directory\n"); break;
      case MMAL_ENXIO : DBG("No such device or address\n"); break;
      case MMAL_EIO : DBG("I/O error\n"); break;
      case MMAL_ESPIPE : DBG("Illegal seek\n"); break;
      case MMAL_ECORRUPT : DBG("Data is corrupt \attention FIXME: not POSIX\n"); break;
      case MMAL_ENOTREADY :DBG("Component is not ready \attention FIXME: not POSIX\n"); break;
      case MMAL_ECONFIG : DBG("Component is not configured \attention FIXME: not POSIX\n"); break;
      case MMAL_EISCONN : DBG("Port is already connected\n"); break;
      case MMAL_ENOTCONN : DBG("Port is disconnected\n"); break;
      case MMAL_EAGAIN : DBG("Resource temporarily unavailable. Try again later\n"); break;
      case MMAL_EFAULT : DBG("Bad address\n"); break;
      default : DBG("Unknown status error\n"); break;
    }
  }
}

/******************************************************************************
  Callback from mmal JPEG encoder
 ******************************************************************************/
static void encoder_input_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
  // We pass our file handle and other stuff in via the userdata field.
//  PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

  // release buffer back to the pool
  mmal_buffer_header_release(buffer);

//  vcos_semaphore_post(&(pData->complete_semaphore));
}

static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
#ifdef _SHOW_DBG
    DBG("\n\n\n in encoder output callback: buffer->length=%d \n\n\n", buffer->length);
#endif
  int complete = 0;

  // We pass our file handle and other stuff in via the userdata field.
  PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

  if (pData)
  {
    if (buffer->length)
    {
      mmal_buffer_header_mem_lock(buffer);

      //fprintf(stderr, "The flags are %x of length %i offset %i\n", buffer->flags, buffer->length, pData->offset);

      //Write bytes
      /* copy JPG picture to global buffer */
      if(pData->offset == 0)
        pthread_mutex_lock(&pglobal->in[plugin_number].db);
      
      memcpy(pData->offset + pglobal->in[plugin_number].buf, buffer->data, buffer->length);
      pData->offset += buffer->length;
      //fwrite(buffer->data, 1, buffer->length, pData->file_handle);
      mmal_buffer_header_mem_unlock(buffer);
    }

    // Now flag if we have completed
    if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED))
    {
      complete = 1;
      pglobal->in[plugin_number].size = pData->offset;
      pData->offset = 0;
      /* signal fresh_frame */
      pthread_cond_broadcast(&pglobal->in[plugin_number].db_update);
      pthread_mutex_unlock(&pglobal->in[plugin_number].db); 
    }
  }
#ifdef _SHOW_DBG
  else
  {
    DBG("Received a encoder buffer callback with no state\n");
  }
#endif

  // release buffer back to the pool
  mmal_buffer_header_release(buffer);

  // and send one back to the port (if still open)
  if (port->is_enabled)
  {
    MMAL_STATUS_T status;
    MMAL_BUFFER_HEADER_T *new_buffer;

    new_buffer = mmal_queue_get(pData->pool->queue);

    if (new_buffer)
    {
      status = mmal_port_send_buffer(port, new_buffer);

      if(status != MMAL_SUCCESS)
      {
#ifdef _SHOW_DBG
        DBG("Failed returning a buffer to the encoder port \n");
#endif
	print_mmal_status(status);
      }

    }
#ifdef _SHOW_DBG
    else
    {
      DBG("Unable to return a buffer to the encoder port\n");
    }
#endif
  }

  if (complete)
    vcos_semaphore_post(&(pData->complete_semaphore));
}

//634.4.0818 hiroshi: add camera_buffer_callback function
static void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
#ifdef _SHOW_DBG
    DBG("\n\n\n in cam buf callback: buffer->length=%d \n\n\n", buffer->length);
#endif    
  int complete = 0;
  if (pglobal->camBufCallbackOn == 0)
    pglobal->camBufCallbackOn = 1;
  
//  DBG("\n\n\npglobal->camBufCallbackOn: %d\n\n\n", pglobal->camBufCallbackOn);

  // We pass our file handle and other stuff in via the userdata field.
  PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

  if (pData)
  {
    if (buffer->length)
    {
      mmal_buffer_header_mem_lock(buffer);
      
      //Write bytes
      /* copy JPG picture to global buffer */
      if(pData->offset == 0)
        pthread_mutex_lock(&pglobal->db);
      
      
      memcpy(pData->offset + pglobal->buf, buffer->data, buffer->length);
      // init buf2encoder_input_port: the same as buf at first, will be changed by opencv
//      memcpy(pData->offset + pglobal->buf2encoder_input_port, buffer->data, buffer->length);
      pData->offset += buffer->length;
      //fwrite(buffer->data, 1, buffer->length, pData->file_handle);
      mmal_buffer_header_mem_unlock(buffer);
    }

    // Now flag if we have completed
    if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED))
    {
      complete = 1;
      pglobal->size = pData->offset;
      pData->offset = 0;
      /* signal fresh_frame */
      pthread_cond_broadcast(&pglobal->db_update);
      pthread_mutex_unlock(&pglobal->db); 
    }
  }
#ifdef _SHOW_DBG
  else
  {
    DBG("Received a encoder buffer callback with no state\n");
  }
#endif
  // release buffer back to the pool
  mmal_buffer_header_release(buffer);

  // and send one back to the port (if still open)
  if (port->is_enabled)
  {
    MMAL_STATUS_T status;
    MMAL_BUFFER_HEADER_T *new_buffer;

    new_buffer = mmal_queue_get(pData->pool->queue);

    if (new_buffer)
    {
      status = mmal_port_send_buffer(port, new_buffer);

      if(status != MMAL_SUCCESS)
      {
#ifdef _SHOW_DBG
        DBG("Failed returning a buffer to the encoder port \n");
#endif
        print_mmal_status(status);
      }

    }
#ifdef _SHOW_DBG
    else
    {
      DBG("Unable to return a buffer to the encoder port\n");
    }
#endif
  }

  if (complete)
    vcos_semaphore_post(&(pData->complete_semaphore));
}

/**
 * buffer header callback function for camera control
 *
 * No actions taken in current version
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
  if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
  {
  }
#ifdef _SHOW_DBG
  else
  {
    DBG("Received unexpected camera control callback event");
  }
#endif
  mmal_buffer_header_release(buffer);
}

/******************************************************************************
  Description.: starts the worker thread and allocates memory
  Input Value.: -
  Return Value: 0
 ******************************************************************************/
int input_run(int id)
{
  pglobal->in[id].buf = malloc(width * height * 3);
  // 636.4.0904 hiroshi: init with camera frame buffer size
  pglobal->buf = malloc(width * height * 3);
  // 640.1.0926 hiroshi: init with camera frame buffer size
  pglobal->buf2encoder_input_port = malloc(width * height * 3);

  if (pglobal->in[id].buf == NULL)
  {
    fprintf(stderr, "could not allocate memory\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&worker, 0, worker_thread, NULL) != 0)
  {
    free(pglobal->in[id].buf);
    // 636.4.0904 hiroshi: free frame buffer
    free(pglobal->buf);
    // 636.4.0904 hiroshi: free frame buffer
    free(pglobal->buf2encoder_input_port);

    fprintf(stderr, "could not start worker thread\n");
    exit(EXIT_FAILURE);
  }

  pthread_detach(worker);
  return 0;
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
static MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
  MMAL_STATUS_T status;

  status = mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);

  if (status == MMAL_SUCCESS)
  {
    status = mmal_connection_enable(*connection);
    if (status != MMAL_SUCCESS)
    {
      mmal_connection_destroy(*connection);
      DBG("Error enabling mmal connection\n");
    }
  } else {
    DBG("Error creating mmal connection\n");
  }

  return status;
}


/******************************************************************************
  Description.: print help message
  Input Value.: -
  Return Value: -
 ******************************************************************************/
void help(void)
{
  fprintf(stderr, " ---------------------------------------------------------------\n" \
      " Help for input plugin..: "INPUT_PLUGIN_NAME"\n" \
      " ---------------------------------------------------------------\n" \
      " The following parameters can be passed to this plugin:\n\n" \
      " [-fps | --framerate]...: set video framerate, default 5 frame/sec \n"\
      " [-x | --width ]........: width of frame capture, default 640\n" \
      " [-y | --height]........: height of frame capture, default 480 \n"\
      " [-quality].............: set JPEG quality 0-100, default 85 \n"\
      " [-usestills]...........: uses stills mode instead of video mode \n"\
      " [-preview].............: Enable full screen preview\n"\

      " \n"\
      " -sh  : Set image sharpness (-100 to 100)\n"\
      " -co  : Set image contrast (-100 to 100)\n"\
      " -br  : Set image brightness (0 to 100)\n"\
      " -sa  : Set image saturation (-100 to 100)\n"\
      " -ISO : Set capture ISO\n"\
      " -vs  : Turn on video stablisation\n"\
      " -ev  : Set EV compensation\n"\
      " -ex  : Set exposure mode (see raspistill notes)\n"\
      " -awb : Set AWB mode (see raspistill notes)\n"\
      " -ifx : Set image effect (see raspistill notes)\n"\
      " -cfx : Set colour effect (U:V)\n"\
      " -mm  : Set metering mode (see raspistill notes)\n"\
      " -rot : Set image rotation (0-359)\n"\
      " -hf  : Set horizontal flip\n"\
      " -vf  : Set vertical flip\n"\
      " ---------------------------------------------------------------\n");

}

/******************************************************************************
  Description.: setup mmal and callback
  Input Value.: arg is not used
  Return Value: NULL
 ******************************************************************************/
void *worker_thread(void *arg)
{
  int i = 0;

  /* set cleanup handler to cleanup allocated resources */
  pthread_cleanup_push(worker_cleanup, NULL);
  //Lets not let this thread be cancelled, it needs to clean up mmal on exit
  if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0)
  {
    fprintf(stderr, "Unable to set cancel state\n");
    exit(EXIT_FAILURE);
  }

  IPRINT("Starting Camera\n");

  //Camera variables
  MMAL_COMPONENT_T *camera = 0;
  MMAL_COMPONENT_T *preview = 0;
  MMAL_ES_FORMAT_T *format;
  MMAL_STATUS_T status;
  MMAL_PORT_T *camera_preview_port = NULL;
  MMAL_PORT_T *camera_video_port = NULL;
  MMAL_PORT_T *camera_still_port = NULL;
  MMAL_PORT_T *preview_input_port = NULL;

  MMAL_CONNECTION_T *camera_preview_connection = 0;

  //Encoder variables
  MMAL_COMPONENT_T *encoder = 0;
  MMAL_PORT_T *encoder_input = NULL;
  MMAL_PORT_T *encoder_output = NULL;
  MMAL_POOL_T *pool;
  MMAL_CONNECTION_T *encoder_connection;

  //fps count
  struct timespec t_start, t_finish;
  double t_elapsed;
  int frames;

  //Create camera code
  bcm_host_init();
  DBG("Host init, starting mmal stuff\n");

  status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "error create camera\n");
    exit(EXIT_FAILURE);
  }

  if (!camera->output_num)
  {
    fprintf(stderr, "Camera doesn't have output ports\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
  }

  camera_preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
  camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
  camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

  //Enable camera control port
  // Enable the camera, and tell it its control callback function
  status = mmal_port_enable(camera->control, camera_control_callback);

  if (status)
  {
    fprintf(stderr, "Unable to enable camera port\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
  }


  {
    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
      { MMAL_PARAMETER_CAMERA_CONFIG, sizeof (cam_config)},
      .max_stills_w = width,
      .max_stills_h = height,
      .stills_yuv422 = 0,
      .one_shot_stills = (usestills ? 1 : 0),
      .max_preview_video_w = width,
      .max_preview_video_h = height,
      .num_preview_video_frames = 3,
      .stills_capture_circular_buffer_height = 0,
      .fast_preview_resume = 0,
      .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };
    mmal_port_parameter_set(camera->control, &cam_config.hdr);
  }

  //Set camera parameters
  if (raspicamcontrol_set_all_parameters(camera, &c_params))
    fprintf(stderr, "camera parameters couldn't be set\n");

  // Set the encode format on the Preview port

  format = camera_preview_port->format;
  format->encoding = MMAL_ENCODING_OPAQUE;
  format->es->video.width = VCOS_ALIGN_UP(width, 32);
  format->es->video.height = VCOS_ALIGN_UP(height, 16);
  format->es->video.crop.x = 0;
  format->es->video.crop.y = 0;
  format->es->video.crop.width = width;
  format->es->video.crop.height = height;
  format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
  format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

  status = mmal_port_format_commit(camera_preview_port);

  // Create preview component
  status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview);

  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to create preview component\n");
    exit(EXIT_FAILURE);
  }

  if (!preview->input_num)
  {
    status = MMAL_ENOSYS;
    fprintf(stderr, "No input ports found on preview component");
    exit(EXIT_FAILURE);
  }

  preview_input_port = preview->input[0];

  MMAL_DISPLAYREGION_T param;
  param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
  param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

  param.set = MMAL_DISPLAY_SET_LAYER;
  param.layer = PREVIEW_LAYER;

  param.set |= MMAL_DISPLAY_SET_ALPHA;
  param.alpha = 255;

  param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
  param.fullscreen = 1;

  status = mmal_port_parameter_set(preview_input_port, &param.hdr);

  if (status != MMAL_SUCCESS && status != MMAL_ENOSYS)
  {
   fprintf(stderr, "unable to set preview port parameters (%u)", status);
   exit(EXIT_FAILURE);
  }

  if (!usestills)
  {
    // Set the encode format on the video port
    format = camera_video_port->format;
    format->encoding_variant = MMAL_ENCODING_RGB24; // 633.6.0813 hiroshi: original: I420
    format->encoding = MMAL_ENCODING_RGB24;
    format->es->video.width = width;
    format->es->video.height = height;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = fps;
    format->es->video.frame_rate.den = 1;
    status = mmal_port_format_commit(camera_video_port);
    if (status)
      fprintf(stderr, "camera video format couldn't be set");

#ifdef _camera_callback
            status = mmal_port_enable ( camera_video_port,camera_buffer_callback );
            if ( status ) {
                mmal_component_destroy ( camera );
                return 0;
            }
#endif

// Ensure there are enough buffers to avoid dropping frames
    if (camera_video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      camera_video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    
    // 633.6.0813 hiroshi: add callback function

  }
    // 633.6.0813 hiroshi: add pool
            MMAL_POOL_T *vpool;
            camera_video_port->buffer_size = camera_video_port->buffer_size_recommended;
            camera_video_port->buffer_num = camera_video_port->buffer_num_recommended;
            vpool = mmal_port_pool_create ( camera_video_port, camera_video_port->buffer_num, camera_video_port->buffer_size );

  
  format = camera_still_port->format;

  // Set our stills format on the stills (for encoder) port
  format->encoding = MMAL_ENCODING_OPAQUE;
  format->es->video.width = width;
  format->es->video.height = height;
  format->es->video.crop.x = 0;
  format->es->video.crop.y = 0;
  format->es->video.crop.width = width;
  format->es->video.crop.height = height;
  format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
  format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;

  status = mmal_port_format_commit(camera_still_port);

  if (status)
  {
    fprintf(stderr, "format couldn't be set\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
  }

  /* Ensure there are enough buffers to avoid dropping frames */
  if (camera_still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
    camera_still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


  /* Enable component */
  status = mmal_component_enable(camera);
  if (status)
  {
    fprintf(stderr, "camera couldn't be enabled\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
  }

  /* Enable component */
  status = mmal_component_enable(preview);
  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to enable preview/null sink component (%u)\n", status);
    mmal_component_destroy(preview);
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
  }

  DBG("Camera enabled, creating encoder\n");

  //Create Encoder
  status = mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder);

  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to create JPEG encoder component\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
    if (encoder)
      mmal_component_destroy(encoder);
  }

  if (!encoder->input_num || !encoder->output_num)
  {
    fprintf(stderr, "Unable to create JPEG encoder input/output ports\n");
    mmal_component_destroy(camera);
    exit(EXIT_FAILURE);
    if (encoder)
      mmal_component_destroy(encoder);
  }
  
  encoder_input = encoder->input[0];
  encoder_output = encoder->output[0];

#ifdef _camera_callback
    // Set the encode format on the encoder input port
    format = encoder_input->format;
    format->encoding_variant = MMAL_ENCODING_RGB24; // 633.6.0813 hiroshi: original: I420
    format->encoding = MMAL_ENCODING_RGB24;
    format->es->video.width = width;
    format->es->video.height = height;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = fps;
    format->es->video.frame_rate.den = 1;
    status = mmal_port_format_commit(encoder_input);
    if (status)
    {
      fprintf(stderr, "encoder input format couldn't be set");
#ifdef _SHOW_DBG
      DBG("\n\n\nencoder input format couldn't be set\n\n\n");
#endif
    }
#endif

  // We want same format on input and output
  mmal_format_copy(encoder_output->format, encoder_input->format);

  // Specify out output format JPEG
  encoder_output->format->encoding = MMAL_ENCODING_JPEG;

  encoder_output->buffer_size = encoder_output->buffer_size_recommended;
  encoder_input->buffer_size = encoder_input->buffer_size_recommended; // hiroshi


  if (encoder_output->buffer_size < encoder_output->buffer_size_min)
    encoder_output->buffer_size = encoder_output->buffer_size_min;
  if (encoder_input->buffer_size < encoder_input->buffer_size_min)//hiroshi
    encoder_input->buffer_size = encoder_input->buffer_size_min;

  fprintf(stderr,"Encoder Buffer Size %i\n", encoder_output->buffer_size);

  encoder_input->buffer_num = encoder_input->buffer_num_recommended;//hiroshi
  encoder_output->buffer_num = encoder_output->buffer_num_recommended;

  if (encoder_input->buffer_num < encoder_input->buffer_num_min)//hiroshi
    encoder_input->buffer_num = encoder_input->buffer_num_min;
  if (encoder_output->buffer_num < encoder_output->buffer_num_min)
    encoder_output->buffer_num = encoder_output->buffer_num_min;

  // Commit the port changes to the output port
  status = mmal_port_format_commit(encoder_output);

  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to set video format output ports\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }

  status = mmal_port_format_commit(encoder_input); // hiroshi#10

  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to set video format input ports\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }
  
  // Set the JPEG quality level
  status = mmal_port_parameter_set_uint32(encoder_output, MMAL_PARAMETER_JPEG_Q_FACTOR, quality);

  if (status != MMAL_SUCCESS)
  {
    fprintf(stderr, "Unable to set JPEG quality\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }


  // Enable encoder component
  status = mmal_component_enable(encoder);

  if (status)
  {
    fprintf(stderr, "Unable to enable encoder component\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }

  DBG("Encoder enabled, creating pool and connecting ports\n");

  /* Create pool of buffer headers for the output port to consume */
  pool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);

  if (!pool)
  {
    fprintf(stderr, "Failed to create buffer header pool for encoder output port\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }

  // hiroshi#11
            MMAL_POOL_T *pooli;
  pooli = mmal_port_pool_create(encoder_input, encoder_input->buffer_num, encoder_input->buffer_size);

  if (!pooli)
  {
    fprintf(stderr, "Failed to create buffer header pool for encoder input port\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }

  if (wantPreview)
  {
    // Connect camera to preview
    status = connect_ports(camera_preview_port, preview_input_port, &camera_preview_connection);

    if (status != MMAL_SUCCESS)
      camera_preview_connection = NULL;
  }

      // 634.6.0820 hiroshi: mark connection
#ifndef _camera_callback// 642.4.1013 hiroshi: comment the following line 'cause no connection between cam and encoder input
  // Now connect the camera to the encoder      
  if(usestills){
    status = connect_ports(camera_still_port, encoder->input[0], &encoder_connection);
  } else {
    status = connect_ports(camera_video_port, encoder->input[0], &encoder_connection);
  }

  if (status)
  {
    fprintf(stderr, "Unable to connect components\n");
    if (camera_preview_connection)
      mmal_connection_destroy(camera_preview_connection);
    if (preview)
      mmal_component_destroy(preview);
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }
#endif
  // Set up our userdata - this is passed though to the callback where we need the information.
  // Null until we open our filename
  PORT_USERDATA callback_data;
  callback_data.file_handle = NULL;
  callback_data.pool = pool;
  callback_data.offset = 0;

  vcos_assert(vcos_semaphore_create(&callback_data.complete_semaphore, "RaspiStill-sem", 0) == VCOS_SUCCESS);

  encoder->output[0]->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;


//#ifdef _camera_callback
  //634.4.0818 hiroshi*6
  PORT_USERDATA callback_datav;
  callback_datav.file_handle = NULL;
  callback_datav.pool = vpool;
  callback_datav.offset = 0;

  vcos_assert(vcos_semaphore_create(&callback_datav.complete_semaphore, "RaspiStill-sem2", 0) == VCOS_SUCCESS);

  camera->output[MMAL_CAMERA_VIDEO_PORT]->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_datav;
//#endif

  // Enable the encoder output port and tell it its callback function
  status = mmal_port_enable(encoder->output[0], encoder_buffer_callback);
  if (status)
  {
    fprintf(stderr, "Unable to enable encoder component\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }

  // hiroshi#9
#ifdef _camera_callback
  status = mmal_port_enable(encoder->input[0], encoder_input_buffer_callback);
  if (status)
  {
    fprintf(stderr, "Unable to enable encoder input port component\n");
    mmal_component_destroy(camera);
    if (encoder)
      mmal_component_destroy(encoder);
    exit(EXIT_FAILURE);
  }
#endif

  if(usestills){
    DBG("Starting stills output\n");

    //setup fps
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    frames = 0;
    int delay = (1000 * 1000) / fps;

    while(!pglobal->stop) {
      //Wait the delay
      usleep(delay);

      // Send all the buffers to the encoder output port
      int num = mmal_queue_length(pool->queue);
      int q;
      for (q=0;q<num;q++)
      {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);

        if (!buffer)
          fprintf(stderr, "Unable to get a required buffer from pool queue");

        if (mmal_port_send_buffer(encoder->output[0], buffer)!= MMAL_SUCCESS)
          fprintf(stderr, "Unable to send a buffer to encoder output port");
      }

      if (mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
      {
        fprintf(stderr, "starting captue failed");
      }
      else
      {
        // Wait for capture to complete
        // For some reason using vcos_semaphore_wait_timeout sometimes returns immediately with bad parameter error
        // even though it appears to be all correct, so reverting to untimed one until figure out why its erratic
        vcos_semaphore_wait(&callback_data.complete_semaphore);
        //DBG("Jpeg Captured\n");
        frames++;
      }

      frames++;
      if (frames == 100)
      {
        //calculate fps      
        clock_gettime(CLOCK_MONOTONIC, &t_finish);
        t_elapsed = (t_finish.tv_sec - t_start.tv_sec);
        t_elapsed += (t_finish.tv_nsec - t_start.tv_nsec) / 1000000000.0;
        fprintf(stderr, "%i frames captured in %f seconds (%f fps)\n", frames, t_elapsed, (frames / t_elapsed));
        frames = 0;
        clock_gettime(CLOCK_MONOTONIC, &t_start);
      }
    }
  } 
  else
  { //if(usestills)
    //Video Mode

#ifdef _camera_callback
    DBG("Starting camera output\n");
    // Send all the buffers to the video output port
    int numv = mmal_queue_length(vpool->queue);
#ifdef _SHOW_DBG
    DBG("\nvideo queue length: %d\n", numv);
#endif
    int p;
    for (p=0;p<numv;p++)
    {
      MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(vpool->queue);
#ifdef _SHOW_DBG
      DBG("\n\n\n buffer: %d\n\n\n", buffer);
#endif
      if (!buffer)
      {
        fprintf(stderr, "Unable to get a required buffer from vpool queue");
#ifdef _SHOW_DBG
	DBG("\n\n\n buffer not created for video port \n\n\n");
#endif
    }
      else
#ifdef _SHOW_DBG
	DBG("\n\n\n buffer created for video port \n\n\n");
#endif
	
      if (mmal_port_send_buffer(camera->output[MMAL_CAMERA_VIDEO_PORT], buffer)!= MMAL_SUCCESS)
      {
        fprintf(stderr, "Unable to send a buffer to camera output port");
#ifdef _SHOW_DBG
	DBG("\n\n\n buffer not sent to video port\n\n\n");
#endif
      }
#ifdef _SHOW_DBG
      else
  	DBG("\n\n\n buffer sent to video port\n\n\n");
#endif

    }

//    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
//      fprintf(stderr, "starting capture failed");

//    while(!pglobal->stop) usleep(1000);




#endif

    DBG("Starting video output\n");
    // Send all the buffers to the encoder output port
    int num = mmal_queue_length(pool->queue);
    DBG("\n encode queue length: %d\n", num);
    int q;
    for (q=0;q<num;q++)
    {
      MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
#ifdef _SHOW_DBG
      DBG("\n\n\n buffer: %d\n\n\n", buffer);
#endif
      if (!buffer)
      {
        fprintf(stderr, "Unable to get a required buffer from pool queue");
#ifdef _SHOW_DBG
	DBG("\n\n\n buffer not created for encoder output port \n\n\n");
#endif
      }

      if (mmal_port_send_buffer(encoder->output[0], buffer)!= MMAL_SUCCESS)
      {
#ifdef _SHOW_DBG
  	DBG("\n\n\n buffer NOT sent to encoder output port\n\n\n");
#endif
	fprintf(stderr, "Unable to send a buffer to encoder output port");
      }
#ifdef _SHOW_DBG
      else
  	DBG("\n\n\n buffer sent to encoder output port\n\n\n");
#endif
    }

#ifdef _SHOW_DBG
    DBG("\n\n\n ready to mmal_port_parameter_set_boolean... \n\n\n");
#endif
    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
    {
	fprintf(stderr, "starting capture failed");
#ifdef _SHOW_DBG
  	DBG("\n\n\n mmal_port_parameter_set_boolean FAIL \n\n\n");
#endif
    }
#ifdef _SHOW_DBG
    else
  	DBG("\n\n\n mmal_port_parameter_set_boolean success \n\n\n");
#endif
    
usleep(1000000);
    while(!pglobal->stop) 
    {
	usleep(100000);
	
// 635.2.0823 hiroshi: send buffer to encoder input port
//	int numi = mmal_queue_length(pooli->queue);
//	DBG("\n\n\nnum=%d\n\n\n", numi);
//	int r;
//	for (r=0;r<numi;r++)
//	{
	    MMAL_BUFFER_HEADER_T *buffer;
	    if( (buffer = mmal_queue_get(pooli->queue)) != NULL)
	    {

	    if (!buffer)
	    {
		fprintf(stderr, "Unable to get a required buffer from pooli queue");
#ifdef _SHOW_DBG
		DBG("\n\n\n unable to get buffer from pooli queue \n\n\n");
#endif
	    }
      
#ifdef _SHOW_DBG
	    DBG("\n\n\n pglobal->camBufCallbackOn: %d \n\n\n", pglobal->camBufCallbackOn);
#endif
	    
	    if (  pglobal->camBufCallbackOn ==1 )
	    {
    /* wait for a fresh frame */
#ifdef _SHOW_DBG
		DBG("\n\n\n ready to unlock \n\n\n");
#endif
//    pthread_mutex_lock(&pglobal->db);
  //  pthread_cond_wait(&pglobal->db_update, &pglobal->db);

#ifdef _SHOW_DBG
		DBG("\n\n\nglobal buffer: %d size: %d\n\n\n", pglobal->buf2encoder_input_port, pglobal->size);
#endif
		buffer->data = malloc(pglobal->size);
		buffer->length = pglobal->size;
		memcpy(buffer->data, pglobal->buf2encoder_input_port, pglobal->size);
#ifdef _SHOW_DBG
		DBG("\n\n\nbuffer: %d size: %d\n\n\n", buffer->data, buffer->length);
#endif	
		if (mmal_port_send_buffer(encoder->input[0], buffer)!= MMAL_SUCCESS)
		{
#ifdef _SHOW_DBG
		    DBG("\n\n\n buffer NOT sent to encoder input port\n\n\n");
#endif
		    fprintf(stderr, "Unable to send a buffer to encoder input port");
		}
		else
#ifdef _SHOW_DBG
		    DBG("\n\n\n buffer sent to encoder input port\n\n\n");
#endif	
		free(buffer->data);
    //  pthread_cond_broadcast(&pglobal->db_update);
      //pthread_mutex_unlock(&pglobal->db); 
	    
	    }
	    }
	//}

	
    }//////////
    
    
    
  }
  
  ///642.1.1010 hiroshi: cv_thread waits this flag to tell streaming-end, so ends itself
  /// add flg here, something like "cam_end = true"
  // in fact, it should be "pglobal->stop" (already exists!)
#ifdef _SHOW_DBG  
  DBG("\n\n\nbefore vcos delete\n\n\n");
#endif
  
#ifdef _camera_callback  
  vcos_semaphore_delete(&callback_datav.complete_semaphore);
#endif
  vcos_semaphore_delete(&callback_data.complete_semaphore);


#ifdef _SHOW_DBG
  DBG("\n\n\n after vcos delete \n\n\n");
#endif  
  
  /////////////////////////
  //Close everything MMAL//
  /////////////////////////
  if (usestills)
  {
    if (camera_video_port && camera_video_port->is_enabled)
      mmal_port_disable(camera_video_port);
  }
  else
  {
    if (camera_still_port && camera_still_port->is_enabled)
      mmal_port_disable(camera_still_port);
  }

  if (camera_preview_connection)
    mmal_connection_destroy(camera_preview_connection);

  if (encoder->output[0] && encoder->output[0]->is_enabled)
    mmal_port_disable(encoder->output[0]);

#ifndef _camera_callback
  mmal_connection_destroy(encoder_connection);
#endif
  
  // Disable components
  if (encoder)
    mmal_component_disable(encoder);
  if (preview)
    mmal_component_disable(preview);
  if (camera)
    mmal_component_disable(camera);
  
  //Destroy encoder component
  // Get rid of any port buffers first
  if (pool)
  {
    mmal_port_pool_destroy(encoder->output[0], pool);
  }

  //destroy camera component
  // 633.6.0813 hiroshi: destroy vpool
  if (vpool)
  {
    mmal_port_pool_destroy(camera->output[MMAL_CAMERA_VIDEO_PORT], vpool);
  }

  if (pooli)
  {
    mmal_port_pool_destroy(encoder->input[0], pooli);
  }

  if (encoder)
  {
    mmal_component_destroy(encoder);
    encoder = NULL;
  }
  if (preview)
  {
    mmal_component_destroy(preview);
    preview = NULL;
  }

  if (camera)
  {
    mmal_component_destroy(camera);
    camera = NULL;
  }

  DBG("mmal cleanup done\n");
  pthread_cleanup_pop(1);

  return NULL;
}

/******************************************************************************
  Description.: this functions cleans up allocated resources
  Input Value.: arg is unused
  Return Value: -
 ******************************************************************************/
void worker_cleanup(void *arg)
{
  static unsigned char first_run = 1;

  if (!first_run)
  {
    DBG("already cleaned up resources\n");
    return;
  }

  first_run = 0;
  DBG("cleaning up resources allocated by input thread\n");

  if(pglobal->in[plugin_number].buf != NULL)
    free(pglobal->in[plugin_number].buf);
  // 636.4.0901 hiroshi#2: free frame buffer
  if(pglobal->buf != NULL)
    free(pglobal->buf);
  // 636.4.0901 hiroshi#2: free frame buffer
  if(pglobal->buf2encoder_input_port != NULL)
    free(pglobal->buf2encoder_input_port);
}




