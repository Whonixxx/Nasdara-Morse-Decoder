// Custom RPC IDs for server-to-client synchronization (keep IDs unique to avoid mod conflicts)
enum EMorseDecoderRPC
{
	RPC_SHOW_DECODED_BUNKER = 784512, // Sends decoded bunker coordinates and password to client notification
	RPC_SYNC_BUNKER_MARKER  = 784513 // Syncs optional map marker data with the client
};
