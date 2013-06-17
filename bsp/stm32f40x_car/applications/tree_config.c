/**
 * @author: ideawu
 * @url: http://www.ideawu.net
 * @description: C 语言使用配置文件库.
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_config.h"
#include <rtthread.h>

#define PRINT_TRACE(fmt, args...)	rt_kprintf(__FILE__ "(%d): " fmt "\n", __LINE__, ##args)


__inline static int is_whitespace(const int ch)
{	
	return strchr(" \t\r\n", ch) != NULL;
}

__inline static int is_alpha(const int ch)
{	
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

__inline static int is_digit(const int ch)
{	
	return (ch >= '0' && ch <= '9');
}

__inline static int is_wordchar(const int ch)
{	
	return (ch == '_') || is_alpha(ch) || is_digit(ch);
}
__inline static int is_empty_str(const char *str)
{	
	const char *p = str;	
	while(*p && is_whitespace(*p))
	{		
		p++;	
	}	
	return *p == '\0';
}

/* 返回左边不包含空白字符的字符串的指针 */
__inline static char *ltrim(char *str)
{	
	char *p = str;	
	while(*p && is_whitespace(*p))
	{		
		p++;	
	}	
	return p;
}

/* 返回右边不包含空白字符的字符串的指针 */
__inline static char *rtrim(char *str)
{	
	char *p;	
	p = str + strlen(str) - 1;	
	while(p >= str && is_whitespace(*p))
	{
		p--;	
	}	
	*(p + 1) = '\0';
	return str;
}

/* 返回左右两边不包含空白字符的字符串的指针 */
__inline static char *trim(char *str)
{	
	char *p;	
	p = ltrim(str);	
	rtrim(p);
	return p;
}





static int is_kv_seperator(int ch){
	return strchr(":=", ch) != NULL;
}

static struct tree_config *cfg_find_child(struct tree_config *cfg, char *key);

static struct tree_config *add_node(struct tree_config *parent, char *key, char *val, int lineno){
	struct tree_config *cfg;
	int size;

	if(key[0] != '#' && cfg_find_child(parent, key)){
		log_error("line: %d, duplicated '%s'", lineno, key);
		return NULL;
	}

	if(parent->size == parent->count){
		size = parent->size * 2 + 5;
		cfg = realloc(parent->items, size * sizeof(*cfg));
		if(!cfg){
			return NULL;
		}

		PRINT_TRACE("realloc items: %d => %d", parent->size, size);

		parent->items = cfg;
		parent->size = size;
	}

	cfg = &parent->items[parent->count++];
	cfg->parent = parent;
	cfg->items = NULL;
	cfg->size = cfg->count = 0;

	cfg->key = malloc(strlen(key) + 1);
	assert(cfg->key);
	strcpy(cfg->key, key);

	cfg->val = malloc(strlen(val) + 1);
	assert(cfg->val);
	strcpy(cfg->val, val);

	PRINT_TRACE("'%s' <= '%s'", parent->key, cfg->key);

	return cfg;
}

struct tree_config *cfg_load_file(char *filename){
	struct tree_config *root_cfg, *cfg;
	FILE *fp;
	char buf[CONFIG_MAX_LINE + 3];
	char *key, *val;
	int lineno = 0;
	int indent, last_indent;
	int i, len;

	fp = fopen(filename, "r");
	if(!fp){
		log_error("error opening file '%s': %s", filename, strerror(errno));
		return NULL;
	}

	root_cfg = calloc(1, sizeof(*cfg));
	if(!root_cfg){
		goto err;
	}
	root_cfg->key = malloc(strlen("root") + 1);
	assert(root_cfg->key);
	strcpy(root_cfg->key, "root");

	root_cfg->val = malloc(strlen("") + 1);
	assert(root_cfg->val);
	strcpy(root_cfg->val, "");

	cfg = root_cfg;
	last_indent = 0;

