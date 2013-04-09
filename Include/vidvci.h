/* $Id: vidvci.h,v 1.3 2004/05/27 20:33:35 smilcke Exp $ */

/* VCA32.H */

#define VIDEO_IOCTL_CAT   140

#define VCAERR_SUCCESS            0
#define VCAERR_INVALID_BUFFER     1
#define VCAERR_INVALID_RECT       2
#define VCAERR_INVALID_PARM       3
#define VCAERR_UNSUPPORTED_CMD    4
#define VCAERR_HW_ERROR           5
                                          /* KLL New Tuner Functions - Start */
#define VCAERR_CHANNEL_TOO_LOW       6
#define VCAERR_CHANNEL_TOO_HIGH      7
#define VCAERR_CHANNEL_SKIP          8
#define VCAERR_CHANNEL_NO_TUNER      9
#define VCAERR_SIGNAL_LOCKED        10
#define VCAERR_SIGNAL_NOT_LOCKED    11
#define VCAERR_SIGNAL_INDETERMINATE 12
                                          /* KLL New Tuner Functions - End   */

/* IOCTL Commands */
#define  VCAI_INIINFO          0x60
#define  VCAI_SAVE             0x61
#define  VCAI_RESTORE          0x62
                                  /* KLL New Image Restore Functions - Start */
#define  VCAI_LOAD_MICROCODE   0x63
#define  VCAI_RESTORE_FORMAT   0x64
#define  VCAI_CAPTURE_FORMAT   0x65
#define  VCAI_RESTORE_IMAGE    0x66
#define  VCAI_PLAY             0x67
                                  /* KLL New Image Restore Functions - End   */
                                  /* KLL New Tuner Functions - Start         */
#define  VCAI_QUERYVIDEOSIGNAL 0x68
#define  VCAI_TUNERCHANNEL     0x69
                                  /* KLL New Tuner Functions - End           */
#define  VCAI_VIDEOINPUT       0x6A
#define  VCAI_SETCAPTRECT      0x6B
#define  VCAI_GETIMAGESCALE    0x6C
#define  VCAI_GETDEVINFO       0x6D
#define  VCAI_VALIDRECT        0x6E
#define  VCAI_UNFREEZE         0x72
#define  VCAI_FREEZE           0x74
#define  VCAI_VIDEOADJ         0x75
#define  VCAI_SETFPS           0x76
#define  VCAI_USER             0x79
#define  VCAI_AUDIOINPUT       0x7A
#define  VCAI_VIDEOINOUT       0x7B
#define  VCAI_SCREENINFO       0x7C
#define  VCAI_SETMONITOR       0x80
#define  VCAI_EDCOLORKEY       0x81
#define  VCAI_SETCOLORKEY      0x82
#define  VCAI_SETCLIPLIST      0x83
#define  VCAI_SETDATATYPE      0x84


/* Common Cature Rectangle Define */
typedef struct CRECT {     /* CR */
   ULONG X_Left;
   ULONG Y_Top;
   ULONG Y_Height;
   ULONG X_Width;
} CRECT;

/* XLATOFF */
#pragma pack(1)
/* XLATON */

/* IOCTL category 140 code 6Ah - Set Video Input Source Connector     */
/*                             & Query Current Input Source Connector */
typedef struct VCASETVIDEOINPUT{      /* VI */
   ULONG  INPUT_CONNECTOR;  /* -1 NO_Change Returns Current Setting */
} VCASETVIDEOINPUT;
typedef VCASETVIDEOINPUT FAR * PVCASETVIDEOINPUT;

/* IOCTL category 140 code 7Bh - Connect/disconnect a video output connector*/
/*                               to/from a video input connector. Or        */
/*                               Query if a output connector is connectored */
/*                               to a specific Input Connector              */
/*  On a VIO_SET all fields are input.                                      */
/*  On a VIO_QUERY all fields are input except OUTPUT_ENABLE which returns  */
/*  if a output connector is enabled with a specific input connector.      .*/
typedef struct VCAVIDEOINOUT{      /* VIO */
   ULONG  Length;            /* Length of this structure                   */
   ULONG  Flags;             /* Set or Query                               */
   ULONG  Input_Connector;   /* Input Connector to Set/Query               */
   ULONG  Output_Connector;  /* Output Connector                           */
   ULONG  Output_Enable;     /* Input on VIO_SET. Returned on VIO_Query    */
} VCAVIDEOINOUT;
typedef VCAVIDEOINOUT FAR * PVCAVIDEOINOUT;

