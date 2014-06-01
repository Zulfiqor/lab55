#define SYSFS


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

#ifdef SYSFS
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#else
#include <linux/proc_fs.h>
#endif

// names
#define ARG1 "arg1"
#define ARG2 "arg2"
#define OPERATION "operation"
#define RESULT "result"

#define PARENT_DIR "calc"


#define WRITE_SIZE 100

static char arg1_input[WRITE_SIZE];
static char arg2_input[WRITE_SIZE];
static char operation_input[WRITE_SIZE];



long calculate(void) {
	long a1 = 0;
	long a2 = 0;
	long res = 0;

	if (arg1_input[strlen(arg1_input) - 2] == '\n') {
		arg1_input[strlen(arg1_input) - 2] = (char)0;
	}

	kstrtol(arg1_input, 10, &a1);
	kstrtol(arg2_input, 10, &a2);

	if (operation_input[0] == '+') {
		res = a1 + a2;
	} else if (operation_input[0] == '-') {
		res = a1 - a2;
	} else if (operation_input[0] == '.') {
		res = a1 * a2;
	} else if (operation_input[0] == '/') {
		res = a1 / a2;
	}
	return res;
}

#ifdef SYSFS

static struct attribute arg1 = {
	.name = ARG1,
	.mode = 0666,
};

static struct attribute arg2 = {
	.name = ARG2,
	.mode = 0666,
};

static struct attribute operation = {
	.name = OPERATION,
	.mode = 0666,
};

static struct attribute result = {
	.name = RESULT,
	.mode = 0666,
};

static struct attribute * calc_attributes[] = {
	&arg1,
	&arg2,
	&operation,
	&result,
	NULL
};


static ssize_t default_show(struct kobject *kobj, struct attribute *attr,
		char *buf)
{
	if (!strcmp(attr->name, RESULT)) {
		long res = calculate();

		return sprintf(buf, "%ld\n", res);
	} else {
		return 0;
	}
}

static ssize_t default_store(struct kobject *kobj, struct attribute *attr,
		const char *buf, size_t len)
{
	if(len > WRITE_SIZE) {
		len = WRITE_SIZE;
	}

	if (!strcmp(attr->name, ARG1)) {
		memcpy(arg1_input, buf, len);
	} else if (!strcmp(attr->name, ARG2)) {
		memcpy(arg2_input, buf, len);
	} else if (!strcmp(attr->name, OPERATION)) {
		memcpy(operation_input, buf, len);
	}
	return len;
}

static struct sysfs_ops calc_ops = {
	.show = default_show,
	.store = default_store,
};

static struct kobj_type calc_type = {
	.sysfs_ops = &calc_ops,
	.default_attrs = calc_attributes,
};

struct kobject *calc_obj;
static int __init sysfsexample_module_init(void)
{
	int err = -1;
	calc_obj = kzalloc(sizeof(*calc_obj), GFP_KERNEL);
	if (calc_obj) {
		kobject_init(calc_obj, &calc_type);
		if (kobject_add(calc_obj, NULL, "%s", PARENT_DIR)) {
			 err = -1;
			 printk("Sysfs creation failed\n");
			 kobject_put(calc_obj);
			 calc_obj = NULL;
		}
		err = 0;
	}
	return err;
}

static void __exit sysfsexample_module_exit(void)
{
	if (calc_obj) {
		kobject_put(calc_obj);
		kfree(calc_obj);
	}
}

module_init(sysfsexample_module_init);
module_exit(sysfsexample_module_exit);
MODULE_LICENSE("GPL");

