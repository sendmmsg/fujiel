#define SEND_FUJITSU_AC 1


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

  if (this->sensor_) {
    this->sensor_->add_on_state_callback([this](float state) {
      this->current_temperature = state;
      // current temperature changed, publish state
      this->publish_state();
    });
  	ESP_LOGI(TAG, "current_temperature: this->sensor_->state");
    this->current_temperature = this->sensor_->state;
  } else{
  	ESP_LOGI(TAG, "current_temperature: NAN");
    this->current_temperature = NAN;
  }
  // restore set points
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    // restore from defaults
    this->mode = climate::CLIMATE_MODE_OFF;
    // initialize target temperature to some value so that it's not NAN
    this->target_temperature =
        roundf(clamp(this->current_temperature, this->minimum_temperature_, this->maximum_temperature_));
    this->fan_mode = climate::CLIMATE_FAN_AUTO;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
    this->preset = climate::CLIMATE_PRESET_NONE;
  }
  // Never send nan to HA
  if (std::isnan(this->target_temperature))
    this->target_temperature = 24;



  delay(500);
  ESP_LOGI(TAG, "->begin");
  ac.begin();
  ESP_LOGI(TAG, "->ARREB1E");
  ac.setModel(ARREB1E);
  ESP_LOGI(TAG, "->Swing BOTH");
  ac.setSwing(kFujitsuAcSwingBoth);
  ESP_LOGI(TAG, "->Mode COOL");
  ac.setMode(kFujitsuAcModeCool);
  ESP_LOGI(TAG, "->Fan High");
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ESP_LOGI(TAG, "->Temp 24");
  ac.setTemp(22);  // 24C
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

  delay(500);
  ESP_LOGI(TAG, "->begin");
  ac.begin();
  ESP_LOGI(TAG, "->Turn On");
  ac.setCmd(kFujitsuAcCmdTurnOn);

  ESP_LOGI(TAG, "->target temp %d", this->target_temperature);
  // Set temperature
  uint8_t temperature_clamped =
      (uint8_t) roundf(clamp<float>(this->target_temperature, FUJI_EL_TEMP_MIN, FUJI_EL_TEMP_MAX));
  uint8_t temperature_offset = temperature_clamped - FUJI_EL_TEMP_MIN;
  //SET_NIBBLE(remote_state, FUJI_EL_TEMPERATURE_NIBBLE, temperature_offset);
  ac.setTemp(temperature_clamped);
  ESP_LOGI(TAG, "->Temp %d", temperature_clamped);

  ESP_LOGI(TAG, "->ALWAYS ON");
  ac.setCmd(kFujitsuAcCmdTurnOn);
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
      break;
  }

  // Set swing
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
	ac.setSwing(kFujitsuAcSwingVert);
    	ESP_LOGI(TAG, "->Swing Vert");
	break;
    case climate::CLIMATE_SWING_HORIZONTAL:
	ac.setSwing(kFujitsuAcSwingHoriz);
	ESP_LOGI(TAG, "->Swing Horiz");
	break;
    case climate::CLIMATE_SWING_BOTH:
	    ac.setSwing(kFujitsuAcSwingBoth);
    ESP_LOGI(TAG, "->Swing Both");
      break;
    case climate::CLIMATE_SWING_OFF:
    default:
	    ac.setSwing(kFujitsuAcSwingOff);
    ESP_LOGI(TAG, "->Swing Off");
      break;
  }

  ESP_LOGI(TAG, "Sending");
  ac.send();
  this->power_ = true;
}

void FujiElClimate::transmit_off_() {
  ESP_LOGI(TAG, "transmit_OFF");
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
