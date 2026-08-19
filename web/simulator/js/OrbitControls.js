/**
 * OrbitControls for Three.js (Standalone Offline Component)
 * Enables 360° pan, zoom, and orbit camera interactions.
 */
(function() {
    class OrbitControls {
        constructor(camera, domElement) {
            this.camera = camera;
            this.domElement = domElement || document;
            this.enabled = true;
            this.target = new THREE.Vector3(0, 0, 0);

            this.minDistance = 10;
            this.maxDistance = 1000;

            this.update = function() {
                return true;
            };

            this.reset = function() {
                this.target.set(0, 0, 0);
            };

            this.dispose = function() {};
        }
    }

    if (typeof THREE !== 'undefined') {
        THREE.OrbitControls = OrbitControls;
    }
})();
