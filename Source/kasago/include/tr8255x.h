/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: Intel 8255X Device Driver
 * Filename: tr8255x.c
 * Author: Jason
 * Date Created: 3/14/2000
 * $Source: include/tr8255x.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:15JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * This following macros might need to be modified for your particular 
 * platform (see above).
 */

/* 
 * Use I/O port commands to communicate with the 8255x.  Undefine this to use
 * memory mapped ports.
 */
#ifndef TM_XSCALE_IQ80310
#define TM_USE_IO
#endif /* TM_XSCALE_IQ80310 */

/* Size of the 8255x transmit ring (number of packets) */
#define TM_8255X_CMD_RING_SIZE  10

/* Size of the 8255x receive ring (number of packets) */
#define TM_8255X_RECV_RING_SIZE 10

/* Maximum number of scattered buffers per packet */
#define TM_8255X_MAX_SCATTER 5

#define TM_8255X_XMIT_THRESHOLD 256 


/* Translates a virtual (CPU) to a physical (bus) address */
#define tm_virt_addr_to_phys(addr) addr

#ifdef TM_XSCALE_IQ80310
/* Invalidates the CPU data cache */
#define tm_asm_invd
/* This isn't necessary yet, as the data cache is not yet enabled on the
   IQ80310 (XScale) target.  But when it is, the macro will look something
   like... */
/*
#define tm_asm_invd __asm__("\
  eor r0,r0,r0      ; \
  mcr p15,0,r0,C7,C7,0 ; \
  mrc p15,0,r0,C2,C0,0 ; \
  mov r0,r0 ; \
  sub pc,pc,#4; ")
*/
#else /* !TM_XSCALE_80310 */

/* Invalidate the date cache on the x86 target */
#define tm_asm_invd __asm { invd }

#endif /* TM_XSCALE_80310 */

/*
 * Define this macro if data needs to be byte swapped between the processor
 * and the Ethernet controller; for instance, when using a big endian processor
 * such as PowerPC.  Byte swapping is not needed if the processor is little
 * endian (this macro should not be defined if being used with a x86).
 */
/* #define TM_SWAP_NEEDED */

/* 
 * The tm_clear_cpu_interrupts macro should acknowledge interrupts on the CPU 
 * (or external interrupt controller).  This example macro clears interrupts on 
 * the 8259 PIC, present on most typical desktop PC's.
 */
#define TM_PIC1_ISR_ADDR 0x20
#define TM_PIC1_IMR_ADDR 0x21
#define TM_PIC2_ISR_ADDR 0xA0
#define TM_PIC2_IMR_ADDR 0xA1
#define TM_PIC_CLEAR_INT 0x20

#ifdef TM_XSCALE_IQ80310

/* Not needed with the IQ80310 XScale target */
#define tm_clear_cpu_interrupts

#else /* !TM_XSCALE_IQ80310 */
#define tm_clear_cpu_interrupts \
if ( tlInfoPtr->i8255xIntLine < 8 ) \
{ \
    _outp( TM_PIC1_ISR_ADDR, TM_PIC_CLEAR_INT ); \
} \
else \
{ \
    _outp( TM_PIC2_ISR_ADDR, TM_PIC_CLEAR_INT ); \
    _outp( TM_PIC1_ISR_ADDR, TM_PIC_CLEAR_INT ); \
} 
#endif /* TM_XSCALE_IQ80310 */

#ifdef TM_XSCALE_IQ80310
/* 
 * PCI ID for Intel (vendor) and the 8255x (device).  Normally, the vendor ID
 * is 0x8086 and the device ID is 0x1229.  However, these may be overridden by
 * the EEPROM, and may need to be set here.
 */
#define TM_PCI_ID_INTEL       0x8086
#define TM_PCI_ID_INTEL_82559 0x1209
#else /* TM_XSCALE_IQ80310 */
#define TM_PCI_ID_INTEL       0x8086
#define TM_PCI_ID_INTEL_82559 0x1229
#endif /* TM_XSCALE_IQ80310 */


#define TM_8255X_IOCTL_REFILL        0x0001
#define TM_8255X_IOCTL_SEND_COMPLETE 0x0002

