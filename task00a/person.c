#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"

struct person_ctx
{
	char *name;
};

//static?
static void *set_name(struct person *self, const char *name)
{
    self->ctx = malloc(sizeof(struct person));
    self->ctx->name = malloc(strlen(name)+1);
    strcpy(self->ctx->name, name);
/*
	if (self->ctx == NULL && self->ctx->name == NULL)
	{
		free(self->ctx->name);
	}
	else if (self->ctx)
	{
		self->ctx = malloc(sizeof(struct person));
		self->ctx->name = malloc(strlen(name)+1);
		strcpy(self->ctx->name, name);
	}
*/
}


const char* get_name(struct person *self)
{
	return self->ctx->name;
}

struct person *create()
{
	struct person *result = malloc(sizeof(struct person));
	result->ctx = 0;
	result->set_name = (void*)set_name;
	result->get_name = get_name;
	return result;
}


void delete(struct person *self)
{
	if (self)
	{
		if (self->ctx)
		{
			if (self->ctx->name)
			{
				free(self->ctx->name);
			}
			free(self->ctx);
		}
		free(self);
	}
}





