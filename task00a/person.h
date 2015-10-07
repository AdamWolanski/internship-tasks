struct person_ctx; //kontekst

struct person
{
	struct person_ctx *ctx;
	void (*set_name)(struct person *self, const char *name);
	const char* (*get_name)(struct person *self);
};

extern struct person* create();
void delete(struct person *self);





