struct person_data;

struct person {
	void (*set_name)(struct person* self, const char* p_name);
	const char* (*get_name)(struct person* self);
	struct person_data* pd;
};

struct person* create(const char* p_name);
void destroy(struct person* p_person);

