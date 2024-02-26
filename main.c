#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include "parser.h"

void print_attr(pdom_list *list)
{
	for (int i = 0; i < list->length; i++)
	{
		pdom_attr *attr = list->list[i];
		printf("%s => %s \n", attr->name, attr->value);
	}
}

void print_list(pdom_list *list)
{

	if (list->length == 0)
		return;

	for (int i = 0; i < list->length; i++)
	{
		pdom_tag *tag = list->list[i];
		printf("%s => %s\n", tag->tag, tag->content?tag->content:"");
		if ( tag->attrs && tag->attrs->length > 0)
		{
			printf("	attrs => {\n");
			print_attr(tag->attrs);
			printf("	}\n");
		}
		if (tag->childrens && tag->childrens->length > 0)
		{
			print_list(tag->childrens);
		}
	}
}

uint64_t GetTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec;
}

char *getFileContent(char *filename)
{
	char *buffer = 0;
	long length;
	FILE *f = fopen(filename, "rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer)
		{
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}

	return buffer;
}

char * concat_string( char *str, char *format, ... ) {

    va_list args;
    va_start(args, format);

	int size = strlen(str);
	int i = size;

	size += strlen( format );
	
	str = realloc(str, sizeof(char) * (size+1) );

	char c;
	while( c = *format++ ) {
		if( c == '%' ) {
			c = *format++;
		
			if( c == 'c' ) {
				char cc = va_arg(args,int);
		
				str[i++] = cc;	
			} else {
				char *value = va_arg(args,char *);
				size += strlen( value);
		
				str = realloc(str, sizeof(char) * (size+1) );
				char c1;
				while( c1 = *value++ ) {
					str[i++] = c1;
				}
			}
		} else {
			str[i++] = c;
		}
	}
	str[i] = '\0';
    va_end(args);
	return str;
}

char *json_encode( pdom_list *list ) {

	char *ret = calloc( sizeof(char), 1 );
	char pre1;
	pre1 = ' ';
	ret = concat_string(ret, "[");
	for (int i = 0; i < list->length; i++)
	{
		
		ret = concat_string(ret, "%c{", pre1 );
		//printf("%s\n", ret);
		
		pdom_tag *tag = list->list[i];
		if( tag->content ) {
			char *tmp = tag->content;
			char c;
			int i = 0;
			char *new_content = malloc(sizeof(char) * 2 );
			while( c = *tmp++ ) {
				if( c == '\\' ) {
					new_content[i++] = '\\';
					new_content[i++] = '\\';
				} else if( c == '"' ) {
					new_content[i++] = '\\';
					new_content[i++] = '"';
				}
				else if( c == '\n' ) {
					new_content[i++] = '\\';
					new_content[i++] = 'n';
				}
				else if( c == '\t' )  {
					new_content[i++] = '\\';
					new_content[i++] = 't';
				}
				else
					new_content[i++] = c;
				new_content = realloc(new_content, sizeof( char ) * (i+3) );
			}
			new_content[i] = '\0';
			ret = concat_string(ret, "\"tag\":\"%s\",\"content\":\"%s\"", tag->tag, new_content);
		} else {
			ret = concat_string(ret, "\"tag\":\"%s\"", tag->tag);
	
		}
			
		
		if ( tag->attrs && tag->attrs->length > 0)
		{
			ret = concat_string(ret, ",\"attrs\":[");
			char pre;
			pre = ' ';
			for (int i = 0; i < tag->attrs->length; i++)
			{
				pdom_attr *attr = tag->attrs->list[i];
				ret = concat_string(ret, "%c{\"%s\":\"%s\"}", pre, attr->name, attr->value);
				pre = ',';
			}
			ret = concat_string(ret, "]");
		}
		if (tag->childrens && tag->childrens->length > 0)
		{
	
			char *childs = json_encode(tag->childrens);

			ret = concat_string(ret, ",\"childrens\":%s", childs );
		}
		ret = concat_string(ret, "}" );
		
		pre1 = ',';
	}
	ret = concat_string(ret, "]");
	return ret;
}

int main()
{

	char *html = getFileContent("fightclub.html");
	//while( 1 ) {
		uint64_t time = GetTimeStamp();
		pdom_parser *p = malloc(sizeof(pdom_parser));
		p->html = html;
		p->i = 0;
		pdom_tag *document = malloc(sizeof(pdom_tag));
		document->tag = "document";
		p->current = document;
		document->childrens = pdom_parse(p, document);

		printf("%.6f\n", (float)(GetTimeStamp() - time) / 1000);
		printf("%s\n", json_encode(document->childrens) );
		pdom_free( document );
	//}

	//print_list(document->childrens);

}
