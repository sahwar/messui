// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEC Serial Bus emulation

**********************************************************************/

/*

C64 SERIAL BUS


 Serial Bus Pinouts


    Pin Name    Description
     1  SRQ     Serial Service Request In
     2  GND     Ground
     3  ATN     Serial Attention In/Out
     4  CLK     Serial Clock In/Out
     5  DATA    Serial Data In/Out
     6  RESET   Serial Reset

    All signals are active low.


  SRQ: Serial Service Request In

 This signal is not used on the C64. On C128 it is replaced with Fast Serial
 Clock for the 1571 disk drive.


  ATN: Serial Attention In/Out

 Sending any byte with the ATN line low (sending under attention) causes it
 to be interpreted as a Bus Command for peripherals on the serial bus.

 When the C64 brings this signal LOW, all other devices start listening for
 it to transmit an address. The device addressed must respond in a preset
 period of time; otherwise, the C64 will assume that the device addressed is
 not on the bus, and will return an error in the STATUS word.

 Usually, the address byte will be followed by one to two commands for the
 device addressed, meaning the secondary address and channel on the peripheral.
 Such a command can be one of the following:

    20
    40
    60
    E0
    F0


  CLK: Serial Clock In/Out

  This signal is for timing the data sent on the serial bus. This signal is
  always generated by the active TALKER. RISING EDGE OF THE CLOCK means data
  bit is valid.


  DATA: Serial Data In/Out

  Data on the serial bus is transmitted bit by bit at a time on this line.


  RESET: Serial Reset

  You may disconnect this line to save your disk drive. The easiest way is to
  do that on the cable, thus avoiding any modifications on your peripherals.



  Serial Bus Timing

 ___
 CLK    |____|~~~~| Ts Bit Set-up time
    : Ts : Tv : Tv Bit Valid time



     |<--------- Byte sent under attention (to devices) ------------>|

 ___    ____                                                        _____ _____
 ATN       |________________________________________________________|
       :                                :
 ___    ______     ________ ___ ___ ___ ___ ___ ___ ___ ___         :
 CLK       : |_____|      |_| |_| |_| |_| |_| |_| |_| |_| |______________ _____
       :       :        :                 :         :
       : Tat : :Th: Tne :                             : Tf : Tr :
 ____   ________ : :  :___________________________________:____:
 DATA   ___|\\\\\\\\\\__:__|    |__||__||__||__||__||__||__||__|    |_________ _____
                  :     0   1   2   3   4   5   6   7      :
                  :    LSB                         MSB     :
              :     :                      :
              :     : Data Valid      Listener: Data Accepted
              : Listener READY-FOR-DATA




        END-OR-IDENTIFY HANDSHAKE (LAST BYTE IN MESSAGE)
 ___    _______________________________________________________________________
 ATN
 ___     ___ ___      ________________ ___ ___ ___ ___ ___ ___ ___ ___       __
 CLK    _| |_| |______|              |_| |_| |_| |_| |_| |_| |_| |_| |_______|_
           :      :          :                               :       :
           :Tf:Tbb:Th:Tye:Tei:Try:                               :Tf :Tfr:
 ____   __________:   :  :___:   :_______________________________________:   :_
 DATA   |__||__|  |______|   |___|   :                                   |___|_
     6   7        :  :   :   :   :                   :
        MSB       :  :   :   :   : Talker Sending            :
              :  :   :   : Listener READY-FOR-DATA      System
              :  :   : EOI-Timeout Handshake          Line Release
              :  : Listener READY-FOR-DATA
              : Talker Ready-To-Send




        TALK-ATTENTION TURN AROUND (TALKER AND LISTENER REVERSED)
 ___                 _________________________________________________________
 ATN    _____________|
             :
 ___     ___ ___     :   _____   ________ ___ ___ ___ ___ ___ ___ ___ ___
 CLK    _| |_| |_________|   |___|      |_| |_| |_| |_| |_| |_| |_| |_| |_____
           :     :   :   :   :      :                               :
           :Tf:Tr:Ttk:Tdc:Tda:Th:Try:                               :Tf :
 ____   __________:  :       :   :  :_______________________________________:
 DATA   |__||__|  |_________________|   :|__||__||__||__||__||__||__||__|   |_
     6   7       :   :   :   :  :   : 0   1   2   3   4   5   6   7
        MSB      :   :   :   :  :   :LSB                         MSB
             :   :   :   :  :   :
             :   :   :   :  :   : Data Valid
             :   :   :   :  : Listener READY-FOR-DATA
             :   :   :   : Talker Ready-To-Send
             :   :   : Device acknowledges it's now TALKER.
             :   : Becomes LISTENER, Clock = High, Data = Low
             : Talker Ready-To-Send




 ___    _____________________________________________________________________
 ATN
 ___        _________ ___ ___ ___ ___ ___ ___ ___ ___       ________ ___ ___
 CLK    ____|       |_| |_| |_| |_| |_| |_| |_| |_| |_______|      |_| |_| |_
        :       :                   :       :      :
        :Th :Tne:                               :Tf :Tbb:Th:Tne:
 ____       :   :___:___________________________________:      :_____________
 DATA   ________|   :|__||__||__||__||__||__||__||__|   |______|
        :   :   : 0   1   2   3   4   5   6   7     :
        :   :   :LSB                         MSB    :
        :   :   :                   :
        :   :   : TALKER SENDING        Listener: Data Accepted
        :   : LISTENER READY-FOR-DATA
        : TALKER READY-TO-SEND



  Serial Bus Timing


    Description         Symbol   Min     Typ     Max

    ATN Response (required) 1)   Tat      -   - 1000us
    Listener Hold-Off        Th   0   -  oo
    Non-EOI Response to RFD 2)   Tne      -  40us   200us
    Bit Set-Up Talker  4)        Ts  20us    70us     -
    Data Valid           Tv  20us    20us     -
    Frame Handshake  3)      Tf   0   20    1000us
    Frame to Release of ATN      Tr  20us     -   -
    Between Bytes Time       Tbb    100us     -   -
    EOI Response Time        Tye    200us   250us     -
    EOI Response Hold Time  5)   Tei     60us     -   -
    Talker Response Limit        Try      0  30us    60us
    Byte-Acknowledge  4)         Tpr     20us    30us     -
    Talk-Attention Release       Ttk     20us    30us   100us
    Talk-Attention Acknowledge   Tdc      0   -   -
    Talk-Attention Ack. Hold     Tda     80us     -   -
    EOI Acknowledge          Tfr     60us     -   -


   Notes:
    1)  If maximum time exceeded, device not present error.
    2)  If maximum time exceeded, EOI response required.
    3)  If maximum time exceeded, frame error.
    4)  Tv and Tpr minimum must be 60us for external device to be a talker.
    5)  Tei minimum must be 80us for external device to be a listener.
*/

