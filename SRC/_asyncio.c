/*******************************************************************************
 *
 * (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * asyncio.c - See AmigaMail Vol.2 Section 2 P77 for more details. Note that
 * this has a few modifications of my own to reduce time spent copying memory
 * from the Asyncio routine's buffers to mine.
 *
 ******************************************************************************/


#include "uiki:uik_protos.h"
#include "uiki:uik_pragmas.h"

#include "lci:azur.h"

#include "lci:_asyncio.h"

extern struct Glob { ULONG toto; } __near GLOB;


/*****************************************************************************/


/* this macro lets us long-align structures on the stack */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
                       type *name = (type *)((LONG)(a_##name+3) & ~3);


/*****************************************************************************/


static void SendPacket( struct AsyncFile *file, APTR arg2 )
{
    file->af_Packet.sp_Pkt.dp_Port = &file->af_PacketPort;
    file->af_Packet.sp_Pkt.dp_Arg2 = (LONG)arg2;
    PutMsg( file->af_Handler, &file->af_Packet.sp_Msg );
    file->af_PacketPending = TRUE;
}


/*****************************************************************************/


static LONG WaitPacket( struct AsyncFile *file )
{
  LONG bytes;
  UWORD version = ((struct Library *)(*(ULONG*)4))->lib_Version;

    if (file->af_PacketPending)
        {
        while (TRUE)
            {
            /* This enables signalling when a packet comes back to the port */
            file->af_PacketPort.mp_Flags = PA_SIGNAL;

            /* Wait for the packet to come back, and remove it from the message
             * list. Since we know no other packets can come in to the port, we can
             * safely use Remove() instead of GetMsg(). If other packets could come in,
             * we would have to use GetMsg(), which correctly arbitrates access in such
             * a case
             */
            Remove( (struct Node *)WaitPort( &file->af_PacketPort ) );

            /* set the port type back to PA_IGNORE so we won't be bothered with
             * spurious signals
             */
            file->af_PacketPort.mp_Flags = PA_IGNORE;

            /* mark packet as no longer pending since we removed it */
            file->af_PacketPending = FALSE;

            bytes = file->af_Packet.sp_Pkt.dp_Res1;
            if (bytes >= 0)
                {
                /* packet didn't report an error, so bye... */
                return( bytes );
                }

            if (version >= 37)
                {
                /* see if the user wants to try again... */
                if (ErrorReport( file->af_Packet.sp_Pkt.dp_Res2, REPORT_STREAM, file->af_File, NULL ))
                    return(-1);

                /* user wants to try again, resend the packet */
                if (file->af_ReadMode)
                    SendPacket( file, file->af_Buffers[file->af_CurrentBuf] );
                else
                    SendPacket( file, file->af_Buffers[1 - file->af_CurrentBuf] );
                }
            }
        }

    if (version >= 37)
        {
        /* last packet's error code, or 0 if packet was never sent */
        SetIoErr(file->af_Packet.sp_Pkt.dp_Res2);
        }
    return( file->af_Packet.sp_Pkt.dp_Res1 );
}

/*
static VOID WaitPacket( struct AsyncFile *file )
{
    /* This enables signalling when a packet comes back to the port */
    file->af_PacketPort.mp_Flags = PA_SIGNAL;

    /* Wait for the packet to come back, and remove it from the message
     * list. Since we know no other packets can come in to the port, we can
     * safely use Remove() instead of GetMsg(). If other packets could come in,
     * we would have to use GetMsg(), which correctly arbitrates access in such
     * a case
     */
    Remove( (struct Node *)WaitPort( &file->af_PacketPort ) );

    /* set the port type back to PA_IGNORE so we won't be bothered with
     * spurious signals
     */
    file->af_PacketPort.mp_Flags = PA_IGNORE;

    /* packet is no longer pending, we got it */
    file->af_PacketPending = FALSE;
}
*/


/*****************************************************************************/


/* this function puts the packet back on the message list of our
 * message port.
 */
/*
static void RequeuePacket( struct AsyncFile *file )
{
    AddHead( &file->af_PacketPort.mp_MsgList, &file->af_Packet.sp_Msg.mn_Node );
    file->af_PacketPending = TRUE;
}
*/


/*****************************************************************************/


/* this function records a failure from a synchronous DOS call into the
 * packet so that it gets picked up by the other IO routines in this module
 */
/*
static void RecordSyncFailure( struct AsyncFile *file )
{
    file->af_Packet.sp_Pkt.dp_Res1 = -1;
    file->af_Packet.sp_Pkt.dp_Res2 = IoErr();
}
*/


/*****************************************************************************/

struct AsyncFile *OpenAsync( const STRPTR fileName, UBYTE accessMode, LONG bufferSize )
{
  struct AsyncFile *file;
  struct FileHandle *fh;
  BPTR handle, lock;
  ULONG blocksize;
  UWORD version = ((struct Library *)(*(ULONG*)4))->lib_Version;
  D_S(struct InfoData,infoData);

    handle = NULL;
    file   = NULL;
    lock   = NULL;

    if (accessMode == MODE_READ)
        {
        if (handle = Open( fileName, MODE_OLDFILE ))
            lock = Lock( fileName, ACCESS_READ );
        }
    else{
        if (accessMode == MODE_WRITE)
            {
            handle = Open(fileName,MODE_NEWFILE);
            }
        else if (accessMode == MODE_APPEND)
            {
            if (handle = Open( fileName, (version >= 37 ? MODE_READWRITE : MODE_OLDFILE) ))
                {
                if (Seek( handle, 0, OFFSET_END ) < 0)
                    {
                    Close( handle );
                    handle = NULL;
                    }
                }
            }

        /* we want a lock on the same device as where the file is. We can't
         * use DupLockFromFH() for a write-mode file though. So we get sneaky
         * and get a lock on the parent of the file
         */
        if (handle)
            if (version >= 37) lock = ParentOfFH( handle );
        }

    if (handle)
        {
        /* if it was possible to obtain a lock on the same device as the
         * file we're working on, get the block size of that device and
         * round up our buffer size to be a multiple of the block size.
         * This maximizes DMA efficiency.
         */

        blocksize = 0;
        if (lock)
            {
            if (version >= 37 && Info( lock, infoData ))
                {
                blocksize = infoData->id_BytesPerBlock;
                bufferSize = (((bufferSize + blocksize - 1) / blocksize) * blocksize) * 2;
                }
            UnLock( lock );
            }
        if (blocksize == 0)
            {
            blocksize = 512;
            bufferSize = (((bufferSize + blocksize - 1) / blocksize) * blocksize) * 2;
            }

        /* now allocate the ASyncFile structure, as well as the read buffers.
         * Add 15 bytes to the total size in order to allow for later
         * quad-longword alignement of the buffers
         */

        if (! (file = (APTR) BufAllocP( AGP.Pool, sizeof(struct AsyncFile) + bufferSize + 15, 4 )))
            {
            bufferSize = 2 * 4096;
            file = (APTR) BufAllocP( AGP.Pool, sizeof(struct AsyncFile) + bufferSize + 15, 4 );
            }

        if (file)
            {
            file->af_File      = handle;
            file->af_ReadMode  = (accessMode == MODE_READ);
            file->af_BlockSize = (ULONG) blocksize;

            /* initialize the ASyncFile structure. We do as much as we can here,
             * in order to avoid doing it in more critical sections
             *
             * Note how the two buffers used are quad-longword aligned. This
             * helps performance on 68040 systems with copyback cache. Aligning
             * the data avoids a nasty side-effect of the 040 caches on DMA.
             * Not aligning the data causes the device driver to have to do
             * some magic to avoid the cache problem. This magic will generally
             * involve flushing the CPU caches. This is very costly on an 040.
             * Aligning things avoids the need for magic, at the cost of at
             * most 15 bytes of ram.
             */

            fh                     = BADDR(file->af_File);
            file->af_Handler       = fh->fh_Type;
            file->af_BufferSize    = bufferSize / 2;
            file->af_Buffers[0]    = (APTR)(((ULONG)file + sizeof(struct AsyncFile) + 15) & 0xfffffff0);
            file->af_Buffers[1]    = (APTR)((ULONG)file->af_Buffers[0] + file->af_BufferSize);
            file->af_Offset        = file->af_Buffers[0];
            file->af_CurrentBuf    = 0;
            file->af_SeekOffset    = 0;
            file->af_PacketPending = FALSE;

            /* this is the port used to get the packets we send out back.
             * It is initialized to PA_IGNORE, which means that no signal is
             * generated when a message comes in to the port. The signal bit
             * number is initialized to SIGB_SINGLE, which is the special bit
             * that can be used for one-shot signalling. The signal will never
             * be set, since the port is of type PA_IGNORE. We'll change the
             * type of the port later on to PA_SIGNAL whenever we need to wait
             * for a message to come in.
             *
             * The trick used here avoids the need to allocate an extra signal
             * bit for the port. It is quite efficient.
             */

            file->af_PacketPort.mp_MsgList.lh_Head     = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Tail;
            file->af_PacketPort.mp_MsgList.lh_Tail     = NULL;
            file->af_PacketPort.mp_MsgList.lh_TailPred = (struct Node *)&file->af_PacketPort.mp_MsgList.lh_Head;
            file->af_PacketPort.mp_Node.ln_Type        = NT_MSGPORT;
            file->af_PacketPort.mp_Flags               = PA_IGNORE;
            file->af_PacketPort.mp_SigBit              = SIGB_SINGLE;
            file->af_PacketPort.mp_SigTask             = FindTask(NULL);

            file->af_Packet.sp_Pkt.dp_Link          = &file->af_Packet.sp_Msg;
            file->af_Packet.sp_Pkt.dp_Arg1          = fh->fh_Arg1;
            file->af_Packet.sp_Pkt.dp_Arg3          = file->af_BufferSize;
            file->af_Packet.sp_Pkt.dp_Res1          = 0;
            file->af_Packet.sp_Pkt.dp_Res2          = 0;
            file->af_Packet.sp_Msg.mn_Node.ln_Name  = (STRPTR)&file->af_Packet.sp_Pkt;
            file->af_Packet.sp_Msg.mn_Node.ln_Type  = NT_MESSAGE;
            file->af_Packet.sp_Msg.mn_Length        = sizeof(struct StandardPacket);

            if (accessMode == MODE_READ)
                {
                /* if we are in read mode, send out the first read packet to
                 * the file system. While the application is getting ready to
                 * read data, the file system will happily fill in this buffer
                 * with DMA transfers, so that by the time the application
                 * needs the data, it will be in the buffer waiting
                 */

                file->af_Packet.sp_Pkt.dp_Type = ACTION_READ;
                file->af_BytesLeft             = 0;
                if (file->af_Handler)
                    SendPacket( file, file->af_Buffers[0] );
                }
            else{
                file->af_Packet.sp_Pkt.dp_Type = ACTION_WRITE;
                file->af_BytesLeft             = file->af_BufferSize;
                }
            }
        else{
            Close( handle );
            }
        }

    return( file );
}


/*****************************************************************************/


LONG CloseAsync( struct AsyncFile *file )
{
  LONG result;
  UWORD version = ((struct Library *)(*(ULONG*)4))->lib_Version;

    if (file)
        {
        result = WaitPacket( file );
        if (result >= 0)
            {
            if (! file->af_ReadMode)
                {
                /* this will flush out any pending data in the write buffer */
                if (file->af_BufferSize > file->af_BytesLeft)
                    result = Write( file->af_File, file->af_Buffers[file->af_CurrentBuf], file->af_BufferSize - file->af_BytesLeft );
                }
            }

        Close( file->af_File );
        BufFree( (UBYTE*)file );
        }
    else{
        if (version >= 37) SetIoErr( ERROR_INVALID_LOCK );
        result = -1;
        }

    return( result );
}


/*****************************************************************************/


LONG ReadAsync( struct AsyncFile *file, UBYTE **readybuffer )
{
  LONG bytesArrived;
  UWORD version = ((struct Library *)(*(ULONG*)4))->lib_Version;

    if (! file->af_PacketPending) return(0);

    bytesArrived = WaitPacket( file );
    if (bytesArrived <= 0) /* error, get out of here */
        {
        if (version >= 37) SetIoErr( file->af_Packet.sp_Pkt.dp_Res2 );
        return( bytesArrived );
        }

    *readybuffer = (APTR) file->af_Buffers[file->af_CurrentBuf];
    file->af_CurrentBuf = 1 - file->af_CurrentBuf;

    SendPacket( file, file->af_Buffers[file->af_CurrentBuf] ); /* ask that the buffer be filled */

    return( bytesArrived );
}


/*****************************************************************************/

LONG WriteAsync( struct AsyncFile *file, APTR buffer, LONG numBytes )
{
  LONG totalBytes;

    totalBytes = 0;

    while (numBytes > file->af_BytesLeft)
        {
        if (! file->af_Handler) /* this takes care of NIL: */
            {
            file->af_Offset    = file->af_Buffers[0];
            file->af_BytesLeft = file->af_BufferSize;
            return( numBytes );
            }

        if (file->af_BytesLeft)
            {
            CopyMem( buffer, file->af_Offset, file->af_BytesLeft );

            numBytes   -= file->af_BytesLeft;
            buffer      = (APTR)((ULONG)buffer + file->af_BytesLeft);
            totalBytes += file->af_BytesLeft;
            }

        if (WaitPacket( file ) < 0)
            return(-1);

        SendPacket( file, file->af_Buffers[file->af_CurrentBuf] ); /* send the current buffer out to disk */

        file->af_CurrentBuf   = 1 - file->af_CurrentBuf;
        file->af_Offset       = file->af_Buffers[file->af_CurrentBuf];
        file->af_BytesLeft    = file->af_BufferSize;
        }

    if (numBytes)
        {
        CopyMem( buffer, file->af_Offset, numBytes );
        file->af_BytesLeft -= numBytes;
        file->af_Offset     = (APTR)((ULONG)file->af_Offset + numBytes);
        }

    return( totalBytes + numBytes );
}
