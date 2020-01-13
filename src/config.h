#ifndef LATEX4EE_CONFIG_H
#define LATEX4EE_CONFIG_H

#include <stdint.h>

typedef enum ini_keys {
	INI_KEY_INVALID_KEY = -1,
	INI_KEY_HOST = 0, /* Host for which server will answer queries */
	INI_KEY_PORT,     /* Port on which server will listen */
	INI_KEY_ROOT,     /* Root folder to serve web content */
	INI_KEY_N_KEYS
} INI_KEY_T;

typedef struct conf_kv_t{
	INI_KEY_T key;
	const char * value;
} CONF_KV_T;

#ifdef __CONFIG_C__
const int CONFIG_KEY_MAXLEN = 128;
const int CONFIG_VAL_MAXLEN = 128;

const char * ini_keys_str[INI_KEY_N_KEYS] = {
	"HOST",
	"PORT",
	"ROOT"
};

/*
CONF_KV_T default_config[] = [ {8080,
	"root",
	"localhost"
};
*/
#else
extern const char * ini_keys_str[INI_KEY_N_KEYS];
#endif

CONF_KV_T* read_config(const char * filepath);
const char * config_lookup_key_str(const CONF_KV_T* config, const INI_KEY_T key);
const long config_lookup_key_long(const CONF_KV_T* config, const INI_KEY_T key);

#endif
