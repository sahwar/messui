// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/flstory.h"

READ8_MEMBER(flstory_state::flstory_mcu_status_r)
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
		((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
		((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

WRITE8_MEMBER(flstory_state::onna34ro_mcu_w)
{
	uint16_t score_adr = m_workram[0x29e] * 0x100 + m_workram[0x29d];

	switch (data)
	{
		case 0x0e:
			m_from_mcu = 0xff;
			break;
		case 0x01:
			m_from_mcu = 0x6a;
			break;
		case 0x40:
			if(score_adr >= 0xe000 && score_adr < 0xe800)
				m_from_mcu = m_workram[score_adr - 0xe000];         /* score l*/
			break;
		case 0x41:
			if(score_adr >= 0xe000 && score_adr < 0xe800)
				m_from_mcu = m_workram[(score_adr + 1) - 0xe000];       /* score m*/
			break;
		case 0x42:
			if(score_adr >= 0xe000 && score_adr < 0xe800)
				m_from_mcu = m_workram[(score_adr + 2) - 0xe000] & 0x0f;    /* score h*/
			break;
		default:
			m_from_mcu = 0x80;
	}
}

READ8_MEMBER(flstory_state::onna34ro_mcu_r)
{
	return m_from_mcu;
}

READ8_MEMBER(flstory_state::onna34ro_mcu_status_r)
{
	int res = 3;

	return res;
}


#define VICTNINE_MCU_SEED   (m_workram[0x685])

static const uint8_t victnine_mcu_data[0x100] =
{
	0x3e, 0x08, 0xdd, 0x29, 0xcb, 0x14, 0xfd, 0x29,
	0xcb, 0x15, 0xd9, 0x29, 0xd9, 0x30, 0x0d, 0xd9,
	0x19, 0xd9, 0xdd, 0x09, 0x30, 0x01, 0x24, 0xfd,
	0x19, 0x30, 0x01, 0x2c, 0x3d, 0x20, 0xe3, 0xc9,
	0x11, 0x14, 0x00, 0x19, 0x7e, 0x32, 0xed, 0xe4,
	0x2a, 0x52, 0xe5, 0x22, 0xe9, 0xe4, 0x2a, 0x54,
	0xe5, 0x22, 0xeb, 0xe4, 0x21, 0x2a, 0xe6, 0xfe,
	0x06, 0x38, 0x02, 0xcb, 0xc6, 0xcb, 0xce, 0xc9,
	0x06, 0x00, 0x3a, 0xaa, 0xe4, 0x07, 0x07, 0x07,
	0xb0, 0x47, 0x3a, 0xab, 0xe4, 0x07, 0x07, 0xb0,
	0x47, 0x3a, 0xac, 0xe4, 0xe6, 0x03, 0xb0, 0x21,
	0xe3, 0xe6, 0xc9, 0x38, 0xe1, 0x29, 0x07, 0xc9,
	0x23, 0x7e, 0x47, 0xe6, 0x1f, 0x32, 0x0c, 0xe6,
	0x78, 0xe6, 0xe0, 0x07, 0x07, 0x47, 0xe6, 0x03,
	0x28, 0x06, 0xcb, 0x7a, 0x28, 0x02, 0xc6, 0x02,
	0x32, 0x0a, 0xe6, 0x78, 0xe6, 0x80, 0xc9, 0x3a,
	0x21, 0x29, 0xe5, 0x7e, 0xe6, 0xf8, 0xf6, 0x01,
	0x77, 0x23, 0x3a, 0x0a, 0xe6, 0x77, 0x21, 0x08,
	0xe6, 0xcb, 0xc6, 0xcb, 0x8e, 0x3a, 0x2b, 0xe5,
	0x21, 0xff, 0xe5, 0xfe, 0x02, 0xc9, 0x1f, 0xc6,
	0x47, 0x3a, 0xaa, 0xe4, 0xa7, 0x21, 0x00, 0xe5,
	0x28, 0x03, 0x21, 0x1b, 0xe5, 0x70, 0x3a, 0xaa,
	0xe4, 0xee, 0x01, 0x32, 0xaa, 0xe4, 0x21, 0xb0,
	0xe4, 0x34, 0x23, 0x36, 0x00, 0xc9, 0x2b, 0xb2,
	0xaf, 0x77, 0x12, 0x23, 0x13, 0x3c, 0xfe, 0x09,
	0x20, 0xf7, 0x3e, 0x01, 0x32, 0xad, 0xe4, 0x21,
	0x48, 0xe5, 0xcb, 0xfe, 0xc9, 0x32, 0xe5, 0xaa,
	0x21, 0x00, 0x13, 0xe4, 0x47, 0x1b, 0xa1, 0xc9,
	0x00, 0x08, 0x04, 0x0c, 0x05, 0x0d, 0x06, 0x0e,
	0x22, 0x66, 0xaa, 0x22, 0x33, 0x01, 0x11, 0x88,
	0x06, 0x05, 0x03, 0x04, 0x08, 0x01, 0x03, 0x02,
	0x06, 0x07, 0x02, 0x03, 0x15, 0x17, 0x11, 0x13
};

WRITE8_MEMBER(flstory_state::victnine_mcu_w)
{
	uint8_t seed = VICTNINE_MCU_SEED;

	if (!seed && (data & 0x37) == 0x37)
	{
		m_from_mcu = 0xa6;
		logerror("mcu initialize (%02x)\n", data);
	}
	else
	{
		data += seed;

		if ((data & ~0x1f) == 0xa0)
		{
			m_mcu_select = data & 0x1f;
			//logerror("mcu select: 0x%02x\n", m_mcu_select);
		}
		else if (data < 0x20)
		{
			int offset = m_mcu_select * 8 + data;

			//logerror("mcu fetch: 0x%02x\n", offset);
			m_from_mcu = victnine_mcu_data[offset];
		}
		else if (data >= 0x38 && data <= 0x3a)
		{
			m_from_mcu = m_workram[0x691 - 0x38 + data];
		}
		else
		{
			//logerror("mcu: 0x%02x: unknown command\n", data);
		}
	}
}

READ8_MEMBER(flstory_state::victnine_mcu_r)
{
	//logerror("%04x: mcu read (0x%02x)\n", space.device().safe_pcbase(), m_from_mcu);

	return m_from_mcu - VICTNINE_MCU_SEED;
}

READ8_MEMBER(flstory_state::victnine_mcu_status_r)
{
	int res = 3;

	return res;
}
