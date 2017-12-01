#ifndef FASTDEFS_H_INCLUDED
#define FASTDEFS_H_INCLUDED

#include <cstdint>
#include "tier1\bitbuf.h"

inline unsigned short alt_ntohs(unsigned short num)
{
    return (num>>8) | (num<<8);
}

typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;

typedef uint32_t uint;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef struct netadr_s
{
public:
	const char*		ToString( bool onlyBase = false ) const; // returns xxx.xxx.xxx.xxx:ppppp

public:	// members are public to avoid to much changes

	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;

typedef struct netpacket_s
{
	netadr_t		from;		// sender IP
	int				source;		// received source
	double			received;	// received time
	unsigned char	*data;		// pointer to raw packet data
	bf_read			message;	// easy bitbuf data access
	int				size;		// size in bytes
	int				wiresize;   // size in bytes before decompression
	bool			stream;		// was send as stream
	struct netpacket_s *pNext;	// for internal use, should be NULL in public
} netpacket_t;

#endif // FASTDEFS_H_INCLUDED
