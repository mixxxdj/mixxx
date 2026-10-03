import template from "./knob.html?raw";

/**
 * @param {object} config - Optional configuration
 * @param {string} config.name - Name
 * @param {number} config.min - Minimum value (default: 0)
 * @param {number} config.max - Maximum value (default: 100)
 * @param {number} config.step - Increment step (default: 0.5)
 * @param {number} config.value - Initial value (default: min)
 * @param {number} config.default - Value to reset to on double-click (default: 50)
 * @param {string} config.color - Primary color (default: '#6d4aff')
 * @param {number} config.size - Size in pixels (default: 100)
 * @param {number} config.trackWidth - Track width (default: 8)
 * @param {boolean} config.showLabel - Show label (default: true)
 * @param {boolean} config.showTooltip - Show tooltip (default: false)
 * @param {string} config.unit - Unit of measurement (default: '')
 */
export default (config = {}) => ({
    // Configuration
    name: config.name ?? "",
    min: config.min ?? 0,
    max: config.max ?? 100,
    step: config.step ?? 0.5,
    value: config.value ?? (config.min ?? 0),
    defaultVal: config.default ?? 50,
    color: config.color ?? "#6d4aff",
    size: config.size ?? 100,
    trackWidth: config.trackWidth ?? 8,
    showLabel: config.showLabel ?? true,
    showTooltip: config.showTooltip ?? false,
    unit: config.unit ?? "",
    sensitivity: config.sensitivity ?? 1,

    // Internal state
    angle: 0,
    totalRotation: 0,
    normalizedAngle: 0,
    dragging: false,
    lastClientX: 0,
    lastClientY: 0,
    startClientX: 0,
    startClientY: 0,
    clickCount: 0,
    clickTimer: null,
    doubleClickThreshold: 300, // ms
    clickMoveThreshold: 5, // px — max movement to still count as a click

    init() {
        this.$nextTick(() => { this.$el.innerHTML = template; });

        this.onMouseMoveBound = (e) => this.handleMove(e);
        this.onTouchMoveBound = (e) => this.handleMove(e);
        this.onMouseUpBound = () => this.endDrag();

        this.updateAngleFromValue();
    },

    /**
     * Update angle from current value
     */
    updateAngleFromValue() {
        const range = this.max - this.min;
        const percentage = range > 0 ? (this.value - this.min) / range : 0;
        this.totalRotation = percentage * 270; // 270° knob arc
        this.normalizeAngle();
    },

    /**
     * Normalize angle for display (-135 to +135)
     */
    normalizeAngle() {
        this.normalizedAngle = Math.max(-135, Math.min(135, this.totalRotation - 135));
    },

    get containerClass() {
        return this.dragging ? "cursor-grabbing" : "cursor-grab";
    },

    /**
     * Determine quadrant based on coordinates (dx, dy) from center
     *
     *   Q2 (/)  |  Q1 (\)
     * ----------+----------
     *   Q3 (\)  |  Q4 (/)
     *
     * @param {number} dx - Horizontal distance from center (positive = right)
     * @param {number} dy - Vertical distance from center (positive = down)
     * @returns {number} 1, 2, 3 or 4
     */
    getQuadrant(dx, dy) {
        if (dx >= 0 && dy < 0) { return 1; }  // top-right
        if (dx < 0 && dy < 0) { return 2; }   // top-left
        if (dx < 0 && dy >= 0) { return 3; }  // bottom-left
        return 4;                         // bottom-right
    },

    /**
     * Extract client coordinates from event (mouse or touch)
     * @param e
     */
    getClientPos(e) {
        if (e.touches && e.touches.length > 0) {
            return {x: e.touches[0].clientX, y: e.touches[0].clientY};
        }
        return {x: e.clientX, y: e.clientY};
    },

    /**
     * Start knob drag
     * @param e
     */
    startDrag(e) {
        this.dragging = true;
        const pos = this.getClientPos(e);
        this.lastClientX = pos.x;
        this.lastClientY = pos.y;
        this.startClientX = pos.x;
        this.startClientY = pos.y;

        document.addEventListener("mousemove", this.onMouseMoveBound);
        document.addEventListener("mouseup", this.onMouseUpBound);
        document.addEventListener("touchmove", this.onTouchMoveBound, {passive: false});
        document.addEventListener("touchend", this.onMouseUpBound);
    },

    onMouseMoveBound: null,
    onTouchMoveBound: null,
    onMouseUpBound: null,

    /**
     * Handle movement during drag
     *
     * Uses quadrant approach to simulate tangential push:
     * - Q1 and Q3: \ direction
     * - Q2 and Q4: / direction
     * @param e
     */
    handleMove(e) {
        if (!this.dragging) { return; }

        e.preventDefault?.();

        const pos = this.getClientPos(e);
        const dx = pos.x - this.lastClientX;
        const dy = pos.y - this.lastClientY;
        this.lastClientX = pos.x;
        this.lastClientY = pos.y;

        // Knob center
        const rect = this.$el.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;

        // Current position relative to center
        const cx = pos.x - centerX;
        const cy = pos.y - centerY;

        // Current quadrant
        const quad = this.getQuadrant(cx, cy);

        // Approximate tangential delta based on quadrant
        let delta;
        switch (quad) {
        case 1: delta = dx + dy; break;       // Q1: \
        case 2: delta = dx - dy; break;       // Q2: /
        case 3: delta = -(dx + dy); break;    // Q3: \
        case 4: delta = -dx + dy; break;      // Q4: /
        }

        this.totalRotation = Math.max(0, Math.min(270, this.totalRotation + delta * this.sensitivity));

        this.normalizeAngle();
        this.updateValueFromAngle();
        this.roundValue();
    },

    /**
     * End drag and remove listeners
     */
    endDrag() {
        this.dragging = false;
        document.removeEventListener("mousemove", this.onMouseMoveBound);
        document.removeEventListener("mouseup", this.onMouseUpBound);
        document.removeEventListener("touchmove", this.onTouchMoveBound);
        document.removeEventListener("touchend", this.onMouseUpBound);

        // Check if this was a click (no significant movement)
        const dx = this.lastClientX - this.startClientX;
        const dy = this.lastClientY - this.startClientY;
        const moved = Math.sqrt(dx * dx + dy * dy);

        if (moved < this.clickMoveThreshold) {
            this.handleClick();
        }
    },

    /**
     * Handle click / double-click → default value (defaultVal)
     */
    handleClick() {
        this.clickCount++;

        if (this.clickCount === 1) {
            // First click — wait to see if a second one follows
            this.clickTimer = setTimeout(() => {
                this.clickCount = 0;
                this.clickTimer = null;
            }, this.doubleClickThreshold);
        } else if (this.clickCount >= 2) {
            // Double-click — set to default value
            if (this.clickTimer) {
                clearTimeout(this.clickTimer);
                this.clickTimer = null;
            }
            this.clickCount = 0;

            this.setValue(this.defaultVal);
        }
    },

    /**
     * Update value from angle
     */
    updateValueFromAngle() {
        let percentage = this.totalRotation / 270;
        percentage = Math.max(0, Math.min(1, percentage));

        const range = this.max - this.min;
        this.value = this.min + (range * percentage);
    },

    /**
     * Round to nearest step
     */
    roundValue() {
        if (this.step <= 0) { return; }

        this.value = Math.round(this.value / this.step) * this.step;
        this.value = Math.max(this.min, Math.min(this.max, this.value));

        // Update angle after rounding
        this.updateAngleFromValue();
    },

    /**
     * Set value programmatically
     * @param val
     */
    setValue(val) {
        this.value = Math.max(this.min, Math.min(this.max, val));
        this.roundValue();
        this.updateAngleFromValue();
    },

    /**
     * Reset to minimum value
     */
    reset() {
        this.setValue(this.min);
    },

    /**
     * Increase value by one step
     */
    increment() {
        this.setValue(this.value + this.step);
    },

    /**
     * Decrease value by one step
     */
    decrement() {
        this.setValue(this.value - this.step);
    },
});