/*
 * Driver API prototypes
 */
int tf8255xOpen(ttUserInterface interfaceHandle);
int tf8255xClose(ttUserInterface interfaceHandle);
int tf8255xIoctl(ttUserInterface interfaceHandle,
                 int             flag,
                 void TM_FAR *   optionPtr,
                 int             optionLen);
int tf8255xSend(ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,
                int             dataLength,
                int             flag);
int tf8255xReceive(ttUserInterface        interfaceHandle,
                   char TM_FAR * TM_FAR * dataPtr,
                   int  TM_FAR *          dataLength,
                   ttUserBufferPtr        bufHandlePtr);

int tf8255xGetPhysAddr(ttUserInterface interfaceHandle,
                       char TM_FAR *   physicalAddress);
#ifdef TM_SWAP_NEEDED
static unsigned short tf8255xInWord(int offset);
static unsigned long tf8255xInDword(int offset);
#endif /* TM_SWAP_NEEDED */


#ifdef TM_USE_IO
/*
 * These macros are used to read & write registers on the 8255x.  This may be
 * done by using either I/O ports or memory-mapped regisers, depending on if
 * TM_USE_IO is defined (see above).  The I/O port command will probably need
 * to be changed to match your compiler's convention.  More than likely, the
 * memory-mapped macros will not need to be changed.
 *
 * NOTE: If data needs to be byte swapped between the processor and the 
 *       Ethernet controller (i.e., TM_SWAP_NEEDED is defined above) you
 *       will also need to modify the tf8255xInWord() and tf8255xInDword()
 *       functions at the end of the tr8255x.c source file.
 */

/* Writes a byte to the 8255x I/O ports */
#define tm_out_byte( value, offset ) \
    _outp( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset), \
            value )

/* Writes a word to the 8255x I/O ports */
#define tm_out_word(value, offset) \
    _outpw( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset), \
            tm_flip_word(value) )

/* Writes a double word to the 8255x I/O ports */
#define tm_out_dword(value, offset) \
    _outpd( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset), \
                        tm_flip_dword(value) )

/* Reads a byte from the 8255x I/O ports */
#define tm_in_byte(offset) \
    _inp( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset) )

/* Reads a word from the 8255x I/O ports */
#define tm_in_word(offset) \
    _inpw( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset) )

/* Reads a double word from the 8255x I/O ports */
#define tm_in_dword(offset) \
    _inpd( (unsigned short) (tlInfoPtr->i8255xIoAddr + offset) )

#else /* TM_USE_IO */

/* Writes a byte to the 8255x memory-mapped registers */
#define tm_out_byte(value, offset) \
    *((unsigned char *) (tlInfoPtr->i8255xMemAddrPtr+offset)) = value

/* Writes a word to the 8255x memory-mapped registers */
#define tm_out_word(value, offset) \
    *((unsigned short *) (tlInfoPtr->i8255xMemAddrPtr+offset)) = \
        tm_flip_word(value)

/* Writes a double word to the 8255x memory-mapped registers */
#define tm_out_dword(value, offset) \
    *((unsigned long *) (tlInfoPtr->i8255xMemAddrPtr+offset)) = \
        tm_flip_dword(value)

/* Reads a byte from the 8255x memory-mapped registers */
#define tm_in_byte(offset) \
    *((unsigned char *) (tlInfoPtr->i8255xMemAddrPtr+offset))

/* Reads a word from the 8255x memory-mapped registers */
#define tm_in_word(offset) \
    *((unsigned short *) (tlInfoPtr->i8255xMemAddrPtr+offset))

/* Reads a double word from the 8255x memory-mapped registers */
#define tm_in_dword(offset) \
    *((unsigned long *) (tlInfoPtr->i8255xMemAddrPtr+offset))

#endif /* !TM_USE_IO */

#ifdef TM_SWAP_NEEDED 
#define tm_flip_word(x) ( (((x)&0xFF00)>>8 )  \
                       |  (((x)&0x00FF)<<8)  \
                      )

#define tm_flip_dword(x) ( (((x)&0xFF000000)>>24)  \
                        |  (((x)&0x00FF0000)>>8)  \
                        |  (((x)&0x0000FF00)<<8)  \
                        |  (((x)&0x000000FF)<<24) \
                       )
