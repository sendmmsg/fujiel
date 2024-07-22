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
  ESP_LOGI(TAG, "setup");
  delay(500);
  ESP_LOGI(TAG, "->begin");
  ac.begin();
  ESP_LOGI(TAG, "->ARREB1E");
  ac.setModel(ARREB1E);
  ESP_LOGI(TAG, "->Swing ON");
  ac.setSwing(kFujitsuAcSwingOn);
  ESP_LOGI(TAG, "->Mode COOL");
  ac.setMode(kFujitsuAcModeCool);
  ESP_LOGI(TAG, "->Fan High");
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ESP_LOGI(TAG, "->Temp 24");
  ac.setTemp(24);  // 24C
  ESP_LOGI(TAG, "->Turn On");
  ac.setCmd(kFujitsuAcCmdTurnOn);
  ac.send();
  delay(500);
}

void FujiElClimate::transmit_state() {
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    this->transmit_off_();
    return;
  }

  ESP_LOGI(TAG, "Transmit state");

  ESP_LOGI(TAG, "->Turn Off");
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
  ESP_LOGI(TAG, "->Temp %d", temperature_clamped);

  // Set power on
  if (!this->power_) {
    ESP_LOGI(TAG, "->Turn On");
    ac.setCmd(kFujitsuAcCmdTurnOn);
  }


  // Set mode
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
	    ac.setMode(kFujitsuAcModeCool);
    ESP_LOGI(TAG, "->Mode Cool");
      break;
    case climate::CLIMATE_MODE_HEAT:
	    ac.setMode(kFujitsuAcModeHeat);
    ESP_LOGI(TAG, "->Mode Heat");
      break;
    case climate::CLIMATE_MODE_DRY:
	    ac.setMode(kFujitsuAcModeDry);
    ESP_LOGI(TAG, "->Mode Dry");
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
	    ac.setMode(kFujitsuAcModeFan);
    ESP_LOGI(TAG, "->Mode Fan");
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
    default:
	    ac.setMode(kFujitsuAcModeAuto);
    ESP_LOGI(TAG, "->Mode Auto");
      break;
      // TODO: CLIMATE_MODE_10C is missing from esphome
  }

  // Set fan
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_HIGH:
      ac.setFanSpeed(kFujitsuAcFanHigh);
    ESP_LOGI(TAG, "->Fan High");
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      ac.setFanSpeed(kFujitsuAcFanMed);
    ESP_LOGI(TAG, "->Fan Medium");
      break;
    case climate::CLIMATE_FAN_LOW:
      ac.setFanSpeed(kFujitsuAcFanLow);
    ESP_LOGI(TAG, "->Fan Low");
      break;
    case climate::CLIMATE_FAN_QUIET:
      ac.setFanSpeed(kFujitsuAcFanQuiet);
    ESP_LOGI(TAG, "->Fan Quiet");
      break;
    case climate::CLIMATE_FAN_AUTO:
    default:
    ESP_LOGI(TAG, "->Fan Auto");
      ac.setFanSpeed(kFujitsuAcFanAuto);
      //SET_NIBBLE(remote_state, FUJI_EL_FAN_NIBBLE, FUJI_EL_FAN_AUTO);
      break;
  }

  // Set swing
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
	    ac.setSwing(kFujitsuAcSwingVert);
    ESP_LOGI(TAG, "->Swing Vert");
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_VERTICAL);
      break;
    case climate::CLIMATE_SWING_HORIZONTAL:
	    ac.setSwing(kFujitsuAcSwingHoriz);
    ESP_LOGI(TAG, "->Swing Horiz");
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_HORIZONTAL);
      break;
    case climate::CLIMATE_SWING_BOTH:
	    ac.setSwing(kFujitsuAcSwingBoth);
    ESP_LOGI(TAG, "->Swing Both");
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_BOTH);
      break;
    case climate::CLIMATE_SWING_OFF:
    default:
	    ac.setSwing(kFujitsuAcSwingOff);
    ESP_LOGI(TAG, "->Swing Off");
      //SET_NIBBLE(remote_state, FUJI_EL_SWING_NIBBLE, FUJI_EL_SWING_NONE);
      break;
  }

    ESP_LOGI(TAG, "Sending");
  ac.send();
  this->power_ = true;
}

void FujiElClimate::transmit_off_() {
  ESP_LOGV(TAG, "Transmit off");
  ac.setCmd(kFujitsuAcCmdTurnOff);

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