#include "emu.h"
#include "cbmiec.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


static const char *const SIGNAL_NAME[] = { "SRQ", "ATN", "CLK", "DATA", "RESET" };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM_IEC = device_creator<cbm_iec_device>;
const device_type CBM_IEC_SLOT = device_creator<cbm_iec_slot_device>;



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cbm_iec_interface - constructor
//-------------------------------------------------

device_cbm_iec_interface::device_cbm_iec_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device), m_next(nullptr), m_bus(nullptr), m_slot(nullptr)
{
}


//-------------------------------------------------
//  ~device_cbm_iec_interface - destructor
//-------------------------------------------------

device_cbm_iec_interface::~device_cbm_iec_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm_iec_slot_device - constructor
//-------------------------------------------------

cbm_iec_slot_device::cbm_iec_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, CBM_IEC_SLOT, "CBM IEC slot", tag, owner, clock, "cbm_iec_slot", __FILE__),
		device_slot_interface(mconfig, *this), m_address(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm_iec_slot_device::device_start()
{
	cbm_iec_device* bus = nullptr;

	for (device_t *device = owner(); device != nullptr; device = device->owner())
	{
		bus = device->subdevice<cbm_iec_device>(CBM_IEC_TAG);
		if (bus != nullptr) break;
	}

	assert(bus);

	device_cbm_iec_interface *dev = dynamic_cast<device_cbm_iec_interface *>(get_card_device());
	if (dev) bus->add_device(this, get_card_device());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm_iec_device - constructor
//-------------------------------------------------

cbm_iec_device::cbm_iec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CBM_IEC, "CBM IEC bus", tag, owner, clock, "cbm_iec", __FILE__),
		m_write_srq(*this),
		m_write_atn(*this),
		m_write_clk(*this),
		m_write_data(*this),
		m_write_reset(*this)
{
	for (auto & elem : m_line)
	{
		elem = 1;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm_iec_device::device_start()
{
	// resolve callbacks
	m_write_srq.resolve_safe();
	m_write_atn.resolve_safe();
	m_write_clk.resolve_safe();
	m_write_data.resolve_safe();
	m_write_reset.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm_iec_device::device_reset()
{
	reset_w(0);
	reset_w(1);
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void cbm_iec_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_device -
//-------------------------------------------------

void cbm_iec_device::add_device(cbm_iec_slot_device *slot, device_t *target)
{
	auto entry = global_alloc(daisy_entry(target));

	entry->m_interface->m_slot = slot;
	entry->m_interface->m_bus = this;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

cbm_iec_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(nullptr),
		m_device(device),
		m_interface(nullptr)
{
	for (auto & elem : m_line)
	{
		elem = 1;
	}

	device->interface(m_interface);
}


//-------------------------------------------------
//  set_signal -
//-------------------------------------------------

void cbm_iec_device::set_signal(device_t *device, int signal, int state)
{
	bool changed = false;

	if (device == this)
	{
		if (m_line[signal] != state)
		{
			if (LOG) logerror("CBM IEC: '%s' %s %u\n", tag(), SIGNAL_NAME[signal], state);
			m_line[signal] = state;
			changed = true;
		}
	}
	else
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!strcmp(entry->m_device->tag(), device->tag()))
			{
				if (entry->m_line[signal] != state)
				{
					if (LOG) logerror("CBM IEC: '%s' %s %u\n", device->tag(), SIGNAL_NAME[signal], state);
					entry->m_line[signal] = state;
					changed = true;
				}
			}

			entry = entry->next();
		}
	}

	if (changed)
	{
		switch (signal)
		{
		case SRQ:   m_write_srq(state);  break;
		case ATN:   m_write_atn(state);  break;
		case CLK:   m_write_clk(state);  break;
		case DATA:  m_write_data(state); break;
		case RESET: m_write_reset(state);break;
		}

		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			switch (signal)
			{
			case SRQ:
				entry->m_interface->cbm_iec_srq(state);
				break;

			case ATN:
				entry->m_interface->cbm_iec_atn(state);
				break;

			case CLK:
				entry->m_interface->cbm_iec_clk(state);
				break;

			case DATA:
				entry->m_interface->cbm_iec_data(state);
				break;

			case RESET:
				entry->m_interface->cbm_iec_reset(state);
				break;
			}

			entry = entry->next();
		}

		if (LOG) logerror("CBM IEC: SRQ %u ATN %u CLK %u DATA %u RESET %u\n",
			get_signal(SRQ), get_signal(ATN), get_signal(CLK), get_signal(DATA), get_signal(RESET));
	}
}


//-------------------------------------------------
//  get_signal -
//-------------------------------------------------

int cbm_iec_device::get_signal(int signal)
{
	int state = m_line[signal];

	if (state)
	{
		daisy_entry *entry = m_device_list.first();

		while (entry)
		{
			if (!entry->m_line[signal])
			{
				state = 0;
				break;
			}

			entry = entry->next();
		}
	}

	return state;
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm_iec_devices )
//-------------------------------------------------

// slot devices
#include "c1541.h"
#include "c1571.h"
#include "c1581.h"
#include "c64_nl10.h"
#include "cmdhd.h"
#include "diag264_lb_iec.h"
#include "fd2000.h"
#include "interpod.h"
#include "serialbox.h"
#include "vic1515.h"
#include "vic1520.h"
#include "c1526.h"

SLOT_INTERFACE_START( cbm_iec_devices )
	SLOT_INTERFACE("c1540", C1540)
	SLOT_INTERFACE("c1541", C1541)
	SLOT_INTERFACE("c1541c", C1541C)
	SLOT_INTERFACE("c1541ii", C1541II)
	SLOT_INTERFACE("fsd1", FSD1)
	SLOT_INTERFACE("fsd2", FSD2)
	SLOT_INTERFACE("csd1", CSD1)
	SLOT_INTERFACE("c1541dd", C1541_DOLPHIN_DOS)
	SLOT_INTERFACE("c1541pd", C1541_PROFESSIONAL_DOS_V1)
	SLOT_INTERFACE("c1541pdc", C1541_PROLOGIC_DOS_CLASSIC)
	SLOT_INTERFACE("c1570", C1570)
	SLOT_INTERFACE("c1571", C1571)
	SLOT_INTERFACE("c1581", C1581)
	SLOT_INTERFACE("indusgt", INDUS_GT)
	SLOT_INTERFACE("cmdhd", CMD_HD)
	SLOT_INTERFACE("fd2000", FD2000)
	SLOT_INTERFACE("fd4000", FD4000)
	SLOT_INTERFACE("interpod", INTERPOD)
	SLOT_INTERFACE("minichief", MINI_CHIEF)
	SLOT_INTERFACE("serialbox", SERIAL_BOX)
	SLOT_INTERFACE("diag264", DIAG264_SERIAL_LOOPBACK)
	SLOT_INTERFACE("nl10", C64_NL10_INTERFACE)
	SLOT_INTERFACE("vic1515", VIC1515)
	SLOT_INTERFACE("vic1520", VIC1520)
	SLOT_INTERFACE("c1526", C1526)
	SLOT_INTERFACE("mps802", MPS802)
SLOT_INTERFACE_END