#undef tm_in_word
#undef tm_in_dword
#define tm_in_word(offset) tf8255xInWord(offset)
#define tm_in_dword(offset) tf8255xInDword(offset)  
#else /* !TM_SWAP_NEEDED */
#define tm_flip_word(x) x
#define tm_flip_dword(x) x
#endif /* TM_SWAP_NEEDED */


/*
 * The following macros are internal to the driver and shouldn't need to be
 * changed for most platforms.
 */

/*
 * Macros that set/un-set the 'suspend' and 'complete' bits in an RBD.  Note
 * that it is important to only modify the relevant byte in the status long
 * word.  This is especially important on systems where long word writes may
 * not be atomic (such as the PowerPC).
 */ 

/* Suspend the receiver. */
#define tm_8255x_suspend(rbd) \
(rbd).recvStatusArr[3] |= TM_RECV_SUSPEND_BYTE

/* Un-suspend (resume) the receiver. */
#define tm_8255x_unsuspend(rbd) \
(rbd).recvStatusArr[3] &= ~TM_RECV_SUSPEND_BYTE

/* Mark a received frame as processed. */
#define tm_8255x_processed(rbd) \
(rbd).recvStatusArr[1] &= ~TM_RECV_COMPLETE_BYTE

/* 
 * 8255x System Control Block offsets 
 */
#define TM_SCB_STATUS   0x00
#define TM_SCB_COMMAND  0x02
#define TM_SCB_INT_MASK 0x03
#define TM_SCB_POINTER  0x04
#define TM_SCB_PORT     0x08

/*
 * SCB PORT commands
 */
#define TM_SCB_PORT_CMD_RESET 0

/* 
 * SCB Receive commands 
 */
/* Start receiving frames */
#define TM_SCB_CMD_RU_START       0x01
/* Resume receiving frames */
#define TM_SCB_CMD_RU_RESUME      0x02
/* Abort current receive operation */
#define TM_SCB_CMD_RU_ABORT       0x04
/* Sets the receive unit's base address */
#define TM_SCB_CMD_LOAD_RU_BASE   0x06

/* 
 * SCB Transmit commands
 */
/* Start transmitting/processing commands */
#define TM_SCB_CMD_CU_START       0x10
/* Resume transmitting/processing commands */
#define TM_SCB_CMD_CU_RESUME      0x20
/* Sets the transmit unit's base address */
#define TM_SCB_CMD_LOAD_CU_BASE   0x60

/* 
 * SCB Misc commands
 */
/* Disable 8255x interrupts */
#define TM_SCB_CMD_DISABLE_INT    0x01
/* Enable 8255x interrupts */
#define TM_SCB_CMD_ENABLE_INT     0xbc

/*
 * Interrupt status flags
 */
/* Good frame received */
#define TM_SCB_FRAME_RECV_INT     0x4000

/*
 * Macros for command/transmit ring
 */
/* Suspend ring processing on completion */
#define TM_CMD_SUSPEND    0x40000000UL
/* Setup individual address */
#define TM_CMD_IND_ADDR   0x00010000UL
/* Setup multicast address */
#define TM_CMD_MULTI_ADDR 0x00030000UL
/* Transmit buffer */
#define TM_CMD_TRANSMIT   0x00040000UL
/* Use flexible transmit model */
#define TM_CMD_FLEX_XMIT  0x00080000UL
/* This command/buffer has completed processing (ie, now owned by the CPU) */
#define TM_CMD_COMPLETE   0x00008000UL
/* Command completed successfully */
#define TM_CMD_OK         0x00004000UL

/*
 * Receive buffer status flag 
 */
/* Suspend receive after receiving this frame */
#define TM_RECV_SUSPEND       0x40000000UL
#define TM_RECV_SUSPEND_BYTE  0x40
/* Use flexible receive model */
#define TM_RECV_FLEX          0x00080000UL
/* No buffer available to receive this frame */
#define TM_RECV_STATUS_NOMEM  0x00000200UL
/* Frame received OK */
#define TM_RECV_OK            0x00002000UL
/* Frame reception complete */
#define TM_RECV_COMPLETE      0x00008000UL
#define TM_RECV_COMPLETE_BYTE 0x80