/* Flags for VCAVIDEOINOUT */
#define VIO_SET           1
#define VIO_QUERY         0

/* Output_Enable values for VCAVIDEOINOUT */
#define VIO_ENABLE        1
#define VIO_DISABLE       0

/* IOCTL category 140 code 6Bh - Set Source and Destination Capture Rectangles */
typedef struct VCASETCAPTURERECT{      /* CR */
   ULONG  Source_X_Left;
   ULONG  Source_Y_Top;
   ULONG  Source_Y_Height;
   ULONG  Source_X_Width;
   ULONG  Dest_X_Left;
   ULONG  Dest_Y_Top;
   ULONG  Dest_Y_Height;
   ULONG  Dest_X_Width;
} VCASETCAPTURERECT;
typedef VCASETCAPTURERECT FAR * PVCASETCAPTURERECT;

/* IOCTL category 140 code 6Ch - Get Image and Scale Into RAM Buffer */
typedef struct VCAGETIMAGESCALE{      /* GIS */
   ULONG  Capture_Buf_Len;
   ULONG  Capture_Buf_Ptr;
   ULONG  Source_X_Left;
   ULONG  Source_Y_Top;
   ULONG  Source_Y_Height;
   ULONG  Source_X_Width;
   ULONG  Dest_X_Left;
   ULONG  Dest_Y_Top;
   ULONG  Dest_Y_Height;
   ULONG  Dest_X_Width;
} VCAGETIMAGESCALE;
typedef VCAGETIMAGESCALE FAR * PVCAGETIMAGESCALE;


/* IOCTL category 140 code 6Dh - Get Devinfo */
typedef struct _vcadevinfo {     /* DI */
   ULONG  Length;
   CHAR   ProdInfo[30];
   CHAR   ManInfo[30];
   CHAR   Version[10];
   ULONG  ImgFormat;        /* Image Format Supported by the card            */
   USHORT BitsPerPEL;       /* Bit Per PEL in this image format              */
   USHORT Overlay;          /* Device has overlay support                    */
   ULONG  Brightness;       /* Default Video Attributes for the card         */
   ULONG  hue;
   ULONG  saturation;
   ULONG  contrast;
   ULONG  Sharpness;
   ULONG  unused1;
   ULONG  S_X_Left;         /* Default Source Coordinates                    */
   ULONG  S_Y_Top;
   ULONG  S_Y_Height;
   ULONG  S_X_Width;
   ULONG  D_X_Left;         /* Default Destination Coordinates               */
   ULONG  D_Y_Top;
   ULONG  D_Y_Height;
   ULONG  D_X_Width;
   ULONG  D_ScaleFactor;    /* Default Scale Factor on copy                  */
   ULONG  S_X_MAX;          /* Maximum X size for the digitized Source       */
   ULONG  S_Y_MAX;          /* Maximun Y size for the digitized Source       */
   ULONG  D_X_MAX;          /* Maximun X size for the Destination            */
   ULONG  D_Y_MAX;          /* Maximun Y size for the Destination            */
   ULONG  O_X_MAX;          /* Maximun X size for the Overlay Destination    */
   ULONG  O_Y_MAX;          /* Maximun Y size for the Overlay Destination    */
   USHORT VideoInputs;      /* Number of Software Switchable video Inputs    */
   USHORT CanRestore;
   USHORT CanStretch;
   USHORT CanDistort;
   USHORT HasVolume;        /* Has Volume  Control                           */
   USHORT HasBalance;       /* Has Balance Control                           */
   USHORT CanScale;         /* Can Scale Down on GetImage                    */
   USHORT CanStream;        /* Can do streaming of Images to Stream Handler  */
   ULONG  ulFileNum;        /* System File Number used in Streaming          */
   //////////// New Items after 1.0 release below //////////////////////////
   BYTE   HasTuner;         /* Card Has a Channel Tuner                      */
   BYTE   HasTeleTex;       /* Card Has a TeleTex support                    */
   LONG   Delay_Time;       /* MS delay between connector change/query signal*/
   BYTE   HasAFC;           /* Automatic Frequency Control/FineTune          */
   BYTE   HasPolarization;  /* Support Video Frequency Polarization          */
   //////////// New Items after 1.1 release below //////////////////////////
   USHORT VideoOutputs;     /* Number of Software Switchable video Outputs   */
} VCADEVINFO;

