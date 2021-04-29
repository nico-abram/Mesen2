#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Utilities/Serializer.h"

class OekaKidsTablet : public BaseControlDevice
{
private:
	bool _strobe = false;
	bool _shift = false;
	uint32_t _stateBuffer = 0;

protected:
	enum Buttons { Click, Touch };
	bool HasCoordinates() override { return true; }

	string GetKeyNames() override
	{
		return "CT";
	}

	void InternalSetStateFromInput() override
	{
		if(_emu->GetSettings()->IsInputEnabled()) {
			MousePosition pos = KeyManager::GetMousePosition();
			SetPressedState(Buttons::Click, KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));
			SetPressedState(Buttons::Touch, pos.Y >= 48 || KeyManager::IsMouseButtonPressed(MouseButton::LeftButton));
			SetCoordinates(pos);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		s.Stream(_strobe, _shift, _stateBuffer);
	}

public:
	OekaKidsTablet(Emulator* emu) : BaseControlDevice(emu, ControllerType::OekaKidsTablet, BaseControlDevice::ExpDevicePort)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			if(_strobe) {
				if(_shift) {
					return (_stateBuffer & 0x40000) ? 0x00 : 0x08;
				} else {
					return 0x04;
				}
			} else {
				return 0x00;
			}
		}

		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_strobe = (value & 0x01) == 0x01;
		bool shift = ((value >> 1) & 0x01) == 0x01;

		if(_strobe) {
			if(!_shift && shift) {
				_stateBuffer <<= 1;
			}
			_shift = shift;
		} else {
			MousePosition pos = GetCoordinates();

			uint8_t xPosition = (uint8_t)((double)std::max(0, pos.X + 8) / 256.0 * 240);
			uint8_t yPosition = (uint8_t)((double)std::max(0, pos.Y - 14) / 240.0 * 256);

			_stateBuffer = (xPosition << 10) | (yPosition << 2) | (IsPressed(OekaKidsTablet::Buttons::Touch) ? 0x02 : 0x00) | (IsPressed(OekaKidsTablet::Buttons::Click) ? 0x01 : 0x00);
		}
	}
};