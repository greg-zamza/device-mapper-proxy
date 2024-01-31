#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>

#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/string.h> 
#include <linux/sysfs.h> 

struct dmp_data {
	struct dm_dev *dev;	/* bdev, с которым связан наш mapped device */
	unsigned int reads;
	unsigned int writes;
};

static int test_data = 10;

static ssize_t volumes_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{
	printk(KERN_CRIT "\n\nVERY IMPORTANT!!! VOLUMES_SHOW IN\n\n");
	int res = sysfs_emit(buf, "%d\n", test_data);
	printk(KERN_CRIT "\n\nsysfs_emit_len: %d\n\n", res);
	return res;
}

/* обработка bio запросов */
static int dmp_map(struct dm_target *ti, struct bio *bio)
{
	struct dmp_data *data = (struct dmp_data *) ti->private;
	
	bio_set_dev(bio, data->dev->bdev);
	
	bool write = (bio_data_dir(bio) == WRITE);
	bool read = (bio_data_dir(bio) == READ);
	
	if (write) {
		printk(KERN_DEBUG "dmp: write request\n");
		data->writes = data->writes + 1;
		printk(KERN_CRIT "dmp: writes: %u\n\n", data->writes);
	}

	if (read) {
		printk(KERN_DEBUG "dmp: read request\n");
		data->reads = data->reads + 1;
		printk(KERN_DEBUG "dmp: reads: %u\n\n", data->reads);
		/*запись числа в sysfs*/
	}

	return DM_MAPIO_REMAPPED;
}

/* конструктор */
static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	if (argc != 1) {
		ti->error = "Invalid argument count";
		return -EINVAL;
	}
	
	struct dmp_data *data;
	int ret;

	data = kmalloc(sizeof(*data), GFP_KERNEL);
	if(data == NULL)
	{
		ti->error = "Cannot allocate";
		return -ENOMEM;
	}       

	ret = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &data->dev);
	if (ret) {
		ti->error = "Device lookup failed";
		goto bad;
	}
		
	printk(KERN_CRIT " DMP_CTR: finish \n");
	
	data->writes = 0;
	data->reads = 0;
	ti->private = data;

	return 0;

bad:
	kfree(data);
	return ret;
}

/* деструктор */
static void dmp_dtr(struct dm_target *ti)
{
	struct dmp_data *data = (struct dmp_data *) ti->private;
	dm_put_device(ti, data->dev);
	/*kobject_put(data->kobj_dir);
	sysfs_remove_file(kernel_kobj, data->volumes_attr.attr);*/
	kfree(data);
}

static struct target_type target_dmp = {
	.name = "dmp",
	.version = {1, 0, 0},
	.module = THIS_MODULE,
	.ctr = dmp_ctr,
	.dtr = dmp_dtr,
	.map = dmp_map,
};


static int init_dmp(void)
{
	int result;
	result = dm_register_target(&target_dmp);
	if(result < 0)
		printk(KERN_CRIT "\n Error in registering target \n");

	static struct kobj_attribute volumes_attr = __ATTR_RO(volumes);
	struct kobject *dmp_object = kobject_create_and_add("dmp", kernel_kobj);
	
	int error = sysfs_create_file(dmp_object, &volumes_attr.attr);

	return error;
}

static void cleanup_dmp(void)
{
  dm_unregister_target(&target_dmp);
}

module_init(init_dmp);
module_exit(cleanup_dmp);

MODULE_AUTHOR("<grigorev@niuitmo.ru>");
MODULE_LICENSE("GPL");
