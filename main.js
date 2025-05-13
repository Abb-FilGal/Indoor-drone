  const SERVICE_UUID = "6b1d428a-3abd-4e08-a55f-63f2a0dfa565";
  const BATTERY_CHAR_UUID = "00002a19-0000-1000-8000-00805f9b34fb"

  function log(msg) {
    console.log(msg);
  }

  const BLEOptions = {
    filters: [{
      services: [SERVICE_UUID]
    }],
    optionalServices: [
      'battery_service',
      'a1209824-94ff-4ed8-a3b3-50f0a9c41325', // Telemetry
      'b3dca7d3-cd74-4726-a9f5-d9f2f3c9f3b3'  // Status
    ]
  };

  function bleWorks() {
    if (!navigator.bluetooth) {
      log("❌ Web Bluetooth API not available in this browser.");
      return false;
    }
    log("✅ Web Bluetooth API is supported.");
    return true;
  }

  export function doBluetooth() {
    log("🔍 Trying to connect to Bluetooth...");
  
    if (!bleWorks()) return;
  
    navigator.bluetooth.requestDevice(BLEOptions)
      .then(device => {
        log(`📱 Found device: ${device.name || "(Unnamed)"}`);
        return device.gatt.connect();
      })
      .then(server => {
        log("🔗 Connected to GATT server.");
        return server.getPrimaryServices();
      })
      .then(services => {
        log(`🧪 Found ${services.length} service(s):`);
  
        // For each service
        services.reduce((promiseChain, service) => {
          return promiseChain.then(() => {
            log(`🔧 Service UUID: ${service.uuid}`);
            return service.getCharacteristics().then(characteristics => {
              log(`   📋 ${characteristics.length} characteristic(s):`);
  
              // For each characteristic
              return characteristics.reduce((charChain, char) => {
                return charChain.then(() => {
                  log(`   • UUID: ${char.uuid}`);
  
                  const props = char.properties;
  
                  const readPromise = props.read
                    ? char.readValue().then(value => {
                    
                        if(char.uuid == "00002a19-0000-1000-8000-00805f9b34fb") {
                            const batteryPercent = value.getUint8(0);
                            log(`     🔔 Notification from ${char.uuid}: " ${batteryPercent}%"`);
                        }else {
                            const text = new TextDecoder("utf-8").decode(value.buffer);
                            log(`     🔔 Notification from ${char.uuid}: "${text}"`);
                        }
                        
                        
                      }).catch(err => {
                        log("     ❌ Read error: " + err);
                      })
                    : Promise.resolve(log("     🚫 Not readable"));
  
                  const notifyPromise = props.notify
                    ? char.startNotifications()
                        .then(() => {
                          char.addEventListener("characteristicvaluechanged", event => {
                            const value = event.target.value;
                            if(char.uuid == "00002a19-0000-1000-8000-00805f9b34fb") {
                                const batteryPercent = value.getUint8(0);
                                log(`     🔔 Notification from ${char.uuid}: "${batteryPercent}%"`);
                            }else {
                                const text = new TextDecoder("utf-8").decode(value.buffer);
                                log(`     🔔 Notification from ${char.uuid}: "${text}"`);
                            }
                            
                          });
                          log("     ✅ Subscribed to notifications");
                        })
                        .catch(err => {
                          log("     ❌ Notification error: " + err);
                        })
                    : Promise.resolve(log("     🚫 Not notifiable"));
  
                  return readPromise.then(() => notifyPromise);
                });
              }, Promise.resolve());
            });
          });
        }, Promise.resolve());
      })
      .catch(error => {
        log("❌ Error: " + error);
      });
  }
