#ifndef PDOM_PARSER
#define PDOM_PARSER

#include <stdint.h>

typedef struct
{
	char *name;
	char *value;
} pdom_attr;

typedef struct
{
	void **list;
	int length;
} pdom_list;

typedef struct
{
	pdom_list *childrens;
	pdom_list *attrs;
	char *tag;
	size_t isEnd;
	char *content;
	int eq;
	struct pdom_tag *pdom_next;
	struct pdom_tag *prev;
	struct pdom_tag *parent;
} pdom_tag;

typedef struct
{
	pdom_tag **tags;
	char *html;
	int i;
	pdom_tag *current;
	size_t isXml;
} pdom_parser;

size_t pdom_is_equal(pdom_parser *p, char *text);
pdom_list *pdom_parse_attr(pdom_parser *p);
size_t pdom_parse_tag(pdom_parser *p, pdom_tag *tag);
char *pdom_get_content_until(pdom_parser *p, char until, char *str, char first);
size_t pdom_parse_content(pdom_parser *p, char first, pdom_tag *tag);
size_t pdom_parse_comment(pdom_parser *p, pdom_tag *tag);
char *pdom_parse_script(pdom_parser *p);
char *pdom_parse_cdata(pdom_parser *p);
size_t pdom_next1(pdom_parser *p, pdom_tag *tag);
size_t pdom_next(pdom_parser *p, pdom_tag *tag);
int pdom_check_end_tag(pdom_tag *tag);
int pdom_get_tag(pdom_parser *p, pdom_tag *tag);
pdom_list *pdom_parse(pdom_parser *p, pdom_tag *parent);

char *hasNoEndpdom_tags[17] = {"comment", "php", "empty", "!DOCTYPE", "area", "base", "col", "embed", "param", "source", "track", "meta", "link", "br", "input", "hr", "img"};

