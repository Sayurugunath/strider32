# RESTful API Specification

Base URL: `http://quadrobot.local/api/v1` or `http://<ESP32-IP>/api/v1`

---

## 1. System & Telemetry Endpoints

### `GET /api/v1/status`
Returns general operational status and hardware summary.

**Response Example (200 OK):**
```json
{
  "success": true,
  "data": {
    "robot_name": "QuadRobot",
    "firmware_version": "1.0.0",
    "uptime_seconds": 1420,
    "free_heap_bytes": 184512,
    "estop_active": false,
    "safety_state": 0,
    "gait": 0,
    "speed": 50,
    "servo_backend": 0,
    "wifi": {
      "mode": "AP",
      "ip": "192.168.4.1",
      "rssi": 0
    }
  }
}
```

### `GET /api/v1/diagnostics`
Returns real system telemetry metrics.

**Response Example (200 OK):**
```json
{
  "success": true,
  "data": {
    "uptime_seconds": 340,
    "free_heap_bytes": 178920,
    "min_free_heap_bytes": 164210,
    "cpu_freq_mhz": 240,
    "chip_revision": 1,
    "firmware_version": "1.0.0",
    "wifi_mode": "AP",
    "wifi_connected": true,
    "rssi_dbm": 0,
    "ip_address": "192.168.4.1",
    "safety_state": 0,
    "estop_active": false,
    "servo_backend": 0,
    "hardware_attached": true,
    "last_error": "None"
  }
}
```

---

## 2. Emergency Stop Endpoints

### `POST /api/v1/estop`
Triggers an immediate emergency stop. Disables all servo PWM outputs.

### `POST /api/v1/estop/reset`
Explicitly resets latched emergency stop state after checking physical switch state on GPIO 34.

**Response Example (200 OK):**
```json
{
  "success": true,
  "message": "E-STOP Reset Successful. System state is now READY.",
  "state": "READY"
}
```

---

## 3. Motion & Control Endpoints

### `POST /api/v1/control`
Executes movement, gait switches, or speed adjustments. (Rejected with HTTP 400 if system is ESTOPPED).

**Request Body Example:**
```json
{
  "action": "walk",
  "forward": 1.0,
  "lateral": 0.0,
  "turn": 0.0
}
```

---

## 4. Servo & Calibration Endpoints

### `GET /api/v1/servos`
Retrieves current target angles, calculated pulse widths, and subtrim offsets.

### `POST /api/v1/servos/calibrate`
Updates subtrim offsets and optionally saves them persistently to LittleFS flash (`servos.json`).

---

## 5. Network Configuration Endpoints

### `GET /api/v1/network`
Returns network mode, AP SSID, STA SSID, current IP, and hostname. Passwords are masked as `***`.

### `POST /api/v1/network`
Saves new network configuration to LittleFS (`network.json`).

### `GET /api/v1/network/scan`
Scans and returns available local Wi-Fi networks.

---

## 6. Keyframe Animation Studio Endpoints

### `GET /api/v1/animations`
Lists saved keyframe animation JSON files in LittleFS (`/animations/*.json`).

### `POST /api/v1/animations`
Saves a new animation sequence to LittleFS with schema validation.

### `GET /api/v1/animations/:id`
Reads a specific animation JSON sequence.

### `DELETE /api/v1/animations/:id`
Deletes a saved animation file from flash.

### `POST /api/v1/animations/:id/play`
Loads and triggers non-blocking playback of an animation file. (Rejects if system is ESTOPPED).
