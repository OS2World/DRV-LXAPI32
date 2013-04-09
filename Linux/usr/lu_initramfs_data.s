/* $Id: lu_initramfs_data.s,v 1.1 2004/07/19 22:37:11 smilcke Exp $ */

/*
 * lu_initramfs_data.s
 * Autor:               Stefan Milcke
 * Erstellt am:         06.07.2004
 * Letzte Aenderung am: 06.07.2004
 *
*/

.data
.globl ___initramfs_start
.data
___initramfs_start:
.incbin "./bin/initramfs_data.cpio.gz"
.globl ___initramfs_end
___initramfs_end:
