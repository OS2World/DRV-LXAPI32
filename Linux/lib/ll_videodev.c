/* $Id: ll_videodev.c,v 1.1 2004/07/19 22:36:59 smilcke Exp $ */

/*
 * videodev.c
 * Autor:               Stefan Milcke
 * Erstellt am:         15.11.2001
 * Letzte Aenderung am: 07.04.2004
 *
*/

#include <lxcommon.h>

#include <linux/errno.h>

//-------------------------- LX_v4lx_get_num_devices ---------------------------
unsigned long LX_v4lx_get_num_devices(void)
{
 unsigned long c=0;
 return c;
}

//---------------------------- LX_v4lx_enum_devices ----------------------------
int LX_v4lx_enum_devices(void *buffer,unsigned long buffer_len)
{
 int c=0;
 return c;
}

//------------------------- LX_v4lx_get_opened_device --------------------------
struct video_device* LX_v4lx_get_opened_device(int handle)
{
 return NULL;
}

//---------------------------- LX_v4lx_open_device -----------------------------
int LX_v4lx_open_device(char *devname,unsigned short fileId,unsigned long free_flags)
{
 return -ENXIO;
}

//--------------------- LX_v4lx_close_file_related_devices ---------------------
void LX_v4lx_close_file_related_devices(unsigned short fileId)
{
}

//---------------------------- LX_v4lx_close_device ----------------------------
int LX_v4lx_close_device(int handle)
{
 return -ENXIO;
}

//------------------------------- LX_v4lx_ioctl --------------------------------
int LX_v4lx_ioctl(int handle,int ioctlfn,void *userData)
{
 int rc=0;
 return rc;
}

//-------------------------------- LX_v4lx_read --------------------------------
int LX_v4lx_read(int handle,void* buffer,int count,int nonblock)
{
 int rc=0;
 return rc;
}
