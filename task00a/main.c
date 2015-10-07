#include <stdio.h>
#include <stdlib.h>

#include "person.h"


int main(int argc, char **argv)
{

	struct person *p = create();

	p->set_name(p, "123 Test");

	printf("Pobrano: %s\n", p->get_name(p));

	return 0;



}




