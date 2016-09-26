// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SNUG HSGPL card emulation.
    See hsgpl.c for documentation

    Original code by Raphael Nabet, 2003.

    Michael Zapf
    October 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __HSGPL__
#define __HSGPL__

#include "emu.h"
#include "peribox.h"
#include "machine/at29x.h"
#include "machine/ram.h"

extern const device_type TI99_HSGPL;

class snug_high_speed_gpl_device : public ti_expansion_card_device
{
public:
	snug_high_speed_gpl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

protected:
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual void device_stop(void) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<at29c040a_device> m_dsr_eeprom;
	required_device<at29c040a_device> m_rom6_eeprom;
	required_device<at29c040a_device> m_grom_a_eeprom;
	required_device<at29c040a_device> m_grom_b_eeprom;

	required_device<ram_device>      m_ram6_memory;
	required_device<ram_device>      m_gram_memory;

	void            dsrspace_readz(address_space& space, offs_t offset, UINT8* value, UINT8 mem_mask);
	void            cartspace_readz(address_space& space, offs_t offset, UINT8* value, UINT8 mem_mask);
	void            grom_readz(address_space& space, offs_t offset, UINT8* value, UINT8 mem_mask);

	void            cartspace_write(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask);
	void            grom_write(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask);

	bool            m_dsr_enabled;
	bool            m_gram_enabled;
	bool            m_bank_inhibit;
	int             m_dsr_page;
	bool            m_card_enabled;
	bool            m_write_enabled;
	bool            m_supercart_enabled;
	bool            m_led_on;
	bool            m_mbx_enabled;
	bool            m_ram_enabled;
	bool            m_flash_mode;

	int             m_current_grom_port;
	int             m_current_bank;

	int             m_module_bank;

	// GROM emulation
	bool            m_waddr_LSB;
	bool            m_raddr_LSB;
	int             m_grom_address;
};

#endif