	while(fgets(buf, sizeof(buf), fp)){
		lineno++;

		rtrim(buf);
		len = strlen(buf);

		if(len > CONFIG_MAX_LINE){
			log_error("line(%d) too long: %d, should be no more than %d characters",
				lineno, len, CONFIG_MAX_LINE);
			goto err;
		}
		if(is_empty_str(buf)){
			continue;
		}

		/* 有效行以 \t* 开头 */
		indent = 0;
		key = buf;
		while(*key == '\t'){
			indent++;
			key++;
		}

		if(indent <= last_indent){
			for(i = indent; i <= last_indent; i++){
				/* 第一个配置时, 此条件为真 */
				if(cfg != root_cfg){
					cfg = cfg->parent;
				}
			}
		}else if(indent > last_indent + 1){
			log_error("invalid indent line(%d)", lineno);
			goto err;
		}

		/* 注释行以 \t*# 开头 */
		if(*key == '#'){
			PRINT_TRACE("%s", key);
			cfg = add_node(cfg, "#", key + 1, lineno);
			if(cfg == NULL){
				goto err;
			}
			last_indent = indent;
			continue;
		}else if(is_whitespace(*key)){
			log_error("invalid line(%d): unexpected whitespace char '%c'", lineno, *key);
			goto err;
		}

		val = key;
		/* 跳过键名 */
		while(*val && !is_kv_seperator(*val)){
			val++;
		}
		if(*val == '\0'){
			log_error("invalid line(%d): %s, expecting ':' or '='", lineno, key);
			goto err;
		}else if(!is_kv_seperator(*val)){
			log_error("invalid line(%d): unexpected char '%c', expecting ':' or '='", lineno, *val);
			goto err;
		}
		*val++ = '\0';

		/* key 或者 value 的前后空白字符会被过滤 */
		key = trim(key);
		val = trim(val);

		cfg = add_node(cfg, key, val, lineno);
		if(cfg == NULL){
			goto err;
		}

		last_indent = indent;
	}
	if(ferror(fp)){
		log_error("error while reading file %s", filename);
		goto err;
	}
	fclose(fp);

	return root_cfg;

err:
	if(root_cfg){
		cfg_free(root_cfg);
	}

	fclose(fp);
	return NULL;
}

static void _cfg_free(struct tree_config *cfg, int indent){
	struct tree_config *c;
	int i;

	PRINT_TRACE("%*sfree %s(%d)", indent * 4, "", cfg->key, cfg->count);

	if(cfg->key){
		free(cfg->key);
	}
	if(cfg->val){
		free(cfg->val);
	}
	if(cfg->items){
		for(i = 0; i < cfg->count; i++){
			c = &cfg->items[i];
			_cfg_free(c, indent + 1);
		}
		free(cfg->items);
	}
}

void cfg_free(struct tree_config *cfg){
	_cfg_free(cfg, 0);
	free(cfg);
}

static struct tree_config *cfg_find_child(struct tree_config *cfg, char *key){
	struct tree_config *c;
	int i;

	/* PRINT_TRACE("find %s", key); */
	for(i = 0; i < cfg->count; i++){
		c = &cfg->items[i];
		if(strcmp(key, c->key) == 0){
			return c;
		}
	}

	return NULL;
}

struct tree_config *cfg_get(struct tree_config *cfg, char *key){
	char path[CONFIG_MAX_LINE];
	struct tree_config *c;
	char *f, *fs; /* field, field seperator */

	assert(key);
	c = cfg;
	strcpy(path, key);

	f = fs = path;
	while(c){
		switch(*fs++){
			case '.':
			case '/':
				*(fs - 1) = '\0';
				c = cfg_find_child(c, f);
				f = fs;
				break;
			case '\0':
				c = cfg_find_child(c, f);
				return c;
			default:
				break;
		}
	}

	return c;
}

int cfg_num(struct tree_config *cfg){
	return atoi(cfg->val);
}

char *cfg_str(struct tree_config *cfg){
	return cfg->val;
}

int cfg_getnum(struct tree_config *cfg, char *key){
	int val;
	struct tree_config *c;

	c = cfg_get(cfg, key);
	if(!c){
		return 0;
	}

	val = atoi(c->val);
	return val;
}

char *cfg_getstr(struct tree_config *cfg, char *key){
	struct tree_config *c;

	c = cfg_get(cfg, key);
	if(!c){
		return NULL;
	}

	return c->val;
}

static void _cfg_print(struct tree_config *cfg, int indent, FILE *fp){
	struct tree_config *c;
	char fs;
	int i, n;

	for(i = 0; i < cfg->count; i++){
		c = &cfg->items[i];
		for(n = 0; n < indent; n++){
			putc('\t', fp);
		}

		if(IS_COMMENT_CONFIG(c)){
			fprintf(fp, "#%s\n", c->val);
		}else{
			fs = c->count? ':' : '=';
			fprintf(fp, "%s %c %s\n", c->key, fs, c->val);
		}

		if(c->count > 0){
			_cfg_print(c, indent + 1, fp);
		}
	}
}

void cfg_print(struct tree_config *cfg, FILE *fp){
	printf("###### tree_config start #######\n\n");
	_cfg_print(cfg, 0, fp);
	printf("\n###### tree_config end #######\n");
}








