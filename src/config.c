#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <ini_parser.h>

#define __CONFIG_C__
#include "config.h"

/* returns list of configuration kv pairs terminated by an entry with key<-INVALID_KEY */
CONF_KV_T* read_config(const char * filepath)
{
	CONF_KV_T* conf = malloc(sizeof(CONF_KV_T)*(INI_KEY_N_KEYS + 1)); 
	if(NULL == conf)
	{
		syslog(LOG_ERR, "Failed to allocate memory for configuration array.\
				Unrecoverable error, exiting from %s:%d",
				__FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	FILE *f = fopen(filepath, "r");
	if(!f) 
	{
		syslog(LOG_WARNING, 
				"Failed to open configuration file at %s. Using compile-time defaults specified in config.h",
				filepath);
		// FIXME add defaults in config.h
	}

	int type;
	CONF_KV_T *conf_tmp = conf;
	INI_KEY_T ini_key = INI_KEY_INVALID_KEY;
	do {
		char key[CONFIG_KEY_MAXLEN], value[CONFIG_VAL_MAXLEN];
		type = parse_ini_file(f, key, sizeof(key), value, sizeof(value));
		char * tmpk = key;
		for(;0 != *tmpk; tmpk++)
			*tmpk = toupper(*tmpk);
		switch(type){
			case INI_SECTION:
				break;
			case INI_VALUE:
				ini_key = INI_KEY_INVALID_KEY;
				for(int i = 0; i < INI_KEY_N_KEYS; i++)
				{
					if(0 == strcmp(ini_keys_str[i],key))
					{
						ini_key = (INI_KEY_T)i;
						conf_tmp->key = ini_key;
						conf_tmp->value = strdup(value);
						if(NULL == conf_tmp->value)
						{
							syslog(LOG_ERR, 
									"Failed to allocate memory for configuration value %s:%d.\
									Unrecoverable error, exiting...",
									__FILE__,__LINE__);
							exit(EXIT_FAILURE);
						}
						conf_tmp++;
						break;
					}
				}
				if(INI_KEY_INVALID_KEY==ini_key)
				{
					fprintf(stderr, "Unrecognised key %s with value %s in config file %s\n",
							key, value, filepath);
				}
				break;
		}
	} while (type);

	fclose(f);

	conf_tmp->key = INI_KEY_INVALID_KEY;
	conf_tmp->value = NULL;
	return conf;
}
