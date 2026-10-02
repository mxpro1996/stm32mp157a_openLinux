/*
 * MAXIO PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * 
 * 
 */
#include <config.h>
#include <common.h>
#include <phy.h>

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

// #define BIT(x)		(1 << (x))

#define MAXIO_PAGE_SELECT			    0x1f
#define MIIM_MAXIO_MAE0621A_INER				0x12
#define MIIM_MAXIO_MAE0621A_INER_LINK_STATUS	BIT(4)
#define MIIM_MAXIO_MAE0621A_INSR				0x1d
#define MIIM_MAXIO_MAE0621A_TX_DELAY			(BIT(6)|BIT(7))
#define MIIM_MAXIO_MAE0621A_RX_DELAY			(BIT(4)|BIT(5))
#define MAXIO_MAE0621A_CLK_MODE_REG          0x02
#define MAXIO_MAE0621A_WORK_STATUS_REG       0x1d

static int maxio_mae0621a_clk_init(struct phy_device *phydev)
{
    u32 workmode,clkmode,oldpage;

    oldpage = phy_read(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT);

     //soft reset
    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0x0);
    phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET | phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR));

    //get workmode
    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0xa43);
    workmode = phy_read(phydev, MDIO_DEVAD_NONE, MAXIO_MAE0621A_WORK_STATUS_REG);

    //get clomode
    phy_write( phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0xd92 );
    clkmode = phy_read( phydev, MDIO_DEVAD_NONE ,MAXIO_MAE0621A_CLK_MODE_REG );

    //abnormal
    if (0 == (workmode&BIT(5))) {
        if (0 == (clkmode&BIT(8))) {
           //oscillator
           phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_MAE0621A_CLK_MODE_REG, clkmode | BIT(8));
           // printf("****maxio_mae0621a_probe**clkmode**0x210a: 0x%x\n", phydev->phy_id);
        } else {
           //crystal
           // printf("****maxio_mae0621a_probe**clkmode**0x200a: 0x%x\n", phydev->phy_id);
           phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_MAE0621A_CLK_MODE_REG, clkmode &(~ BIT(8)));
        }
    }
    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0);

    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, oldpage);

	return 0;
}

static int maxio_mae0621a_probe(struct phy_device *phydev)
{
	return maxio_mae0621a_clk_init(phydev);
}

static int maxio_mae0621a_config(struct phy_device *phydev)
{

    maxio_mae0621a_clk_init(phydev);

    //disable EEE
    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT ,0);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xd, MDIO_MMD_AN);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xe, MDIO_AN_EEE_ADV);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4000 | MDIO_MMD_AN);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0);
    udelay(1000);

	/* Set green LED for Link, yellow LED for Active */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MAXIO_PAGE_SELECT, 0xd04);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x617f);
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MAXIO_PAGE_SELECT, 0x0);

	genphy_config_aneg(phydev);

	return 0;
}
static int maxio_mae0621a_startup(struct phy_device *phydev)
{
    genphy_startup(phydev);
	return 0;
}
static int maxio_mae0621a_shutdown(struct phy_device *phydev)
{
    genphy_shutdown(phydev);
    phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT ,0);

	return 0;
}

/* Support for MAE0621A PHY */
static struct phy_driver MAXIO_MAE0621A_driver = {
	.name = "MAXIO MAE0621A",
	.uid = 0x7b744411,
	.mask = 0xFFFFFFFF,
	.probe = &maxio_mae0621a_probe,
	.features = PHY_GBIT_FEATURES,
	.config = &maxio_mae0621a_config,
	.startup = &maxio_mae0621a_startup,
	.shutdown = &maxio_mae0621a_shutdown,
};


int phy_maxio_init(void)
{
	phy_register(&MAXIO_MAE0621A_driver);

	return 0;
}
