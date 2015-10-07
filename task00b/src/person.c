#include "person.h"
#include <stdlib.h>
#include <string.h>

struct person_data {
	char* name;
};

static void set_name(struct person* self, const char* p_name)
{
	if (self != NULL) {
		if (self->pd != NULL) {
			if (self->pd->name != NULL)
				free(self);
			if (p_name != NULL) {
				self->pd->name = (char*) malloc((strlen(p_name)+1) * sizeof(char));
				strcpy(self->pd->name, p_name);
			} else {
				self->pd->name = 0;
			}
		} else if(self->pd == NULL){
			self->pd = malloc(sizeof(struct person));
			self->pd->name = malloc((strlen(p_name)+1));
			strcpy(self->pd->name, p_name);
		}
	}
}

static const char* get_name(struct person* self)
{
	if (self && self->pd)
		return self->pd->name;
	return 0;
}

struct person* create(const char* p_name)
{
	struct person* result = malloc(sizeof(struct person));
	result->pd = 0;
	result->set_name = set_name;
	result->get_name = get_name;
	
	if (p_name != 0) {
		result->pd = malloc(sizeof(struct person_data));
		result->pd->name = malloc(strlen(p_name)+1);
		strcpy(result->pd->name, p_name);
	}

	return result;
}

void destroy(struct person* p_person)
{
	if (p_person)
		if (p_person->pd)
			if (p_person->pd->name)
				free(p_person->pd->name);
			free(p_person->pd);
		free(p_person);

}

