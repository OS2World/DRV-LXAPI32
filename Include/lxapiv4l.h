/* $Id: lxapiv4l.h,v 1.6 2004/05/27 20:33:34 smilcke Exp $ */

/*
 * lxapiv4l.h
 * Autor:               Stefan Milcke
 * Erstellt am:         14.01.2002
 * Letzte Aenderung am: 27.10.2003
 *
*/

#ifndef LXAPIV4L_H_INCLUDED
#define LXAPIV4L_H_INCLUDED

#pragma pack(1)

#define V4L_IOCTLFN_GCAP                                    0x001
#define V4L_IOCTLFN_GCHAN                                   0x002
#define V4L_IOCTLFN_SCHAN                                   0x003
#define V4L_IOCTLFN_GTUNER                                  0x004
#define V4L_IOCTLFN_STUNER                                  0x005
#define V4L_IOCTLFN_GPICT                                   0x006
#define V4L_IOCTLFN_SPICT                                   0x007
#define V4L_IOCTLFN_CAPTURE                                 0x008
#define V4L_IOCTLFN_GWIN                                    0x009
#define V4L_IOCTLFN_SWIN                                    0x00a
#define V4L_IOCTLFN_GFBUF                                   0x00b
#define V4L_IOCTLFN_SFBUF                                   0x00c
#define V4L_IOCTLFN_KEY                                     0x00d
#define V4L_IOCTLFN_GFREQ                                   0x00e
#define V4L_IOCTLFN_SFREQ                                   0x00f
#define V4L_IOCTLFN_GAUDIO                                  0x010
#define V4L_IOCTLFN_SAUDIO                                  0x011
#define V4L_IOCTLFN_SYNC                                    0x012
#define V4L_IOCTLFN_MCAPTURE                                0x013
#define V4L_IOCTLFN_GMBUF                                   0x014
#define V4L_IOCTLFN_GUNIT                                   0x015
#define V4L_IOCTLFN_GCAPTURE                                0x016
#define V4L_IOCTLFN_SCAPTURE                                0x017
#define V4L_IOCTLFN_SPLAYMODE                               0x018
#define V4L_IOCTLFN_SWRITEMODE                              0x019
#define V4L_IOCTLFN_GPLAYINFO                               0x01a
#define V4L_IOCTLFN_SMICROCODE                              0x01b
#define V4L_IOCTLFN_GVBIFMT                                 0x01c
#define V4L_IOCTLFN_SVBIFMT                                 0x01d

#define V4L_IOCTLFN_SFREQSTEP                               0x021
#define V4L_IOCTLFN_VBIBUF_SIZE                             0x022

#define V4L_IOCTLFN_SRADIO                                  0x102
#define V4L_IOCTLFN_SINPUT                                  0x111


// Define some types here
#ifndef _I386_TYPES_H
typedef signed char __s8;
typedef unsigned char __u8;
typedef signed short __s16;
typedef unsigned short __u16;
typedef signed int __s32;
typedef unsigned int __u32;
#endif

typedef struct _V4L_VIDEO_CAPABILITY
{
 char name[32];
 int type;
#define VID_TYPE_CAPTURE         1      /* Can capture */
#define VID_TYPE_TUNER           2      /* Can tune */
#define VID_TYPE_TELETEXT        4      /* Does teletext */
#define VID_TYPE_OVERLAY         8      /* Overlay onto frame buffer */
#define VID_TYPE_CHROMAKEY       16     /* Overlay by chromakey */
#define VID_TYPE_CLIPPING        32     /* Can clip */
#define VID_TYPE_FRAMERAM        64     /* Uses the frame buffer memory */
#define VID_TYPE_SCALES          128    /* Scalable */
#define VID_TYPE_MONOCHROME      256    /* Monochrome only */
#define VID_TYPE_SUBCAPTURE      512    /* Can capture subareas of the image */
#define VID_TYPE_MPEG_DECODER    1024   /* Can decode MPEG streams */
#define VID_TYPE_MPEG_ENCODER    2048   /* Can encode MPEG streams */
#define VID_TYPE_MJPEG_DECODER   4096   /* Can decode MJPEG streams */
#define VID_TYPE_MJPEG_ENCODER   8192   /* Can encode MJPEG streams */
 int channels; // Num channels
 int audios;   // Num audio devices
 int maxwidth; // Supported width
 int maxheight;// Supported height
 int minwidth; // Supported width
 int minheight;// Supported height
}V4L_VIDEO_CAPABILITY,*PV4L_VIDEO_CAPABILITY;

