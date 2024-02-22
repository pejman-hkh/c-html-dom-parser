#ifndef DOM_PARSER
#define DOM_PARSER

#include <stdint.h>

typedef struct
{
	char *name;
	char *value;
} Attr;

typedef struct
{
	void **list;
	int length;
} List;

typedef struct
{
	List *childrens;
	List *attrs;
	char *tag;
	size_t isEnd;
	char *content;
	int eq;
	struct Tag *next;
	struct Tag *prev;
	struct Tag *parent;
} Tag;

typedef struct
{
	Tag **tags;
	char *html;
	int i;
	Tag *current;
	size_t isXml;
} Parser;

size_t isEqual(Parser *p, char *text);
List *parseAttr(Parser *p);
size_t parseTag(Parser *p, Tag *tag);
char *getContentUntil( Parser *p, char until, char *str, char first );
size_t parseContents(Parser *p, char first, Tag *tag);
size_t parseComment(Parser *p, Tag *tag);
char *parseScriptInner(Parser *p);
char *parseCData(Parser *p);
size_t next1(Parser *p, Tag *tag);
size_t next(Parser *p, Tag *tag);
size_t isEqual(Parser *p, char *text);
int checkHasEndTag(Tag *tag);
int getTag(Parser *p, Tag *tag);
List *parse(Parser *p, Tag *parent);


char *hasNoEndTags[17] = {"comment", "php", "empty", "!DOCTYPE", "area", "base", "col", "embed", "param", "source", "track", "meta", "link", "br", "input", "hr", "img"};

List *parseAttr(Parser *p)
{

	Attr **attrs = malloc(sizeof(Attr *) * 1000);
	char *attr = malloc(sizeof(char));
	char *nowAttr = malloc(sizeof(char));
	char c1;
	int i = 0;
	size_t size = 0, len = 0;
	while (1)
	{
		char c1 = *p->html++;
		if(  c1 == '\0' && c1 != '0' )
			break;
		

		if (c1 == ' ')
		{
			len = 0, size = 0;
			attr = malloc(sizeof(char));
			continue;
		}

		char t = 0;
		if (c1 == '=')
		{
			attr[len] = '\0';
			nowAttr = malloc(strlen(attr));
			nowAttr = attr;

			attr = malloc(sizeof(char));
			len = 0, size = 0;
			char g = *p->html;
			if (g == '"' || g == '\'')
			{
				t = g;
				p->html++;
			}

			char *value = malloc(sizeof(char));
			char c2;
			size_t size1 = 0, len1 = 0;
			while (1)
			{

				char c2 = *p->html++;
				if(  c2 == '\0' && c2 != '0' )
					break;
		

				if (!t && c2 == ' ')
					break;
				

				if (!t && c2 == '>')
				{
					p->html--;
					break;
				}

				if (c2 == t)
					break;

				if (len1 + 1 >= size1)
				{
					size1 = size1 * 2 + 1;
					value = realloc(value, sizeof(char) * size1);
				}

				value[len1++] = c2;
			}
			value[len1] = '\0';
			Attr *pass = malloc(sizeof(Attr));

			pass->name = nowAttr;
			pass->value = value;

			attrs[i++] = pass;
			len = 0, size = 0;
			attr = malloc(sizeof(char));
		}

		if (!t && c1 == '=')
			continue;

		if (c1 == '>')
			break;
		
		if (len + 1 >= size)
		{
			size = size * 2 + 1;
			attr = realloc(attr, sizeof(char) * size);
		}

		attr[len++] = c1;
	}

	List *list = malloc(sizeof(List));
	list->list = attrs;
	list->length = i;

	return list;
}

size_t parseTag(Parser *p, Tag *tag)
{
	if (isEqual(p, "![CDATA["))
	{
		p->html += 8;
		tag->tag = "cdata";
		return 1;
	}

	if ( *(p->html + 1) == '/')
		p->html++;

	char *name = malloc(sizeof(char));
	List *list = malloc(sizeof(List));
	list->length = 0;

	size_t size = 0, len = 0;
	while (1)
	{
		char c1 = *p->html++;
		if(  c1 == '\0' && c1 != '0' )
			break;
		
		if (c1 == '>')
			break;
		

		if (c1 == ' ')
		{
			list = parseAttr(p);
			break;
		}

		if (len + 1 >= size)
		{
			size = size * 2 + 1;
			name = realloc(name, sizeof(char) * size);
		}

		name[len++] = c1;
	}

	name[len] = '\0';

	tag->tag = name;
	tag->attrs = list;
	tag->isEnd = 0;

	if (*name == '/')
	{
		tag->isEnd = 1;
		tag->tag = name + 1;
	}
	if (name[len-1] == '/')
	{
		name[strlen(name) - 1] = '\0';
		tag->tag = name;
	}
	return 1;
}

