# Servo Calibration & IK Alignment Guide

> [!NOTE]
> **Hardware Validation Notice:** Software verified; physical quadruped walking has not yet been hardware validated.

Accurate servo calibration and subtrim alignment are essential for mapping analytical 3D Inverse Kinematics (IK) joint output angles to physical PWM signals on the 8-DOF quadruped frame.

---

## 1. Calibration & Mechanical Zeroing Workflow

1. **Mechanical Horn Zeroing:**
   - Mount servo horns onto splines so that legs align as close as possible to $90^\circ$ perpendicular angles in the neutral standing posture.

2. **Connecting to Web Calibration Studio:**
   - Connect to `http://192.168.4.1` (or `http://strider32.local`) and navigate to the **Calibration Studio** tab.

3. **Subtrim Offset Adjustment:**
   - Adjust subtrim offsets ($-30^\circ$ to $+30^\circ$) for each of the 8 joint servos:
     - **J0 / J1:** FL Coxa / Femur (Front-Left)
     - **J2 / J3:** FR Coxa / Femur (Front-Right)
     - **J4 / J5:** BL Coxa / Femur (Back-Left)
     - **J6 / J7:** BR Coxa / Femur (Back-Right)

4. **Testing Poses:**
   - Click **Stand Stance** to verify that all 4 leg feet reach target $(X=60\text{mm}, Y=0, Z=-40\text{mm})$ evenly.

5. **Saving Configuration:**
   - Click **Save Calibration** to store subtrim values to persistent LittleFS flash storage (`config/servos.json`).
