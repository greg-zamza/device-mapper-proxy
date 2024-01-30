#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>

/*#include <linux/fs.h>*/

struct dmp_data {
	struct dm_dev *dev;	/* bdev, с которым связан наш mapped device */
};

/* обработка bio запросов */
static int dmp_map(struct dm_target *ti, struct bio *bio)
{
	printk(KERN_CRIT " DMP_MAP IN \n");

	struct dmp_data *data = (struct dmp_data *) ti->private;
	
	/*bio->bi_bdev = data->dev->bdev;*/
	bio_set_dev(bio, data->dev->bdev);

	/*printk(KERN_CRIT "bi_opf value: %u\n", bio->bi_opf);*/

	/*if((bio->bi_opf & WRITE) == WRITE)
		printk(KERN_DEBUG "\n dmp: bio is a write request.... \n");
	else
		printk(KERN_DEBUG "\n dmp: bio is a read request.... \n");*/
	
	/*submit_bio(bio);*/
	
	printk(KERN_CRIT " DMP: DMP_MAP exit (i hope there is no troubles...) \n");
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

	ti->private = data;
	printk(KERN_CRIT " DMP_CTR: finish \n");
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
  return 0;
}

static void cleanup_dmp(void)
{
  dm_unregister_target(&target_dmp);
}

module_init(init_dmp);
module_exit(cleanup_dmp);

MODULE_AUTHOR("<grigorev@niuitmo.ru>");
MODULE_LICENSE("GPL");
