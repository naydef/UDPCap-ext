#if defined _udpcap_included
 #endinput
#endif
#define _udpcap_included

enum netadrtype_t
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
};

forward Action UDPC_ProcessIncomingPacket(const char[] ip, int port, const char[] data, int length, netadrtype_t netaddrtype);



public Extension __ext_udpcap = 
{
	name = "UDPCap",
	file = "UDPCap.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};