typedef VCADEVINFO FAR * PVCADEVINFO;

/* XLATOFF */
#pragma pack()
/* XLATON */

#define DI_IMAGEFORMAT_RGB_565        1; /* bits 0-4 =Red, 5-10 =Green, 11-15 Blue   */
#define DI_IMAGEFORMAT_YUV_411        2; /* 4 bytes of Y, 1 Byte of U, 1 Byte of V   */
#define DI_BITSPERPEL_16             16; /* 16 Bits Per PEL                          */
#define DI_NotSupported              -1; /* Not Supported                            */
#define DI_Supported                  1; /* Supported                                */

/* IOCTL category 140 code 6Eh - Set Streaming Capture Image Size */
typedef struct VCACAPISIZE {     /* CS */
   ULONG X_Left;
   ULONG Y_Top;
   ULONG Y_Height;
   ULONG X_Width;
   ULONG ScaleFactor;
} VCACAPISIZE;
typedef VCACAPISIZE FAR * PVCACAPISIZE;

/* IOCTL category 140 code 6Fh - Get Image Into RAM Buffer */
typedef struct VCAGETIMAGE{      /* GI */
   ULONG  Capture_Buf_Len;
   ULONG  Capture_Buf_Ptr;
} VCAGETIMAGE;
typedef VCAGETIMAGE FAR * PVCAGETIMAGE;


/* IOCTL category 140 code 70h - get buffer addressing */
typedef struct VCABUFFER{        /* BA */
   ULONG buf_addr;   /* 32 bit linear address                                 */
   ULONG buf_len;    /* buffer length in bytes                                */
   ULONG buf_banks;  /* number of banks (1 if full aperature)                 */
} VCABUFFER;
typedef VCABUFFER FAR * PVCABUFFER;

/* IOCTL category 140 code 71h - set current VRAM bank number */
typedef struct VCASELECTBANK{    /* SB */
   ULONG bank_num;   /* number of bank to select (0-39)                       */
} VCASELECTBANK;
typedef VCASELECTBANK FAR * PVCASELECTBANK;

/* IOCTL category 140 code 75h - set/query video adjustments  */
typedef struct VCASETVIDEO{      /* SV */
   ULONG set_brightness;   /* 1=min, 255=max, 128=norm, -1=no change, -2=reset default   */
   ULONG set_hue;          /* 1=min, 255=max, 128=neutral, -1=no change, -2=reset default*/
   ULONG set_saturation;   /* 1=min, 255=max, 128=norm, -1=no change, -2=reset default   */
   ULONG set_contrast;     /* 1=min, 255=max, 128=norm, -1=no change, -2=reset default   */
   ULONG ret_brightness;   /* 1=min, 255=max */
   ULONG ret_hue;          /* 1=min, 255=max */
   ULONG ret_saturation;   /* 1=min, 255=max */
   ULONG ret_contrast;     /* 1=min, 255=max */
} VCASETVIDEO;
typedef VCASETVIDEO FAR * PVCASETVIDEO;

/* IOCTL category 140 code 76h - set Frame Rate for Streaming */
typedef struct VCASETFPS{        /* SF */
   ULONG set_FPS;          /* Frames to Second to stream to the Stream Handler   */
   ULONG ulFlags;          /* Frames or MicroSeconds */
} VCASETFPS;
typedef VCASETFPS FAR * PVCASETFPS;
/* Defines for ulFlags */
#define  VCASF_FRAMES       0x0
#define  VCASF_MICROSECONDS 0x1

/* IOCTL cat 140 code 7Ch - New func that supplies info about the display */
typedef struct VCASCREENINFO{      /* SCRINFO */
   ULONG  ulLength;          /* Length of this structure               */
   ULONG  ul_RESV01;         /* Reserved and set to zero               */
   ULONG  ulWidth;           /* Width of the Screen                    */
   ULONG  ulHeight;          /* Height of the Screen                   */
   ULONG  ulNumColours;      /* Number of Color in Screen's Palette    */
   ULONG  ulHFreq;           /* Horizontal Screen Frequency            */
   ULONG  ulVFreq;           /* Vertical   Screen Frequency            */
   ULONG  ulRGB[256];        /* Pallete Info if ulNumColors < 256      */
} VCASCREENINFO, FAR *PVCASCREENINFO;

