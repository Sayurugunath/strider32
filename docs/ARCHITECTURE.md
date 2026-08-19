# Strider32 System Architecture & Kinematic Design

> [!NOTE]
> **Hardware Validation Notice:** Software verified; physical quadruped walking has not yet been hardware validated.

---

## 1. Subsystem Architecture

```
                      +------------------------------------------+
                      |       Strider32 Web Control Center       |
                      |   (HTML5 / CSS3 / Vanilla ES6+ JS)       |
                      +--------------------+---------------------+
                                           | HTTP REST API
                                           v
                      +--------------------+---------------------+
                      |         ESPAsyncWebServer & API          |
                      +----+-------------------+-----------------+
                           |                   |
                           v                   v
            +--------------+----+     +--------+------------------+
            |   Gait Engine &   |     |      Config Manager       |
            | Animation Player  |     |   (LittleFS Flash Storage)|
            +--------------+----+     +--------+------------------+
                           |                   |
                           v                   v
            +--------------+-------------------+------------------+
            |            Safety System & Guard                  |
            +----------------------+------------------------------+
                                   |
                                   v
            +----------------------+------------------------------+
            |          IServoDriver Abstraction Layer             |
            +----------+-------------------------------+----------+
                       |                               |
                       v                               v
         +-------------+---------------+   +-----------+----------+
         | ESP32DirectServoDriver      |   | PCA9685ServoDriver   |
         | (Direct GPIO LEDC PWM)      |   | (Optional I2C Module)|
         +-----------------------------+   +----------------------+
```

---

## 2. 3D Inverse Kinematics (IK) & Coordinate Conventions

Strider32 v0.2 uses an analytical 3D Inverse Kinematics solver (`Kinematics::solveLegIK`) with workspace reach protection.

### Coordinate System (per leg hip pivot origin)
* **$+X$ Direction:** Points forward (front of robot).
* **$+Y$ Direction:** Points outward laterally (left for left legs, right for right legs).
* **$+Z$ Direction:** Points vertically upward ($Z = -40\text{mm}$ ground stance level).

### Leg Joint Conventions & Mirroring Table

| Leg ID | Joint Index | Joint Name | Neutral Angle | Coordinate Mapping |
|---|---|---|---|---|
| **FL** | J0 / J1 | Front-Left Coxa / Femur | $90^\circ / 90^\circ$ | $(+X, +Y, Z)$ |
| **FR** | J2 / J3 | Front-Right Coxa / Femur | $90^\circ / 90^\circ$ | $(+X, -Y, Z)$ |
| **BL** | J4 / J5 | Back-Left Coxa / Femur | $90^\circ / 90^\circ$ | $(+X, +Y, Z)$ |
| **BR** | J6 / J7 | Back-Right Coxa / Femur | $90^\circ / 90^\circ$ | $(+X, -Y, Z)$ |

---

## 3. IK Analytical Equations

For a target foot coordinate $(x, y, z)$ relative to leg shoulder origin ($L_1 = 30\text{mm}, L_2 = 60\text{mm}$):

1. **Coxa (Hip Yaw Angle):**
   $$\theta_{\text{coxa}} = \text{atan2}(y, x) \cdot \frac{180}{\pi} + 90^\circ$$
2. **Planar Radius & Extension Distance:**
   $$r_{\text{eff}} = \sqrt{x^2 + y^2} - L_1$$
   $$D = \sqrt{r_{\text{eff}}^2 + z^2}$$
3. **Femur (Elevation Angle):**
   $$\theta_{\text{femur}} = \left(\text{atan2}(z, r_{\text{eff}}) + \text{acos}\left(\frac{D}{2 L_2}\right)\right) \cdot \frac{180}{\pi}$$
4. **Workspace Boundary Clamp:** If $D > 2 L_2$, $D$ is clamped to $2 L_2 - 0.1\text{mm}$ to prevent `NaN` values.