char *getContentUntil( Parser *p, char until, char *str, char first ) {
	char *content = malloc(sizeof(char) * 2);
	size_t size = 0, len = 0;

	if (first)
	{
		content[0] = first;
		len++;
	}

	char c1;
	while (1)
	{

		char c1 = *p->html++;
		if(  c1 == '\0' && c1 != '0' )
			break;
		

		if ( str != 0 ) {

			if( c1 == until && isEqual(p, str) ) {
				break;
			}
		}
		else if ( c1 == until )
			break;

		if (len + 1 >= size)
		{
			size = size * 2 + 1;
			content = realloc(content, sizeof(char) * size);
		}

		content[len++] = c1;
	}
	content[len] = '\0';
	return content;
}

size_t parseContents(Parser *p, char first, Tag *tag)
{
	p->html--;

	char *content = getContentUntil(p, '<', 0, first);
	
	p->html--;

	tag->tag = "empty";
	tag->content = content;
	return 1;
}

size_t parseComment(Parser *p, Tag *tag)
{
	p->html += 3;

	char *content = getContentUntil(p,'-', "->", 0);

	p->html += 2;

	tag->tag = "comment";
	tag->content = content;
	return 1;
}

char *parseScriptInner(Parser *p)
{
	char *content = getContentUntil(p, '<', "/script", 0);
	p->html += 8;
	return content;
}

char *parseCData(Parser *p)
{
	char *content = getContentUntil(p,']', "]>", 0);
	p->html += 2;
	return content;
}

size_t next1(Parser *p, Tag *tag)
{
	char c = *p->html++;
	
	if(  c == '\0' && c != '0' )
		return 0;
		
	if (c == '<')
	{
		if (isEqual(p, "!--"))
		{
			return parseComment(p, tag);
		}

		if (*p->html == ' ')
		{
			p->html++;

			return parseContents(p, '<', tag);
		}

		return parseTag(p, tag);
	}
	else
	{
		return parseContents(p, 0, tag);
	}
}

size_t next(Parser *p, Tag *tag)
{
	size_t ret = next1(p, tag);
	if (ret)
		p->current = tag;
	return ret;
}

size_t isEqual(Parser *p, char *text)
{

	char *html = p->html;
	char c;
	while (c = *text++)
	{
		char h = *html++;

		if (h == EOF)
		{
			return 0;
		}

		if (h != c)
			return 0;
	}

	return 1;
}

int checkHasEndTag(Tag *tag)
{
	for (int i = 0; i < 17; i++)
	{
		if (strcmp(tag->tag, hasNoEndTags[i]) == 0)
			return 1;
	}
	return 0;
}

int getTag(Parser *p, Tag *tag)
{

	int ret = next(p, tag);
	if ( ! ret )
		return 0;
	

	if (strcmp(tag->tag, "cdata") == 0)
	{
		tag->content = parseCData(p);
		return 1;
	}

	char *substr = malloc(sizeof(char)*5);
	for( int i = 0; i < 4; i++) {
		substr[i] = tag->tag[i];
	}
	substr[4] = '\0';

	if (strcmp(substr, "?xml") == 0)
	{
		p->isXml = 1;
		return 1;
	}

	if (p->isXml)
		hasNoEndTags[11] = "";


	if (checkHasEndTag(tag))
		return 1;

	if (tag->isEnd)
		return 1;

	if ( strcmp(tag->tag, "script") ==0 )
	{
		char *content = parseScriptInner(p);
		tag->content = content;
	}
	else
	{

		List *list = parse(p, tag);
		tag->childrens = list;
	}

	if (strcmp(tag->tag, p->current->tag) == 0)
	{
		return 1;
	}

	Tag *etag = malloc(sizeof(Tag));

	while (next(p, etag))
	{
		if (strcmp(tag->tag, etag->tag) == 0)
			break;
	}

	return 1;
}

List *parse(Parser *p, Tag *parent)
{

	Tag **tags = malloc(sizeof(Tag *));
	Tag *stag = malloc(sizeof(Tag));
	int eq = 0;
	int it = 0;

	while (1)
	{
		if( p->html[p->i] == '\0' ) {
			break;
		}

		Tag *tag = malloc(sizeof(Tag));
		List *childs = malloc(sizeof(List));
		childs->length = 0;
		List *attrs = malloc(sizeof(List));
		attrs->length = 0;
		
		tag->isEnd = 0;
		tag->childrens = childs;
		tag->attrs = attrs;
		tag->content = "";
		tag->eq = 0;
		tag->next = 0;
		tag->prev = 0;
		tag->parent = 0;
		int st = getTag(p, tag);
	

		if ( !st )
			break;


		if (tag->isEnd && strcmp(parent->tag, tag->tag) == 0)
			break;
		
		if (!tag->isEnd)
		{
			
			tag->eq = eq++;
			tag->prev = stag;
			tag->parent = parent;
			stag->next = tag;
			tags[it++] = tag;
			tags = realloc(tags, sizeof(Tag *) * (it + 1));
		}

		stag = tag;
	}
	List *list = malloc(sizeof(List));
	list->list = tags;
	list->length = it;

	return list;
}

#endif