/* IOCTL category 140 code 60h - Init Info from .INI file  Generic Format  */
typedef struct VCAINITG {      /* INI */
   ULONG ulBrightness;   /* 1=min, 255=max                    */
   ULONG ulHue;          /* 1=min, 255=max  or Tint           */
   ULONG ulSaturation;   /* 1=min, 255=max  or Color          */
   ULONG ulContrast;     /* 1=min, 255=max                    */
   BYTE  bD_Info[512];   /* Device Specific Info              */
} VCAINITG;
typedef VCAINITG FAR * PVCAINITG;

/* IOCTL category 140 code 60h - Init Info from .INI file     */
typedef struct VCAINIT {      /* IN */
   ULONG ulBrightness;   /* 1=min, 255=max                    */
   ULONG ulHue;          /* 1=min, 255=max  or Tint           */
   ULONG ulSaturation;   /* 1=min, 255=max  or Color          */
   ULONG ulContrast;     /* 1=min, 255=max                    */
   BYTE  bControl ;      /* see ulControl Defines             */
   BYTE  bCapPos  ;      /* see ulCappos  Defines             */
} VCAINIT;
typedef VCAINIT FAR * PVCAINIT;


                               /* KLL New Image Restore Functions - Start */
/**************************************************************************/
/* IOCTL category 140 code 63h - Query/Load/Unload MicroCode              */
typedef struct VCALOAD {     /* LOAD */
   ULONG  ulflags;           /* 1=Query,2=Load,3=Unload                   */
   CHAR   ProdInfo[256];     /* Path and Name of MicroCode Load           */
   ULONG  ulLoadID;          /* Load ID (returend on a load)              */
   ULONG  ulLength;          /* Lenght of load data                       */
   PVOID  pLoadData;         /* Pointer to MicroCode Load Data            */
} VCALOAD;
typedef VCALOAD FAR * PVCALOAD;

/*  ulFlags for VCALOAD                                                */
#define  VCALOAD_QUERY          0x01
#define  VCALOAD_LOAD           0x02
#define  VCALOAD_UNLOAD         0x03

/**************************************************************************/
/* IOCTL category 140 code 64h  - Query/Set Image Restore Format           */
typedef struct VCAIMAGERF {      /* IRF */
   ULONG ulFlags;        /* 0 = Query, 1 = Set                            */
   ULONG ulNumFormats;   /* Number of supported format(s)                 */
   ULONG ulCurIndex;     /* Current or Format to set to                   */
   ULONG FourCC[64];     /* Name for Format                               */
} VCAIMAGERF;
typedef VCAIMAGERF FAR * PVCAIMAGERF;

/*  ulFlags for VCAIMAGERF (Image Restore Format)                         */
#define  VCAIRF_Query           0x00
#define  VCAIRF_Set             0x01

/**************************************************************************/
/* IOCTL category 140 code 65h - Query/Set Image Capture Format           */
typedef struct VCAIMAGECF {      /* ICF */
   ULONG ulFlags;        /* 0 = Query, 1 = Set                            */
   ULONG ulNumFormats;   /* Number of supported format(s)                 */
   ULONG ulCurIndex;     /* Current or Format to set to                   */
   ULONG FourCC[64];     /* Name for Format                               */
} VCAIMAGECF;
typedef VCAIMAGECF FAR * PVCAIMAGECF;

/*  ulFlags for VCAIMAGECF (Image Capture Format)                         */
#define  VCAICF_Query           0x00
#define  VCAICF_Set             0x01


/**************************************************************************/
/* IOCTL category 140 code 66h - Restore Image data to the device         */
typedef struct VCAIMAGER {      /* IR */
   ULONG ulLength;       /* Lenght of Image Data                          */
   PVOID pImageData;     /* Pointer to Image Data                         */
} VCAIMAGER;
typedef VCAIMAGER FAR * PVCAIMAGER;

