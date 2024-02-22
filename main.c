#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include "parser.h"

void print_attr(List *list)
{
	for (int i = 0; i < list->length; i++)
	{
		Attr *attr = list->list[i];
		printf("%s => %s \n", attr->name, attr->value);
	}
}

void print_list(List *list)
{

	if( list->length == 0 )
		return;

	for (int i = 0; i < list->length; i++)
	{
		Tag *tag = list->list[i];
		printf("%s => %s\n", tag->tag, tag->content);
		if( tag->attrs->length > 0 ) {
			printf("	attrs => {\n");
			print_attr(tag->attrs);
			printf("	}\n");
		}
		if (tag->childrens->length > 0)
		{
			print_list(tag->childrens);

		}
	}
}


uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec+tv.tv_usec;
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

int main()
{

	char *html = getFileContent("fightclub.html");
	uint64_t time = GetTimeStamp();
	Parser *p = malloc(sizeof(Parser));
	p->html = html;
	p->i = 0;
	Tag *document = malloc(sizeof(Tag));
	document->tag = "document";
	p->current = document;
	document->childrens = parse(p, document);


	printf("%.6f\n", (float)(GetTimeStamp() - time) / 1000 );

	print_list(document->childrens);
}