pdom_list *pdom_parse_attr(pdom_parser *p)
{

	pdom_attr **attrs = malloc(sizeof(pdom_attr *) * 1000);
	char *attr = malloc(sizeof(char));
	char *nowpdom_attr = malloc(sizeof(char));
	char c1;
	int i = 0;
	size_t size = 0, len = 0;
	while (1)
	{
		char c1 = *p->html++;
		if (c1 == '\0' && c1 != '0')
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
			nowpdom_attr = malloc(strlen(attr));
			nowpdom_attr = attr;

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
				if (c2 == '\0' && c2 != '0')
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
			pdom_attr *pass = malloc(sizeof(pdom_attr));

			pass->name = nowpdom_attr;
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

	pdom_list *list = malloc(sizeof(pdom_list));
	list->list = attrs;
	list->length = i;

	return list;
}

size_t pdom_parse_tag(pdom_parser *p, pdom_tag *tag)
{
	if (pdom_is_equal(p, "![CDATA["))
	{
		p->html += 8;
		tag->tag = "cdata";
		return 1;
	}

	if (*(p->html + 1) == '/')
		p->html++;

	char *name = malloc(sizeof(char));
	pdom_list *list = malloc(sizeof(pdom_list));
	list->length = 0;

	size_t size = 0, len = 0;
	while (1)
	{
		char c1 = *p->html++;
		if (c1 == '\0' && c1 != '0')
			break;

		if (c1 == '>')
			break;

		if (c1 == ' ')
		{
			list = pdom_parse_attr(p);
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
	if (name[len - 1] == '/')
	{
		name[strlen(name) - 1] = '\0';
		tag->tag = name;
	}
	return 1;
}

char *pdom_get_content_until(pdom_parser *p, char until, char *str, char first)
{
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
		if (c1 == '\0' && c1 != '0')
			break;

		if (str != 0)
		{

			if (c1 == until && pdom_is_equal(p, str))
			{
				break;
			}
		}
		else if (c1 == until)
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

size_t pdom_parse_content(pdom_parser *p, char first, pdom_tag *tag)
{
	p->html--;

	char *content = pdom_get_content_until(p, '<', 0, first);

	p->html--;

	tag->tag = "empty";
	tag->content = content;
	return 1;
}

size_t pdom_parse_comment(pdom_parser *p, pdom_tag *tag)
{
	p->html += 3;

	char *content = pdom_get_content_until(p, '-', "->", 0);

	p->html += 2;

	tag->tag = "comment";
	tag->content = content;
	return 1;
}

char *pdom_parse_script(pdom_parser *p)
{
	char *content = pdom_get_content_until(p, '<', "/script", 0);
	p->html += 8;
	return content;
}

char *pdom_parse_cdata(pdom_parser *p)
{
	char *content = pdom_get_content_until(p, ']', "]>", 0);
	p->html += 2;
	return content;
}

size_t pdom_next1(pdom_parser *p, pdom_tag *tag)
{
	char c = *p->html++;

	if (c == '\0' && c != '0')
		return 0;

	if (c == '<')
	{
		if (pdom_is_equal(p, "!--"))
		{
			return pdom_parse_comment(p, tag);
		}

		if (*p->html == ' ')
		{
			p->html++;

			return pdom_parse_content(p, '<', tag);
		}

		return pdom_parse_tag(p, tag);
	}
	else
	{
		return pdom_parse_content(p, 0, tag);
	}
}

size_t pdom_next(pdom_parser *p, pdom_tag *tag)
{
	size_t ret = pdom_next1(p, tag);
	if (ret)
		p->current = tag;
	return ret;
}

size_t pdom_is_equal(pdom_parser *p, char *text)
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

int pdom_check_end_tag(pdom_tag *tag)
{
	for (int i = 0; i < 17; i++)
	{
		if (strcmp(tag->tag, hasNoEndpdom_tags[i]) == 0)
			return 1;
	}
	return 0;
}

int pdom_get_tag(pdom_parser *p, pdom_tag *tag)
{

	int ret = pdom_next(p, tag);
	if (!ret)
		return 0;

	if (strcmp(tag->tag, "cdata") == 0)
	{
		tag->content = pdom_parse_cdata(p);
		return 1;
	}

	char *substr = malloc(sizeof(char) * 5);
	for (int i = 0; i < 4; i++)
	{
		substr[i] = tag->tag[i];
	}
	substr[4] = '\0';

	if (strcmp(substr, "?xml") == 0)
	{
		p->isXml = 1;
		return 1;
	}

	if (p->isXml)
		hasNoEndpdom_tags[11] = "";

	if (pdom_check_end_tag(tag))
		return 1;

	if (tag->isEnd)
		return 1;

	if (strcmp(tag->tag, "script") == 0)
	{
		char *content = pdom_parse_script(p);
		tag->content = content;
	}
	else
	{

		pdom_list *list = pdom_parse(p, tag);
		tag->childrens = list;
	}

	if (strcmp(tag->tag, p->current->tag) == 0)
	{
		return 1;
	}

	pdom_tag *etag = malloc(sizeof(pdom_tag));

	while (pdom_next(p, etag))
	{
		if (strcmp(tag->tag, etag->tag) == 0)
			break;
	}

	return 1;
}

pdom_list *pdom_parse(pdom_parser *p, pdom_tag *parent)
{

	pdom_tag **tags = malloc(sizeof(pdom_tag *));
	pdom_tag *stag = malloc(sizeof(pdom_tag));
	int eq = 0;
	int it = 0;

	while (1)
	{
		if (p->html[p->i] == '\0')
		{
			break;
		}

		pdom_tag *tag = malloc(sizeof(pdom_tag));
		pdom_list *childs = malloc(sizeof(pdom_list));
		childs->length = 0;
		pdom_list *attrs = malloc(sizeof(pdom_list));
		attrs->length = 0;

		tag->isEnd = 0;
		tag->childrens = childs;
		tag->attrs = attrs;
		tag->content = "";
		tag->eq = 0;
		tag->pdom_next = 0;
		tag->prev = 0;
		tag->parent = 0;
		int st = pdom_get_tag(p, tag);

		if (!st)
			break;

		if (tag->isEnd && strcmp(parent->tag, tag->tag) == 0)
			break;

		if (!tag->isEnd)
		{

			tag->eq = eq++;
			tag->prev = stag;
			tag->parent = parent;
			stag->pdom_next = tag;
			tags[it++] = tag;
			tags = realloc(tags, sizeof(pdom_tag *) * (it + 1));
		}

		stag = tag;
	}
	pdom_list *list = malloc(sizeof(pdom_list));
	list->list = tags;
	list->length = it;

	return list;
}

#endif
