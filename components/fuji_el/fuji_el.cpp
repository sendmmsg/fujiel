#include "fuji_el.h"
#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Fujitsu.h"

namespace esphome {
namespace fuji_el {

static const char *const TAG = "fuji_el.climate";

IRFujitsuAC ac(kIrLed);

void FujiElClimate::setup() {
  ESP_LOGV(TAG, "setup");
  ac.begin();
  ac.setModel(ARREB1E);
  ac.setSwing(kFujitsuAcSwingOff);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);  // 24C
  ac.setCmd(kFujitsuAcCmdTurnOn);
}

void FujitsuGeneralClimate::transmit_state() {
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    this->transmit_off_();
    return;
  }

  ESP_LOGV(TAG, "Transmit state");

    ac.setCmd(kFujitsuAcCmdTurnOff);
  //uint8_t remote_state[FUJI_EL_STATE_MESSAGE_LENGTH] = {0};

  // Common message header
  //remote_state[0] = FUJI_EL_COMMON_BYTE0;
  //remote_state[1] = FUJI_EL_COMMON_BYTE1;
  //remote_state[2] = FUJI_EL_COMMON_BYTE2;
  //remote_state[3] = FUJI_EL_COMMON_BYTE3;
  //remote_state[4] = FUJI_EL_COMMON_BYTE4;
  //remote_state[5] = FUJI_EL_MESSAGE_TYPE_STATE;
  //remote_state[6] = FUJI_EL_STATE_HEADER_BYTE0;
  //remote_state[7] = FUJI_EL_STATE_HEADER_BYTE1;

  // unknown, does not appear to change with any remote settings
  //remote_state[14] = FUJI_EL_STATE_FOOTER_BYTE0;

  // Set temperature
  uint8_t temperature_clamped =
      (uint8_t) roundf(clamp<float>(this->target_temperature, FUJI_EL_TEMP_MIN, FUJI_EL_TEMP_MAX));
  uint8_t temperature_offset = temperature_clamped - FUJI_EL_TEMP_MIN;
  //SET_NIBBLE(remote_state, FUJI_EL_TEMPERATURE_NIBBLE, temperature_offset);
  ac.setTemp(temperature_clamped);

  // Set power on
  if (!this->power_) {
    //SET_NIBBLE(remote_state, FUJI_EL_POWER_ON_NIBBLE, FUJI_EL_POWER_ON);
    ac.setCmd(kFujitsuAcCmdTurnOn);
  }


  // Set mode
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
	    ac.setMode(kFujitsuAcModeCool);
      //SET_NIBBLE(remote_state, FUJI_EL_MODE_NIBBLE, FUJI_EL_MODE_COOL);
      break;
    case climate::CLIMATE_MODE_HEAT:
	    ac.setMode(kFujitsuAcModeHeat);
      //SET_NIBBLE(remote_state, FUJI_EL_MODE_NIBBLE, FUJI_EL_MODE_HEAT);
      break;
    case climate::CLIMATE_MODE_DRY:
	    ac.setMode(kFujitsuAcModeDry);
      //SET_NIBBLE(remote_state, FUJI_EL_MODE_NIBBLE, FUJI_EL_MODE_DRY);
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
	    ac.setMode(kFujitsuAcModeFan);
      //SET_NIBBLE(remote_state, FUJI_EL_MODE_NIBBLE, FUJI_EL_MODE_FAN);
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
    default:
	    ac.setMode(kFujitsuAcModeAuto);
      //SET_NIBBLE(remote_state, FUJI_EL_MODE_NIBBLE, FUJI_EL_MODE_AUTO);
      break;
      // TODO: CLIMATE_MODE_10C is missing from esphome
  }

  // Set fan
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_HIGH:
      ac.setFanSpeed(kFujitsuAcFanHigh);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_HIGH);
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      ac.setFanSpeed(kFujitsuAcFanMed);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_MEDIUM);
      break;
    case climate::CLIMATE_FAN_LOW:
      ac.setFanSpeed(kFujitsuAcFanLow);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_LOW);
      break;
    case climate::CLIMATE_FAN_QUIET:
      ac.setFanSpeed(kFujitsuAcFanQuiet);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_SILENT);
      break;
    case climate::CLIMATE_FAN_AUTO:
    default:
      ac.setFanSpeed(kFujitsuAcFanAuto);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_AUTO);
      break;
  }

  // Set swing
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
	    ac.setSwing(kFujitsuAcSwingVert);
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_VERTICAL);
      break;
    case climate::CLIMATE_SWING_HORIZONTAL:
	    ac.setSwing(kFujitsuAcSwingHoriz);
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_HORIZONTAL);
      break;
    case climate::CLIMATE_SWING_BOTH:
	    ac.setSwing(kFujitsuAcSwingBoth);
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_BOTH);
      break;
    case climate::CLIMATE_SWING_OFF:
    default:
	    ac.setSwing(kFujitsuAcSwingOff);
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_NONE);
      break;
  }

  ac.send();
  // TODO: missing support for outdoor unit low noise
  // remote_state[14] = (byte) remote_state[14] | FUJI_EL_OUTDOOR_UNIT_LOW_NOISE_BYTE14;
  //remote_state[FUJI_EL_STATE_MESSAGE_LENGTH - 1] = this->checksum_state_(remote_state);
  //this->transmit_(remote_state, FUJI_EL_STATE_MESSAGE_LENGTH);
  this->power_ = true;
}

void FujitsuGeneralClimate::transmit_off_() {
  ESP_LOGV(TAG, "Transmit off");
  ac.setCmd(kFujitsuAcCmdTurnOff);

//  uint8_t remote_state[FUJI_EL_UTIL_MESSAGE_LENGTH] = {0};

 // remote_state[0] = FUJI_EL_COMMON_BYTE0;
  //remote_state[1] = FUJI_EL_COMMON_BYTE1;
  //remote_state[2] = FUJI_EL_COMMON_BYTE2;
  //remote_state[3] = FUJI_EL_COMMON_BYTE3;
  //remote_state[4] = FUJI_EL_COMMON_BYTE4;
  //remote_state[5] = FUJI_EL_MESSAGE_TYPE_OFF;
  //remote_state[6] = this->checksum_util_(remote_state);
//
  //this->transmit_(remote_state, FUJI_EL_UTIL_MESSAGE_LENGTH);
  ac.send();
  this->power_ = false;
}

//void FujitsuGeneralClimate::transmit_(uint8_t const *message, uint8_t length) {
//  ESP_LOGV(TAG, "Transmit message length %d", length);
//
//  auto transmit = this->transmitter_->transmit();
//  auto *data = transmit.get_data();
//
//  data->set_carrier_frequency(FUJI_EL_CARRIER_FREQUENCY);
//
//  // Header
//  data->mark(FUJI_EL_HEADER_MARK);
//  data->space(FUJI_EL_HEADER_SPACE);
//
//  // Data
//  for (uint8_t i = 0; i < length; ++i) {
//    const uint8_t byte = message[i];
//    for (uint8_t mask = 0b00000001; mask > 0; mask <<= 1) {  // write from right to left
//      data->mark(FUJI_EL_BIT_MARK);
//      bool bit = byte & mask;
//      data->space(bit ? FUJI_EL_ONE_SPACE : FUJI_EL_ZERO_SPACE);
//    }
//  }
//
//  // Footer
//  data->mark(FUJI_EL_TRL_MARK);
//  data->space(FUJI_EL_TRL_SPACE);
//
//  transmit.perform();
//}


}  // namespace fuji_general
}  // namespace esphome
