/**
 * Strider32 Procedural 3D Robot Scenegraph Model (v0.3 Phase 2)
 * Manages Three.js Object3D leg hierarchy, joint transforms, and visual rendering.
 * Author: Sayuru Gunathilaka
 */

const RobotModel = {
    scene: null,
    camera: null,
    renderer: null,
    controls: null,
    axesHelper: null,
    robotGroup: null,

    // Joint Objects Reference Table
    legs: [],
    jointAngles: [90, 90, 90, 90, 90, 90, 90, 90], // FL(Coxa,Femur), FR, BL, BR

    // Physical Kinematic Dimensions (mm)
    BODY_LENGTH_MM: 120.0,
    BODY_WIDTH_MM: 80.0,
    BODY_HEIGHT_MM: 25.0,
    L1_COXA_MM: 30.0,
    L2_FEMUR_MM: 60.0,

    /**
     * Initializes the Three.js 3D Viewport, Lighting, Ground Plane, and Robot Scenegraph
     */
    initScene(container) {
        // 1. Create Scene
        this.scene = new THREE.Scene();

        // 2. Create Camera
        const aspect = container.clientWidth / container.clientHeight;
        this.camera = new THREE.PerspectiveCamera(45, aspect, 1, 2000);
        this.camera.position.set(180, 180, 120);
        this.camera.lookAt(0, 0, -20);

        // 3. Create WebGL Renderer
        this.renderer = new THREE.WebGLRenderer({ antialias: true });
        this.renderer.setSize(container.clientWidth, container.clientHeight);
        container.appendChild(this.renderer.domElement);

        // 4. Create Orbit Controls
        if (THREE.OrbitControls) {
            this.controls = new THREE.OrbitControls(this.camera, this.renderer.domElement);
        }

        // 5. Add Lighting
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
        this.scene.add(ambientLight);

        const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
        dirLight.position.set(150, 200, 150);
        this.scene.add(dirLight);

        // 6. Add Ground Grid (Z = -40mm)
        const grid = new THREE.GridHelper(300, 20);
        grid.position.set(0, 0, -40);
        this.scene.add(grid);

        // 7. Add Debug Axes Helper
        this.axesHelper = new THREE.AxesHelper(60);
        this.scene.add(this.axesHelper);

        // 8. Build Procedural Scenegraph
        this.buildRobotMesh();

        // 9. Resize Listener
        window.addEventListener('resize', () => {
            if (!container) return;
            this.camera.aspect = container.clientWidth / container.clientHeight;
            this.renderer.setSize(container.clientWidth, container.clientHeight);
        });

        // Set initial neutral standing posture
        this.setJointAngles([90, 90, 90, 90, 90, 90, 90, 90]);

        this.animate();
        return this.scene;
    },

    /**
     * Builds the Hierarchical Robot Scenegraph Mesh
     */
    buildRobotMesh() {
        this.robotGroup = new THREE.Object3D();
        this.scene.add(this.robotGroup);

        // Main Body Chassis
        const bodyGeom = new THREE.BoxGeometry(this.BODY_LENGTH_MM, this.BODY_WIDTH_MM, this.BODY_HEIGHT_MM);
        const bodyMat  = new THREE.MeshPhongMaterial({ color: 0x1e293b });
        const bodyMesh = new THREE.Mesh(bodyGeom, bodyMat);
        this.robotGroup.add(bodyMesh);

        // Leg Hip Shoulder Origins relative to body center (FL, FR, BL, BR)
        const shoulderOffsets = [
            { id: "FL", x:  60, y:  40, z: 0, isRight: false },
            { id: "FR", x:  60, y: -40, z: 0, isRight: true  },
            { id: "BL", x: -60, y:  40, z: 0, isRight: false },
            { id: "BR", x: -60, y: -40, z: 0, isRight: true  }
        ];

        this.legs = [];

        shoulderOffsets.forEach((s, idx) => {
            // Hip Pivot Pivot Group
            const hipGroup = new THREE.Object3D();
            hipGroup.position.set(s.x, s.y, s.z);
            this.robotGroup.add(hipGroup);

            // Coxa Joint (Yaw Rotation)
            const coxaJointGroup = new THREE.Object3D();
            hipGroup.add(coxaJointGroup);

            // Coxa Link Mesh (L1 = 30mm)
            const coxaLength = this.L1_COXA_MM;
            const coxaGeom = new THREE.BoxGeometry(coxaLength, 12, 12);
            const coxaMat  = new THREE.MeshPhongMaterial({ color: 0x06b6d4 }); // Cyan
            const coxaMesh = new THREE.Mesh(coxaGeom, coxaMat);
            coxaMesh.position.set(coxaLength / 2 * (s.isRight ? -1 : 1), 0, 0);
            coxaJointGroup.add(coxaMesh);

            // Femur Joint (Pitch Rotation)
            const femurJointGroup = new THREE.Object3D();
            femurJointGroup.position.set(coxaLength * (s.isRight ? -1 : 1), 0, 0);
            coxaJointGroup.add(femurJointGroup);

            // Femur Link Mesh (L2 = 60mm)
            const femurLength = this.L2_FEMUR_MM;
            const femurGeom = new THREE.CylinderGeometry(4, 3, femurLength, 8);
            const femurMat  = new THREE.MeshPhongMaterial({ color: 0x10b981 }); // Emerald
            const femurMesh = new THREE.Mesh(femurGeom, femurMat);
            femurMesh.position.set(0, 0, -femurLength / 2);
            femurJointGroup.add(femurMesh);

            // Foot Tip Sphere Marker
            const footGeom = new THREE.SphereGeometry(6, 8, 8);
            const footMat  = new THREE.MeshPhongMaterial({ color: 0xf59e0b }); // Amber
            const footMesh = new THREE.Mesh(footGeom, footMat);
            footMesh.position.set(0, 0, -femurLength);
            femurJointGroup.add(footMesh);

            this.legs.push({
                id: s.id,
                hipGroup: hipGroup,
                coxaJoint: coxaJointGroup,
                femurJoint: femurJointGroup,
                footTip: footMesh,
                isRight: s.isRight,
                coxaIndex: idx * 2,
                femurIndex: idx * 2 + 1
            });
        });
    },

    /**
     * Set 8-Joint Angles (degrees) & Update Scenegraph Transforms
     * @param {Array<number>} angles - 8 Joint Angles [FL_C, FL_F, FR_C, FR_F, BL_C, BL_F, BR_C, BR_F]
     */
    setJointAngles(angles) {
        if (!angles || angles.length < 8) return;
        for (let i = 0; i < 8; i++) {
            this.jointAngles[i] = angles[i];
        }

        this.legs.forEach(leg => {
            const coxaDeg  = this.jointAngles[leg.coxaIndex];
            const femurDeg = this.jointAngles[leg.femurIndex];

            // Convert to Radians relative to neutral 90°
            const coxaRad  = (coxaDeg - 90.0) * Math.PI / 180.0;
            const femurRad = (femurDeg - 90.0) * Math.PI / 180.0;

            // Apply Scenegraph Rotations
            leg.coxaJoint.rotation.z = coxaRad * (leg.isRight ? -1 : 1);
            leg.femurJoint.rotation.y = femurRad;
        });
    },

    /**
     * Set target Cartesian foot coordinates (x,y,z) for 4 legs using JS IK Engine
     * @param {Array<Object>} targets - [{x,y,z}, {x,y,z}, {x,y,z}, {x,y,z}]
     */
    setFootTargets(targets) {
        if (!targets || targets.length < 4 || typeof RobotKinematics === 'undefined') return;
        const angles = [];
        for (let i = 0; i < 4; i++) {
            const ik = RobotKinematics.solveLegIK(targets[i].x, targets[i].y, targets[i].z);
            angles.push(ik.coxa);
            angles.push(ik.femur);
        }
        this.setJointAngles(angles);
    },

    /**
     * Get current 8-Joint Angle state
     */
    getJointState() {
        return [...this.jointAngles];
    },

    /**
     * Toggle visibility of Debug Axes Helper
     */
    toggleAxes(visible) {
        if (this.axesHelper) this.axesHelper.visible = visible;
    },

    /**
     * Main Render Loop
     */
    animate() {
        requestAnimationFrame(() => this.animate());
        if (this.controls) this.controls.update();
        if (this.renderer && this.scene && this.camera) {
            this.renderer.render(this.scene, this.camera);
        }
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = RobotModel;
}
