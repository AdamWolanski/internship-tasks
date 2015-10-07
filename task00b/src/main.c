#include <stdio.h>
#include "person.h"

int main(int argc, const char *argv[])
{
	const char* name = "Roman";

	struct person* p = create(0);
	p->set_name(p, name);
	printf("Person name: %s\n", p->get_name(p));
	destroy(p);

	return 0;
}