/**************************************************************************/
/* IOCTL category 140 code 67h - Restore Image data to the device         */
typedef struct VCASTREAM {      /* STRM */
   ULONG ulLength;       /* Lenght of Image Data                          */
   PVOID pImageData;     /* Pointer to Image Data                         */
   ULONG ulFlags;        /* Flag info                                     */
   ULONG ulSCR;          /* Video Stream Clock                            */
   ULONG ulPTS;          /* Presentation Time Stamp                       */
   ULONG ulAudioTime;    /* Current Audio Stream Time                     */
} VCASTREAM;
typedef VCASTREAM FAR * PVCASTREAM;

/* ulFlag    Defines  for VCASTREAM */
#define  VCA_PLAY_START         0x01
#define  VCA_PLAY_DATA          0x02
#define  VCA_PLAY_STOP          0x04
#define  VCA_PLAY_FLUSH         0x08
#define  VCA_PLAY_PAUSE         0x10
                               /* KLL New Image Restore Functions - End   */

/* ulControl Defines */
/*  For More Info Refer to the Video Capture Adapter/A Reference manual  */
/*  Register 14 Control Register                                         */
#define  VCAC_YC_DECODE         0x80
#define  VCAC_FORMAT_565        0x40
#define  VCAC_CURSOR_ENABLE     0x20
#define  VCAC_DISPLAY_ENABLE    0x10
#define  VCAC_GEN_LOCK          0x08
#define  VCAC_OUTOUT_SYNC_GREEN 0x04
#define  VCAC_NTSC_or_YC        0x00
#define  VCAC_PAL_or_YC         0x00
#define  VCAC_RGB               0x01

/* ulCappos Defines */
/*  NI = Non-Interlaced */
/*  I  = Interlaced     */
/*  The First 3 bit determine what type of signal to decode              */
/*  The Last  5 bit determine synchronization lock position  0=rightmost */
/*  For More Info Refer to the Video Capture Adapter/A Reference manual  */
/*  Register 12 Screen\Capture Position Controls                         */
#define  VCACP_NI262_field_1      0x80
#define  VCACP_NI262_field_2      0xA0
#define  VCACP_NI263_field_1      0xC0
#define  VCACP_NI263_field_2      0xE0
#define  VCACP_I262_5_field_both  0x00
#define  VCACP_I262_5_field_2     0x20
#define  VCACP_I262_5_field_1     0x40
#define  VCACP_I262_5_field_2B    0x60
#define  VCACP_Default_Pos        0x0C


/*********************** ********************** ****************************/
/*********************** Overlay Card Functions ****************************/
/*********************** ********************** ****************************/

/* IOCTL category 140 code 80h - Enable/Disable Monitor       */
typedef struct VCASETMONITOR {   /* EDM */
   BOOL  bMonitor;               /* 1=TRUE=ON, 0=FALSE=OFF    */
} VCASETMONITOR;
typedef VCASETMONITOR FAR * PVCASETMONITOR;

/* IOCTL category 140 code 81h - Enable/Disable Transparrent Color  */
typedef struct VCAEDCOLORKEY {   /* EDCK */
   BOOL  bColorKeying;           /* 1=TRUE=ON, 0=FALSE=OFF          */
} VCAEDCOLORKEY;
typedef VCAEDCOLORKEY FAR * PVCAEDCOLORKEY;

/* IOCTL category 140 code 82h - Set Color Key / Transparrent Color */
typedef struct VCASETCOLORKEY {   /* SCK */
   ULONG  ulColorKey;             /* Transparrent Color             */
} VCASETCOLORKEY;
typedef VCASETCOLORKEY FAR * PVCASETCOLORKEY;

