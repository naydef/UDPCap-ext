#include "fastdefs.h"
#include <stdio.h>

const char * netadr_t::ToString(bool baseOnly) const
{
	static char	s[64];

	strncpy(s, "unknown", sizeof(s));

	if (type == NA_LOOPBACK)
	{
		strncpy (s, "loopback", sizeof(s));
	}
	else if (type == NA_BROADCAST)
	{
		strncpy (s, "broadcast", sizeof(s));
	}
	else if (type == NA_IP)
	{
		if(baseOnly)
		{
			_snprintf_s(s, sizeof(s), "%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
		}
		else
		{
			_snprintf_s (s, sizeof(s), "%i.%i.%i.%i:%i", ip[0], ip[1], ip[2], ip[3], alt_ntohs(port));
		}
	}

	return s;
}
