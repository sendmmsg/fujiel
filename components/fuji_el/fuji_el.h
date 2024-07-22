#pragma once

#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/climate_ir/climate_ir.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Fujitsu.h"


namespace esphome {
namespace fuji_el {

const uint16_t kIrLed = 25;

const uint8_t FUJI_EL_TEMP_MIN = 16;  // Celsius // TODO 16 for heating, 18 for cooling, unsupported in ESPH
const uint8_t FUJI_EL_TEMP_MAX = 30;  // Celsius


class FujiElClimate : public climate_ir::ClimateIR {
 public:
  FujiElClimate()
      : ClimateIR(FUJI_EL_TEMP_MIN, FUJI_EL_TEMP_MAX, 1.0f, true, true,
                  {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                   climate::CLIMATE_FAN_HIGH, climate::CLIMATE_FAN_QUIET},
                  {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL, climate::CLIMATE_SWING_HORIZONTAL,
                   climate::CLIMATE_SWING_BOTH}) {}

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override;
  /// Transmit via IR power off command.
  void transmit_off_();

  /// Parse incoming message
//bool on_receive(remote_base::RemoteReceiveData data) override;

  /// Transmit message as IR pulses
//void transmit_(uint8_t const *message, uint8_t length);

  /// Calculate checksum for a state message
//uint8_t checksum_state_(uint8_t const *message);

  /// Calculate cecksum for a util message
//uint8_t checksum_util_(uint8_t const *message);

  // true if currently on - fujitsus transmit an on flag on when the remote moves from off to on
  bool power_{false};
};

}  // namespace fujitsu_general
}  // namespace esphome