/* IOCTL category 140 code 83h - Set Destination Clip List */
/* Specifies the Visible regions of the Destination Rectangle.        */
/* This Command is only issued by the MCD/VSD if Overlay in the       */
/* VCADEVINFO structure is set to 3.                                  */
/* Note: The number of valid regions will be set to zero when windows */
/* are moving. At this point the driver should suspend all blits to   */
/* its window until this command is reissued with at list one visible */
/* region.                                                            */
/* When windows change the command sequence will be a follows:        */
/*  1) One (or more) VCASETCLIPLIST will ulNum_Rects = 0              */
/*  2) VSD_SETVIDEORECT with the new Source/Destination Rectangles.   */
/*     This command may not be issued if the devices window is        */
/*     unchanged.                                                     */
/*  3) VSD_SETCLIPLIST with 1 or more valid regions                   */
typedef struct VCASETCLIPLISTT{      /* CL */
   ULONG     ulLength;      /* length of the structure             */
   ULONG     ulFlags;       /* Reserved and set to Zero            */
   ULONG     ulNum_Rects;   /* Number of rectanlges in array below */
   CRECT     VisRect[256];  /* Array of n Visible Retangle         */
} VCASETCLIPLIST;
typedef VCASETCLIPLIST FAR * PVCASETCLIPLIST;

/* IOCTL category 140 code 84h - Set Data Type for MPEG */
/* Specifies the Data type as MPEG1 or MPEG2                          */
#define VSD_DATATYPE_MPEG1          15 /* The other XA mode is an audio mode  */
#define VSD_DATATYPE_MPEG2          16 /* The other XA mode is an audio mode  */
typedef struct VCASETDATATYPE{      /* CL */
   ULONG     ulLength;      /* length of the structure                 */
   ULONG     ulFlags;       /* Type of data given - used for expansion */
   ULONG     ulDataType;    /* Data Type of stream                     */
} VCASETDATATYPE;
typedef VCASETDATATYPE FAR * PVCASETDATATYPE;

                                          /* KLL New Tuner Functions - Start */
/* IOCTL category 140 code 68h - Query Video Input Connector's Signal */
/* no structure it just Returns a status */
    /* VCAERR_SIGNAL_LOCKED        10    */
    /* VCAERR_SIGNAL_NOT_LOCKED    11    */
    /* VCAERR_SIGNAL_INDETERMINATE 12    */

/* IOCTL category 140 code 69h - Set/Query Tuner Channel     */
typedef struct VCATUNCHAN {      /* TUC */
   ULONG  ulFlags;        /* 1=Set, 2=Query                      */
   ULONG  ulOptions;      /* Options                             */
   USHORT usResv01;       /* Reserved                            */
   USHORT usResv02;       /* Reserved                            */
   LONG   lFineTune;      /* Fine Tune value                     */
   ULONG  ulFrequency;    /* Frequency                           */
} VCATUNCHAN;
typedef VCATUNCHAN FAR * PVCATUNCHAN;

/* Values for ulFlags on Set/Query Channel */
#define TUC_SET_CHANNEL   1
#define TUC_QUERY_CHANNEL 2

/* Values for ulOptions on Set/Query Channel */
/* Bit sensitive field                       */
#define TUC_AFC_ON             4
#define TUC_AFC_OFF            0
#define TUC_FREQUENCY          8
#define TUC_POLARIZATION_VERT 16
#define TUC_POLARIZATION_HORI 32


/* Specific Status Returned by Set/Query Channel */
    /* VCAERR_CHANNEL_TOO_LOW       6    */
    /* VCAERR_CHANNEL_TOO_HIGH      7    */
    /* VCAERR_CHANNEL_SKIP          8    */
    /* VCAERR_CHANNEL_NO_TUNER      9    */

                                       /* KLL Volume Functions - Start */
/* IOCTL category 140 code 79h - Set/Query  Audio Functions, this is   */
/*                               a General Device Specific command     */
typedef struct VCADEVAUDIO{      /* DEVAUD */
   ULONG  ulLENGTH;          /* Length of this structure               */
   ULONG  ulFUNCTION;        /* Audio Input Channel                    */
   ULONG  ulFLAGS;           /* Flags                                  */
   ULONG  ul_RESV01;         /* Reserved and set to zero               */
   ULONG  ulL_VOLUME;        /* Left  Volume                           */
   ULONG  ulR_VOLUME;        /* Right Volume                           */
   ULONG  ulL_BASS;          /* Left  Volume                           */
   ULONG  ulR_BASS;          /* Right Volume                           */
   ULONG  ulL_TREBLE;        /* Left  Volume                           */
   ULONG  ulR_TREBLE;        /* Right Volume                           */
} VCADEVAUDIO;

typedef VCADEVAUDIO FAR * PVCADEVAUDIO;

