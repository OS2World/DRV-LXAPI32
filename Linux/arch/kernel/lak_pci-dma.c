/* $Id: lak_pci-dma.c,v 1.2 2004/07/20 21:51:49 smilcke Exp $ */

/*
 * lak_pci-dma.c
 * Autor:               Stefan Milcke
 * Erstellt am:         23.06.2004
 * Letzte Aenderung am: 23.06.2004
 *
*/

#include <lxcommon.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <asm/io.h>

/*
 * Dynamic DMA mapping support.
 *
 * On i386 there is no hardware dynamic DMA address translation,
 * so consistent alloc/free are merely page allocation/freeing.
 * The rest of the dynamic DMA mapping interface is implemented
 * in asm/pci.h.
 */
void *dma_alloc_coherent(struct device *dev, size_t size,
            dma_addr_t *dma_handle, int gfp)
{
   void *ret;
   /* ignore region specifiers */
   gfp &= ~(__GFP_DMA | __GFP_HIGHMEM);

   if (dev == NULL || (dev->coherent_dma_mask < 0xffffffff))
      gfp |= GFP_DMA;

   ret = (void *)__get_free_pages(gfp, get_order(size));

   if (ret != NULL) {
      memset(ret, 0, size);
      *dma_handle = virt_to_phys(ret);
   }
   return ret;
}

void dma_free_coherent(struct device *dev, size_t size,
          void *vaddr, dma_addr_t dma_handle)
{
   free_pages((unsigned long)vaddr, get_order(size));
}
