#ifndef LATEX4EE_POST_H
#define LATEX4EE_POST_H

int iterate_post(
		void* con_info_cls, enum MHD_ValueKind kind, const char *key,
		const char* filename, const char* content_type,
		const char* transfer_encoding, const char* data, uint64_t off, size_t size);

#endif
