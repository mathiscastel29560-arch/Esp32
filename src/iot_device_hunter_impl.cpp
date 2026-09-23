#include "iot_device_hunter.h"
#include "wifi_tools.h"
#include "results_display.h"

namespace IoTDeviceHunter {

HuntResult huntDevices(uint32_t durationMs) {
    HuntResult result{0, {}};
    
    Serial.println("\n=== IoT Device Hunter ===");
    Serial.println("Scanning for common IoT devices...");
    
    // Common IoT manufacturer prefixes/names
    const char* iot_patterns[] = {
        "Amazon", "Echo", "alexa",
        "Google", "Nest", "Chromecast",
        "Philips", "Hue", "Light",
        "Ring", "Doorbell", "Camera",
        "TP-Link", "Kasa", "Smart",
        "Tuya", "SmartLife", "eWeLink",
        "LIFX", "Nanoleaf", "RGB",
        "Wyze", "YI", "Cam",
        "August", "Yale", "Lock",
        "Ecobee", "Thermostat", "Heating"
    };
    
    auto networks = WifiTools::scan();
    
    for (const auto &net : networks) {
        bool isIoT = false;
        String vendor = "Unknown";
        String devType = "Device";
        
        for (const char* pattern : iot_patterns) {
            if (net.ssid.indexOf(pattern) >= 0) {
                isIoT = true;
                vendor = pattern;
                
                if (net.ssid.indexOf("Camera") >= 0 || net.ssid.indexOf("Cam") >= 0) devType = "Camera";
                else if (net.ssid.indexOf("Light") >= 0 || net.ssid.indexOf("Hue") >= 0) devType = "Smart Light";
                else if (net.ssid.indexOf("Lock") >= 0 || net.ssid.indexOf("Door") >= 0) devType = "Smart Lock";
                else if (net.ssid.indexOf("Thermostat") >= 0) devType = "Thermostat";
                else if (net.ssid.indexOf("Echo") >= 0 || net.ssid.indexOf("Alexa") >= 0) devType = "Smart Speaker";
                
                break;
            }
        }
        
        if (isIoT) {
            DetectedDevice dev{vendor, devType, net.rssi, "WiFi", millis()};
            result.devices.push_back(dev);
            result.devicesFound++;
            
            Serial.println("  [IoT] " + vendor + " - " + devType + " (" + 
                         net.ssid + ") RSSI:" + String(net.rssi));
        }
    }
    
    Serial.println("✓ Found " + String(result.devicesFound) + " IoT devices");

    std::vector<String> displayLines;
    if (result.devicesFound > 0) {
        displayLines.push_back(String(result.devicesFound) + " IoT device(s)");
        for (size_t i = 0; i < result.devices.size() && i < 8; i++) {
            displayLines.push_back(result.devices[i].vendor + " - " + result.devices[i].deviceType);
            displayLines.push_back("  Signal: " + String(result.devices[i].signal) + "dBm");
        }
    } else {
        displayLines.push_back("No IoT devices found");
    }

    ResultsDisplay::showResult("IoT Hunter", {
        "IoT Device Discovery",
        String(result.devicesFound) + " device(s)",
        100,
        displayLines,
        result.devicesFound > 0 ? ResultsDisplay::ResultType::SCAN_RESULT : ResultsDisplay::ResultType::INFO
    });

    return result;
}

}  // namespace IoTDeviceHunter