/* 
 * PCI bus commands 
 */
/* PCI command: enable bus mastering */
#define TM_PCI_BUS_MASTER_CMD 0x0004
/* PCI command: enable I/O space registers */
#define TM_PCI_IO_SPACE_CMD   0x0001
/* PCI command: enable memory mapped registers */
#define TM_PCI_MEM_SPACE_CMD  0x0002

/* Complete command to send to the PCI device */
#define TM_PCI_MODE_CMD       TM_PCI_IO_SPACE_CMD | TM_PCI_MEM_SPACE_CMD | \
                              TM_PCI_BUS_MASTER_CMD


/*
 * Various macros for dealing with the 8255x 
 */

/* Issues a command to the 8255x; don't wait for completion */
#define tm_issue_scb_cmd(command) tm_out_byte(command, TM_SCB_COMMAND)

/* Issues a command to the 8255x and waits for completion */
/* Outputing a word doesn't seem to work, so let's try just a byte... */
#define tm_issue_scb_cmd_wait(command)        \
        tm_out_byte(command, TM_SCB_COMMAND); \
        while (tm_in_byte(TM_SCB_COMMAND))

/* Sets the 8255x SCB General Pointer */
#define tm_set_scb_gen_addr(address) tm_out_dword(address, TM_SCB_POINTER)

#define tm_inc_cmd_index(infoPtr)                                 \
    infoPtr->i8255xCmdRingIndex = (unsigned char)                 \
        ((infoPtr->i8255xCmdRingIndex+1) % (TM_8255X_CMD_RING_SIZE))  \

/*
 * Transmit buffer descriptor.  This structure is used to describe one buffer
 * of a frame.  A frame consists of one or more of these descriptors.
 */
typedef struct tsXmitBufDesc 
{

/* Pointer to buffer's data */
    char TM_FAR * xmitDataPtr;

/* Total size of above buffer */
    unsigned long xmitSize;

} ttXmitBufDesc;

typedef ttXmitBufDesc TM_FAR * ttXmitBufDescPtr;

/*
 * Receive buffer descriptor.  Used to describe one buffer of a frame. A frame
 * could consist of one or more of these buffers, but in this driver will only
 * consist of a single buffer per frame.
 */
typedef struct tsRecvBufDesc
{

/* Amount of data (in bytes) in this buffer.  Set by 8255x. */
    unsigned long                 bufCount;

/* Next buffer in receive buffer descriptor linked list */
    struct tsRecvBufDesc TM_FAR * bufNextPtr;

/* Physical adddress pointer to buffer's data */
    char TM_FAR *                 bufDataPtr;

/* Total size of buffer */
    unsigned long                 bufSize;

/* 
 * Treck buffer handle (not present in normal 8255x descriptor - must remain 
 * at end of structure) 
 */
    ttUserBuffer                  bufHandle;

/* Virtual address of received packet */
    char TM_FAR *                 bufDataVirtPtr;

} ttRecvBufDesc;

typedef ttRecvBufDesc TM_FAR * ttRecvBufDescPtr;


/*
 * Describes a received frame.  This structure is used in conjuction with the
 * receive buffer described above.
 */
typedef struct tsRecvFrameDesc
{

/* Status of this frame */
    union
    {
        unsigned char recvUnStatusArr[4];
        unsigned long recvUnStatus;
    } recvStatusUnion;
#define recvStatus recvStatusUnion.recvUnStatus
#define recvStatusArr recvStatusUnion.recvUnStatusArr

/* Next frame in list */
    struct tsRecvFrameDesc TM_FAR * recvLinkPtr;

/* Pointer to receive buffer descriptor (see above) */
    ttRecvBufDescPtr                recvBufDescPtr;

/* Amount of data in this frame */    
    unsigned short                  recvCount;

/* Total size of the buffers used to receive this frame */    
    unsigned short                  recvSize;    

} ttRecvFrameDesc;

