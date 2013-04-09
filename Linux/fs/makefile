# $Id: makefile,v 1.16 2006/01/05 23:48:23 smilcke Exp $

TARGET     = lxfs
#LXFORCEO   := 1

#==============================================================================
# Setup subdirectories
#==============================================================================
DIRS	:= ramfs sysfs

#===================================================================
#   List of source files
#===================================================================
SRCFIRST	:=
SRCFILES	:=	lf_aio \
			lf_attr \
			lf_bad_inode \
			lf_bio \
			lf_block_dev \
			lf_char_dev \
			lf_dcache \
			lf_direct-io \
			lf_dnotify \
			lf_fcntl \
			lf_fifo \
			lf_file_table \
			lf_file \
			lf_filesystems \
			lf_fs-writeback \
			lf_inode \
			lf_ioctl \
			lf_libfs \
			lf_locks \
			lf_mpage \
			lf_namei \
			lf_namespace \
			lf_open \
			lf_pipe \
			lf_read_write \
			lf_readdir \
			lf_select \
			lf_seq_file \
			lf_stat \
			lf_super \
			lf_xattr
SRCLAST		:=

EMXOMFAROPTS	:= -p32

include $(LXAPI32DEV)\makes\libmake.mak