typedef struct _V4L_VIDEO_CHANNEL
{
 int channel;
 char name[32];
 int tuners;
 __u32 flags;
#define VIDEO_VC_TUNER     1  // Channel has a tuner
#define VIDEO_VC_AUDIO     2  // Channel has audio
 __u16 type;
#define VIDEO_TYPE_TV      1
#define VIDEO_TYPE_CAMERA  2
 __u16 norm;                 // Norm set by channel
}V4L_VIDEO_CHANNEL,*PV4L_VIDEO_CHANNEL;

typedef struct _V4L_VIDEO_TUNER
{
 int tuner;
 char name[32];
 unsigned long rangelow,rangehigh;  // Tuner range
 __u32 flags;
#define VIDEO_TUNER_PAL       1
#define VIDEO_TUNER_NTSC      2
#define VIDEO_TUNER_SECAM     4
#define VIDEO_TUNER_LOW       8     // Uses KHz not MHz
#define VIDEO_TUNER_NORM      16    // Tuner can set norm
#define VIDEO_TUNER_STEREO_ON 128   // Tuner is seeing stereo
#define VIDEO_TUNER_RDS_ON    256   // Tuner is seeing an RDS datastream
#define VIDEO_TUNER_MBS_ON    512   // Tuner is seeing an MBS datastream
 __u16 mode;                 // PAL/NTSC/SECAM/OTHER
#define VIDEO_MODE_PAL        0
#define VIDEO_MODE_NTSC       1
#define VIDEO_MODE_SECAM      2
#define VIDEO_MODE_AUTO       3
 __u16 signal;               // Signal strength 16bit scale
}V4L_VIDEO_TUNER,*PV4L_VIDEO_TUNER;

typedef struct _V4L_VIDEO_PICTURE
{
 __u16 brightness;
 __u16 hue;
 __u16 colour;
 __u16 contrast;
 __u16 whiteness;             // Black and white only
 __u16 depth;                 // Capture depth
 __u16 palette;               // Palette in use
#define VIDEO_PALETTE_GREY       1     // Linear greyscale
#define VIDEO_PALETTE_HI240      2     // High 240 cube (BT848)
#define VIDEO_PALETTE_RGB565     3     // 565 16 bit RGB
#define VIDEO_PALETTE_RGB24      4     // 24bit RGB
#define VIDEO_PALETTE_RGB32      5     // 32bit RGB
#define VIDEO_PALETTE_RGB555     6     // 555 15bit RGB
#define VIDEO_PALETTE_YUV422     7     // YUV422 capture
#define VIDEO_PALETTE_YUYV       8
#define VIDEO_PALETTE_UYVY       9     // The great thing about this standards is ...
#define VIDEO_PALETTE_YUV420     10
#define VIDEO_PALETTE_YUV411     11    // YUV411 capture
#define VIDEO_PALETTE_RAW        12    // RAW capture (BT848)
#define VIDEO_PALETTE_YUV422P    13    // YUV 4:2:2 Planar
#define VIDEO_PALETTE_YUV411P    14    // YUV 4:1:1 Planar
#define VIDEO_PALETTE_YUV420P    15    // YUV 4:2:0 Planar
#define VIDEO_PALETTE_YUV410P    16    // YUV 4:1:0 Planar
#define VIDEO_PALETTE_PLANAR     13    // start of planar entries
#define VIDEO_PALETTE_COMPONENT  7     // start of component entries
}V4L_VIDEO_PICTURE,*PV4L_VIDEO_PICTURE;

typedef struct _V4L_VIDEO_CLIP
{
 __s32 x,y;
 __s32 width,height;
 struct _V4L_VIDEO_CLIP *next;   // For user use/driver use only
}V4L_VIDEO_CLIP,*PV4L_VIDEO_CLIP;

typedef struct _V4L_VIDEO_WINDOW
{
 __u32 x,y;                   // Position of window
 __u32 width,height;          // Size of window
 __u32 chromakey;
 __u32 flags;
 struct _V4L_VIDEO_CLIP *clips;
 int clipcount;
#define VIDEO_WINDOW_INTERLACE   1
#define VIDEO_WINDOW_CHROMAKEY   16    // Overlay by chromakey
#define VIDEO_CLIP_BITMAP        -1
#define VIDEO_CLIPMAP_SIZE       (128 * 625) // bitmap is 1024x625, a '1' bit represents a clipped pixel
}V4L_VIDEO_WINDOW,*PV4L_VIDEO_WINDOW;

typedef struct _V4L_VIDEO_BUFFER
{
 void *base;
 int height,width;
 int depth;
 int bytesperline;
}V4L_VIDEO_BUFFER,*PV4L_VIDEO_BUFFER;

typedef struct _V4L_VIDEO_KEY
{
 __u8 key[8];
 __u32 flags;
}V4L_VIDEO_KEY,*PV4L_VIDEO_KEY;

typedef struct _V4L_VIDEO_AUDIO
{
 int audio;                   // Audio channel
#define AUDIO_TUNER              0
#define AUDIO_RADIO              1
#define AUDIO_EXTERN             2
#define AUDIO_INTERN             3
#define AUDIO_OFF                4
#define AUDIO_ON                 5
 __u16 volume;                // If settable
 __u16 bass,treble;
 __u32 flags;
#define VIDEO_AUDIO_MUTE         1
#define VIDEO_AUDIO_MUTABLE      2
#define VIDEO_AUDIO_VOLUME       4
#define VIDEO_AUDIO_BASS         8
#define VIDEO_AUDIO_TREBLE       16
 char name[16];
#define VIDEO_SOUND_MONO         1
#define VIDEO_SOUND_STEREO       2
#define VIDEO_SOUND_LANG1        4
#define VIDEO_SOUND_LANG2        8
 __u16 mode;
 __u16 balance;               // Stereo balance
 __u16 step;                  // Step actual volume uses
}V4L_VIDEO_AUDIO,*PV4L_VIDEO_AUDIO;

#define VIDEO_MAX_FRAME          32
typedef struct _V4L_VIDEO_MMAP
{
 unsigned int frame;          // Frame (0-n) for double buffer
 int height,width;
 unsigned int format;         // shoud be VIDEO_PALETTE_*
}V4L_VIDEO_MMAP,*PV4L_VIDEO_MMAP;

typedef struct _V4L_VIDEO_MBUF
{
 int size;                    // Total memory to map
 int frames;                  // Frames
 int offsets[VIDEO_MAX_FRAME];
}V4L_VIDEO_MBUF,*PV4L_VIDEO_MBUF;

#define VIDEO_NO_UNIT         (-1)
typedef struct _V4L_VIDEO_UNIT
{
 int video;                   // Video minor
 int vbi;                     // VBI minor
 int radio;                   // Radio minor
 int audio;                   // Audio minor
 int teletext;                // Teletext minor
}V4L_VIDEO_UNIT,*PV4L_VIDEO_UNIT;

typedef struct _V4L_VIDEO_CAPTURE
{
 __u32 x,y;                   // Offsets into image
 __u32 width,height;          // Area to capture
 __u16 decimation;            // Decimation divider
 __u16 flags;                 // Flags for capture
#define VIDEO_CAPTURE_ODD     0     // Temporal
#define VIDEO_CAPTURE_EVEN    1
}V4L_VIDEO_CAPTURE,*PV4L_VIDEO_CAPTURE;

typedef struct _V4L_VIDEO_PLAY_MODE
{
 int mode;
 int p1;
 int p2;
}V4L_VIDEO_PLAY_MODE,*PV4L_VIDEO_PLAY_MODE;

typedef struct _V4L_VIDEO_INFO
{
 __u32 frame_count;           // frames output since decode/encode began
 __u32 h_size;                // current unscaled horizontal size
 __u32 v_size;                // current unscaled vertical size
 __u32 smpte_timecode;        // current SMPTE timecode (for current GOP)
 __u32 picture_type;          // current picture type
 __u32 temporal_reference;    // current temporal reference
 __u8 user_data[256];        // user data last found in compressed stream
 // user_data[0] contains user data flags, user_data[1] has count
}V4L_VIDEO_INFO,*PV4L_VIDEO_INFO;

typedef struct _V4L_VIDEO_CODE
{
 char loadwhat[16];           // name or tag of file being passed
 int datasize;
 __u8 *data;
}V4L_VIDEO_CODE,*PV4L_VIDEO_CODE;

typedef struct _V4L_VBI_FORMAT
{
 __u32 sampling_rate;         // in Hz
 __u32 samples_per_line;
 __u32 sample_format;         // VIDEO_PALETTE_RAW only (1 byte)
 __s32 start[2];              // Starting line for each frame
 __u32 count[2];              // Count of lines for each frame
 __u32 flags;
#define VBI_UNSYNC            1     // can distingues between top/bottom field
#define VBI_INTERLACED        2     // lines are interlaced
}V4L_VBI_FORMAT,*PV4L_VBI_FORMAT;

#pragma pack()
#endif //LXAPIV4L_H_INCLUDED
