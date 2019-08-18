/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sna.h
 * Author: nick_couchman
 *
 * Created on August 4, 2019, 1:43 PM
 */

#ifndef GUAC_SNA_H
#define GUAC_SNA_H

/**
 * This is the packet header that defines traffic on the TN5250 link as
 * SNA traffic.  It will be present on traffic coming from the mainframe,
 * and should also be sent for any SNA-specific traffic to the mainframe.
 */
#define SNA_PACKET_HEADER 0x12a0

/**
 * This is a constant header on all SNA packets.
 */
#define SNA_VAR_HEADER 0x04

/**
 * Operational codes as defined in RFC1207.
 * 
 * No operation.
 */
#define OPCODE_NOOP 0x00

/**
 * Invite operation, sent by server to allow client to talk.
 */
#define OPCODE_INVITE 0x01

/**
 * Output only
 */
#define OPCODE_OUTPUT 0x02

/**
 * Put/get operation
 */
#define OPCODE_PUT_GET 0x03

/**
 * Save screen
 */
#define OPCODE_SAVE_SCREEN 0x04

/**
 * Restore screen
 */
#define OPCODE_RESTORE_SCREEN 0x05

/**
 * Read immediate
 */
#define OPCODE_READ_IMMEIDATE 0x06

/**
 * Read screen
 */
#define OPCODE_READ_SCREEN 0x08

/**
 * Cancel invite
 */
#define OPCODE_CANCEL_INVITE 0x0a

/**
 * Message light on
 */
#define OPCODE_MSG_ON 0x0b

/**
 * Message light off
 */
#define OPCODE_MSG_OFF 0x0c

/**
 * A 16-bit set of flags, as defined in RFC1205, Section 3, for TN5250-specific
 * data transfer.
 */
struct sna_flags {
    
    /**
     * Indicates data stream output error.
     */
    unsigned int ERR : 1;
    
    /**
     * 5250 Attention Key has been pressed.
     */
    unsigned int ATN : 1;
    
    /**
     * Reserved/unused.
     */
    unsigned int : 3;
    
    /**
     * System Request key was pressed.
     */
    unsigned int SRQ : 1;
    
    /**
     * Test Request key was pressed.
     */
    unsigned int TRQ : 1;
    
    /**
     * Help in Error State function - error code will follow header in data.
     */
    unsigned int HLP : 1;
    
    /**
     * Final 8 bits are reserved/unused.
     */
    unsigned int : 8;
    
} sna_flags;

/**
 * Defines the structure of a TN5250 (SNA over Telnet) packet, including
 * the fields and the length of each field, as described in RFC1205.
 */
struct sna_packet {
    
    /**
     * The length of the packet.
     */
    unsigned int len : 16;
    
    /**
     * The SNA header marker, 0x12a0
     */
    unsigned int sna_header : 16;
    
    /**
     * Reserved portion, should be zeroes.
     */
    unsigned int reserved : 16;
    
    /**
     * The variable header length, always 0x04.
     */
    unsigned int varlen : 8;
    
    /**
     * SNA flags.
     */
    unsigned int sna_flags : 16;
    
    /**
     * The opcode.
     */
    unsigned int opcode : 8;
    
    /**
     * Any data sent in the packet.
     */
    void* sna_data;
    
    /**
     * IAC and EOR marker.
     */
    unsigned int sna_end : 16;
    
} sna_packet;

#endif /* SNA_H */