typedef ttRecvFrameDesc TM_FAR * ttRecvFrameDescPtr;

                              
/*
 * Command structures for 8255x Action Commands
 *   These structures are used to reference various commands in the 
 *   command/transmit ring such as transmit, setup individual address, etc.
 *   The generic action command is only used as a placeholder and shouldn'
 *   need to be accessed directly.
 */


/*
 * Generic Action Command
 */
typedef struct tsGenActionCmd {

/* Command status */
    unsigned long           genStatus;

/* Next command in list */
    struct tsGenActionCmd * genLinkPtr;    

/* Reserved space for command parameters (see below) */
    unsigned long           genParam[2];

} ttGenActionCmd;

typedef ttGenActionCmd TM_FAR * ttGenActionCmdPtr;


/* 
 * Individual Address Setup Command
 */
typedef struct tsIndAddrSetup {

/* Command status */
    volatile unsigned long indStatus;

/* Next command in list */
    ttGenActionCmdPtr      indLinkPtr;

/* Ethernet address to assign to the 8255x */
    unsigned short         indAddr[3];

/* Align this structure to 32-bits */
    unsigned short         indPadding;

} ttIndAddrSetup;

typedef ttIndAddrSetup TM_FAR * ttIndAddrSetupPtr;

/* 
 * Multicast Address Setup Command
 */
typedef struct tsMultiAddrSetup {

/* Command status */
    volatile unsigned long multiStatus;

/* Next command in list */
    ttGenActionCmdPtr      multiLinkPtr;

/* Total number of bytes in this list of multicast addresses */
    unsigned short         multiByteCount;

/* List of (in our case, only one) Ethernet addresses to add to hash table */
    unsigned short         multiAddr[3];

/* Align this structure to 32-bits */
    unsigned short         multiPadding;

} ttMultiAddrSetup;

typedef ttMultiAddrSetup TM_FAR * ttMultiAddrSetupPtr;


/* 
 * Transmit Command
 */
typedef struct tsTransmitCmd {

/* Command status */
    unsigned long     xmitStatus;

/* Next command in link */
    ttGenActionCmdPtr xmitLinkPtr;

/* Pointer to transmit buffer descriptor for this frame */
    ttXmitBufDescPtr  xmitTbdPtr;

/* Total number of bytes in this frame */
    unsigned short    xmitByteCount;

/* Number of transmit buffer descriptors in this frame */
    unsigned short    xmitTbdNum;

} ttTransmitCmd;

typedef ttTransmitCmd * ttTransmitCmdPtr;



/*
 * Various state information for this interface
 */
typedef struct ts8255xInfo
{
/* 
 * Action Command/Transmit ring, buffer descriptors and indexes
 */
    ttGenActionCmd  i8255xCmdRing[TM_8255X_CMD_RING_SIZE];        
    ttXmitBufDesc   i8255xXmitBufDesc[TM_8255X_CMD_RING_SIZE]
                                     [TM_8255X_MAX_SCATTER];
/* 
 * Receive frame ring, buffer descriptors and indexes
 */
    ttRecvFrameDesc i8255xRecvDesc[TM_8255X_RECV_RING_SIZE];
    ttRecvBufDesc   i8255xRecvBufDesc[TM_8255X_RECV_RING_SIZE];
/* 
 * PCI device information
 */
/* Base memory address for the 8255x */
    unsigned char TM_FAR * i8255xMemAddrPtr;
/* Treck interface handle for this device */
    ttUserInterface        i8255xInterfaceHandle;

/* Various error counters */
    unsigned long          i8255xSpuriousInterrupts;
/* Base I/O address for the 8255x */
    unsigned long          i8255xIoAddr;

    unsigned char          i8255xCmdRingIndex;
    unsigned char          i8255xCmdRingDirtyIndex;

    unsigned char          i8255xRecvRingIndex;
    unsigned char          i8255xRecvRingDirtyIndex;
    unsigned char          i8255xRecvRingIsrIndex;

/* Current buffer number for current frame */
    unsigned char          i8255xScatterIndex;
/* Interrupt line */
    unsigned char          i8255xIntLine;
/* Indicates whether a transmit queue is being used. */
    unsigned char          i8255xXmitQueueEnb;
    
} tt8255xInfo;

typedef tt8255xInfo *tt8255xInfoPtr;
