#include "dualboot.h"
#include <Arduino.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

namespace DualBoot {

void markValid() {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) == ESP_OK &&
        state == ESP_OTA_IMG_PENDING_VERIFY) {
        esp_ota_mark_app_valid_cancel_rollback();
    }
}

bool bootIntoBruce() {
    const esp_partition_t *bruce =
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);
    if (!bruce) return false;

    if (esp_ota_set_boot_partition(bruce) != ESP_OK) return false;

    Serial.println("Rebooting into Bruce (ota_1)...");
    delay(200); // let the serial line flush before the reset
    esp_restart();
    return true; // unreachable
}

} // namespace DualBoot
