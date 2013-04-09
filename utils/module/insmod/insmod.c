/* $Id: insmod.c,v 1.1 2005/07/11 22:18:33 smilcke Exp $ */

/*
 * insmod.c
 * Autor:               Stefan Milcke
 * Erstellt am:         05.07.2005
 * Letzte Aenderung am: 07.07.2005
 *
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <lxlibc.h>

//-------------------------------- print_usage ---------------------------------
static void print_usage(const char* progname)
{
 exit(1);
}

//--------------------------------- *grab_file ---------------------------------
static void *grab_file(const char *filename, unsigned long *size)
{
   unsigned int max = 16384;
   int ret, fd;
   void *buffer = malloc(max);

   if (streq(filename, "-"))
      fd = dup(STDIN_FILENO);
   else
      fd = open(filename, O_RDONLY, 0);

   if (fd < 0)
      return NULL;

   *size = 0;
   while ((ret = read(fd, buffer + *size, max - *size)) > 0) {
      *size += ret;
      if (*size == max)
         buffer = realloc(buffer, max *= 2);
   }
   if (ret < 0) {
      free(buffer);
      buffer = NULL;
   }
   close(fd);
   return buffer;
}

//------------------------------------ main ------------------------------------
int main(int argc,char *argv[])
{
 int ret;
 unsigned long len;
 void* file;
 char* filename;
 char* progname=argv[0];
 char* options="";
 filename=argv[1];
 if(!filename)
  print_usage(progname);
 file=grab_file(filename,&len);
 if(!file)
 {
  exit(1);
 }
// DevInt3();
 ret=init_module(file,len,options);
 if(ret!=0)
 {
  exit(1);
 }
 exit(0);
 return 0;
}