/* Defines for ulFunction on VCADEVAUDIO IOCTL */
//                           A U D 1
#define VCADEV_AUDIO_FUNC 0x41554431

/* Defines for ulFlags on VCADEVAUDIO IOCTL */
#define VCADEV_AUDIO_FLAG_MUTE   1
#define VCADEV_AUDIO_FLAG_UNMUTE 0

/* Defines for ulL_xxx and ulR_xxx on VCADEVAUDIO IOCTL         */
/*  -1=no change, -2=reset default, -1 on QUERY = not supported */
#define VCA_QUERY_CURRENT  0xFFFFFFFF
#define VCA_SET_TO_DEFAULT 0xFFFFFFFE
#define VCA_NOT_SUPPORTED  0xFFFFFFFF
                                       /* New Volume Functions - End   */

                    //            E S C
#define  VCAI_USER_CODE        (0x45534300L)
#define  VCAI_GET_ADVANCED     (0x01 | VCAI_USER_CODE)
#define  VCAI_SET_ADVANCED     (0x02 | VCAI_USER_CODE)
#define  VCAI_GET_SL           (0x03 | VCAI_USER_CODE)
#define  VCAI_SET_SL           (0x04 | VCAI_USER_CODE)
#define  VCAI_GET_CONF         (0x05 | VCAI_USER_CODE)
#define  VCAI_SET_CONF         (0x06 | VCAI_USER_CODE)
#define  VCAI_GET_INFO         (0x07 | VCAI_USER_CODE)
#define  VCAI_I2CIO            (0x08 | VCAI_USER_CODE)
#define  VCAI_RESET            (0x09 | VCAI_USER_CODE)
#define  VCAI_HARDWARE         (0x0A | VCAI_USER_CODE)
#define  VCAI_SET_CK           (0x0B | VCAI_USER_CODE)
#define  VCAI_GET_CK           (0x0C | VCAI_USER_CODE)
#define  VCAI_SET_TUNER        (0x0D | VCAI_USER_CODE)
#define  VCAI_GET_TUNER        (0x0E | VCAI_USER_CODE)
#define  VCAI_VARIABLE         (0x0F | VCAI_USER_CODE)
#define  VCAI_GET_PALETTE      (0x10 | VCAI_USER_CODE)
#define  VCAI_REDRAW           (0x11 | VCAI_USER_CODE)
#define  VCAI_SNAP_START       (0x12 | VCAI_USER_CODE)
#define  VCAI_SNAP_GET         (0x13 | VCAI_USER_CODE)
#define  VCAI_SNAP_STOP        (0x14 | VCAI_USER_CODE)
#define  VCAI_PLAY_START       (0x15 | VCAI_USER_CODE)
#define  VCAI_PLAY_SET         (0x16 | VCAI_USER_CODE)
#define  VCAI_PLAY_STOP        (0x17 | VCAI_USER_CODE)

/* Added in support of the Hauppauge WinCast board.                         */
#define  VCAI_WCAST_MONITOR     (0x18 | VCAI_USER_CODE)
#define  VCAI_CLIP_LIST         (0x19 | VCAI_USER_CODE)
// #define  VCAI_WCAST_POS         (0x19 | VCAI_USER_CODE)
// #define  VCAI_SET_HWND          (0x1A | VCAI_USER_CODE)
// #define  VCAI_GET_HWND          (0x1B | VCAI_USER_CODE)

#define  VCAI_DMA_START         (0x1C | VCAI_USER_CODE)

#define  VCAI_GRADD_MODEINFO    (0x20 | VCAI_USER_CODE)
#define  VCAI_GRADD_MOVEPTR     (0x21 | VCAI_USER_CODE)
#define  VCAI_GRADD_SETPTR      (0x22 | VCAI_USER_CODE)
#define  VCAI_GRADD_SHOWPTR     (0x23 | VCAI_USER_CODE)

#define  VCAI_FIRST_FUNCTION    (VCAI_GET_ADVANCED)
#define  VCAI_LAST_FUNCTION     (VCAI_GRADD_SHOWPTR)

typedef struct VCAUSER{      /* Generic general purpose */
   ULONG  user_length;       /* Length of this structure               */
   ULONG  user_function;     /* Function                               */
} VCAUSER, FAR *PVCAUSER;;
