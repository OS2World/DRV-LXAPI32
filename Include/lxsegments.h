/* $Id: lxsegments.h,v 1.2 2006/04/06 21:17:41 smilcke Exp $ */

/*
 * lxsegments.h
 * Autor:               Stefan Milcke
 * Erstellt am:         23.02.2005
 * Letzte Aenderung am: 04.04.2006
 *
*/

#ifndef LXSEGMENTS_H_INCLUDED
#define LXSEGMENTS_H_INCLUDED

struct lx_segment
{
 unsigned long type;             // type of segment, see GDTSEL_R* constants
 unsigned long size;             // size of segment
 void* pVirtual;                 // virtual address
 void* pPhysical;                // physical address
 void* pMapped;                  // mapped address
 unsigned long numSels;          // number of allocated selectors
 unsigned short *sels;           // selectors for mapping
};

extern struct lx_segment* LX_alloc_segment(unsigned long type
                                           ,unsigned long size
                                           ,unsigned long virt_addr
                                           ,struct lx_segment* segptr);
extern unsigned long LX_free_segment(struct lx_segment* segment);

#endif //LXSEGMENTS_H_INCLUDED
