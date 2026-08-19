# Servo Calibration Guide

Accurate servo calibration is essential for stable walking gaits and posture balance.

## Calibration Workflow

1. **Mechanical Zeroing:**
   - When assembling mechanical horns onto servo splines, ensure the robot leg is attached as close as possible to right angles (90°).

2. **Connecting to Calibration Studio:**
   - Power up the robot and open the browser interface at `http://quadrobot.local` or `http://192.168.4.1`.
   - Navigate to the **Servo Calibration Studio** tab.

3. **Adjusting Joint Subtrim Offsets:**
   - Use the individual slider controls for each of the 8 joint servos:
     - **FL Coxa / Femur** (Front-Left)
     - **FR Coxa / Femur** (Front-Right)
     - **BL Coxa / Femur** (Back-Left)
     - **BR Coxa / Femur** (Back-Right)
   - Adjust the offset sliders in 1° increments until the joint aligns perfectly.

4. **Testing Poses:**
   - Click **Test Stand Pose** to verify that all 4 leg feet touch the ground evenly.
   - Click **Test Rest Pose** to verify that legs fold cleanly without mechanical binding.

5. **Saving Configuration:**
   - Click **Save Calibration** to commit subtrim values to persistent LittleFS flash storage (`config/servos.json`).
