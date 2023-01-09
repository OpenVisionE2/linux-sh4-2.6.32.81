/* ============================================================================
 * This is a driver for the Altima AC101L PHY controller.
 *  (based on STE10XP driver)
 *
 *	Audioniek
 *
 * ----------------------------------------------------------------------------
 * Changelog:
 *	first release for Cuberevo
 *	second release for Homecast HS8100 series
 *
 * ===========================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/phy.h>

#undef PHYDEBUG
#define DEFAULT_PHY_ID       0
#define RESOURCE_NAME        "ac101l"

#define MII_XCIIS            0x11   /* Configuration Info IRQ & Status Reg*/
#define MII_XIE              0x12   /* Interrupt Enable Register*/
#define MII_XIE_DEFAULT_MASK 0x0070 /* ANE complete, Remote Fault, Link Down */

/* AC101L phy identifier values */
#define AC101L_PHY_ID		 00225521  // 0x00061c51

static int ac101l_config_init(struct phy_device *phydev)
{
	int value, err;

	printk("%s >\n", __func__);

	/* Software Reset PHY */
	value = phy_read(phydev, MII_BMCR);
	if (value < 0)
	{
		return value;
	}

	value |= BMCR_RESET;
	err = phy_write(phydev, MII_BMCR, value);
	if (err < 0)
	{
		return err;
	}

	do
	{
		value = phy_read(phydev, MII_BMCR);
	} while (value & BMCR_RESET);

	return 0;
}

/* AC101L does not have interrupt */
static int ac101l_config_intr(struct phy_device *phydev)
{
	int err, value;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
	{
		/* Enable all AC101L interrupts (PR12) */
		err = phy_write(phydev, MII_XIE, MII_XIE_DEFAULT_MASK);
		/* clear any pending interrupts */
		if (err == 0)
		{
			value = phy_read(phydev, MII_XCIIS);
			if (value < 0)
			{
				err = value;
			}
		}
	}
	else
	{
		err = phy_write(phydev, MII_XIE, 0);
	}
	return err;
}

static int ac101l_ack_interrupt(struct phy_device *phydev)
{
	int err = phy_read(phydev, MII_XCIIS);

	if (err < 0)
	{
		return err;
	}

	return 0;
}

static struct phy_driver ac101l_pdriver =
{
	.phy_id         = AC101L_PHY_ID,
	.phy_id_mask    = 0xfffffffe,
	.name           = "AC101L",
	.features       = PHY_BASIC_FEATURES,
	.flags          = 0,  // PHY_HAS_INTERRUPT,
	.config_init    = ac101l_config_init,
	.config_aneg    = genphy_config_aneg,
	.read_status    = genphy_read_status,
	.ack_interrupt  = ac101l_ack_interrupt,
	.config_intr    = ac101l_config_intr,
	.suspend        = genphy_suspend,
	.resume         = genphy_resume,
	.driver         =
	{
		.owner      = THIS_MODULE
	}
};

static int __init ac101l_init(void)
{
	int retval;

	return phy_driver_register(&ac101l_pdriver);
}

static void __exit ac101l_exit(void)
{
	phy_driver_unregister(&ac101l_pdriver);
}

module_init(ac101l_init);
module_exit(ac101l_exit);

MODULE_DESCRIPTION("Altima AC101L PHY driver");
MODULE_LICENSE("GPL");
//vim:ts=4
