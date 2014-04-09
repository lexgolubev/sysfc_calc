#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

//#define PROC

#define MAX_PROC_SIZE 100

static int arg1;
static int arg2;
static char op;
static int answer;

void calculate()
{
  if (op == '+') {
    answer = arg1 + arg2;
  } else if (op == '-') {
    answer = arg1 - arg2;
  } else if (op == '*') {
    answer = arg1 * arg2;
  } else if (op == '/'){
    answer = arg1 / arg2;
  }
}

#ifdef PROC
static struct proc_dir_entry *proc_arg1_entry;
static struct proc_dir_entry *proc_arg2_entry;
static struct proc_dir_entry *proc_op_entry;
static struct proc_dir_entry *proc_answer_entry;

int read_proc_arg1(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    return sprintf(buf, "%d", arg1);
}

int read_proc_arg2(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    return sprintf(buf, "%d", arg2);
}

int read_proc_op(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    return sprintf(buf, "%c", op);
}

int read_proc_answer(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    return sprintf(buf, "%d", answer);
}

int write_proc_arg1(struct file *file, const char *buf, int count, void *data)
{
    char tmp[30];
    if (count > MAX_PROC_SIZE)
        count = MAX_PROC_SIZE;
    if (copy_from_user(tmp, buf, count))
        return -EFAULT;
    sscanf(tmp, "%d", &arg1);

    return count;
}

int write_proc_arg2(struct file *file, const char *buf, int count, void *data)
{
    char tmp[30];
    if (count > MAX_PROC_SIZE)
        count = MAX_PROC_SIZE;
    if (copy_from_user(tmp, buf, count))
        return -EFAULT;
    sscanf(tmp, "%d", &arg2);

    return count;
}

int write_proc_op(struct file *file, const char *buf, int count, void *data)
{
    char tmp[30];
    if (count > MAX_PROC_SIZE)
        count = MAX_PROC_SIZE;
    if (copy_from_user(tmp, buf, count))
        return -EFAULT;
    sscanf(tmp, "%c", &op);

    return count;
}

int write_proc_answer(struct file *file, const char *buf, int count, void *data)
{
    char tmp[30];
    if (count > MAX_PROC_SIZE)
        count = MAX_PROC_SIZE;
    if (copy_from_user(tmp, buf, count))
        return -EFAULT;
    sscanf(tmp, "%d", &answer);

    return count;
}

int create_new_proc_entry()
{
    proc_arg1_entry = create_proc_entry("proc_arg1", 0644, NULL);
    proc_arg2_entry = create_proc_entry("proc_arg2", 0644, NULL);
    proc_op_entry = create_proc_entry("proc_op", 0644, NULL);
    proc_answer_entry = create_proc_entry("proc_answer", 0644, NULL);
    if (!(proc_arg1_entry && proc_arg2_entry && proc_op_entry && proc_answer_entry)) {
        printk(KERN_INFO "Error creating proc entries");
        return -ENOMEM;
    }
    proc_arg1_entry->read_proc = read_proc_arg1;
    proc_arg1_entry->write_proc = write_proc_arg1;
    proc_arg2_entry->read_proc = read_proc_arg2;
    proc_arg2_entry->write_proc = write_proc_arg2;
    proc_op_entry->read_proc = read_proc_op;
    proc_op_entry->write_proc = write_proc_op;
    proc_answer_entry->read_proc = read_proc_answer;
    proc_answer_entry->write_proc = write_proc_answer;
    printk(KERN_INFO "proc initialized");
    return 0;
}


#else

static ssize_t read_sysfs_arg1 (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d", arg1);
}

static ssize_t write_sysfs_arg1 (struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%du", &arg1);
    return count;
}

static struct kobj_attribute arg1_attribute =
    __ATTR(arg1, 0666, read_sysfs_arg1, write_sysfs_arg1);

static ssize_t read_sysfs_arg2 (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d", arg2);
}

static ssize_t write_sysfs_arg2 (struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
        sscanf(buf, "%du", &arg2);
        return count;
}

static struct kobj_attribute arg2_attribute =
    __ATTR(arg2, 0666, read_sysfs_arg2, write_sysfs_arg2);

static ssize_t read_sysfs_op (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%c ", op);
}

static ssize_t write_sysfs_op (struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%c", &op);
    return count;
}

static struct kobj_attribute op_attribute =
    __ATTR(op, 0666, read_sysfs_op, write_sysfs_op);

static ssize_t read_sysfs_answer (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    calculate();
    return sprintf(buf, "%d", answer);
}

static ssize_t write_sysfs_answer (struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%du", &answer);
    return count;
}

static struct kobj_attribute answer_attribute =
    __ATTR(answer, 0666, read_sysfs_answer, write_sysfs_answer);


static struct attribute *attrs[] = {
        &arg1_attribute.attr,
        &arg2_attribute.attr,
        &op_attribute.attr,
        &answer_attribute.attr,
        NULL,
};

static struct attribute_group attr_group = {
       .attrs = attrs,
};

static struct kobject *kobj;

int create_new_sysfs_entry()
{
    printk("in create_new_sysfs_entry\n");
    kobj = kobject_create_and_add("calc", kernel_kobj);
    if (!kobj) {
        printk("exception\n");
        return -ENOMEM;
    }

    if (sysfs_create_group(kobj, &attr_group)) {
        kobject_put(kobj);
    }
    return 0;
}


#endif // PROC

int init (void)
{
    printk(KERN_INFO "Inside init_module\n");
#ifdef PROC
    create_new_proc_entry();
#else
    create_new_sysfs_entry();
#endif // PROC
    return 0;
}

void cleanup(void)
{
    printk(KERN_INFO "Inside cleanup_module\n");
#ifdef PROC
    remove_proc_entry("calc_op1", NULL);
    remove_proc_entry("calc_op2", NULL);
    remove_proc_entry("calc_operator", NULL);
    remove_proc_entry("calc_ans", NULL);
#else
    //kobject_del(example_kobj);
#endif
}

MODULE_LICENSE("GPL");
module_init(init);
module_exit(cleanup);
