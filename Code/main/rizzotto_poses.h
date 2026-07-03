#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// ===== SERVO INDEX MAPPING FOR RIZZOTTO =====
typedef enum {
    R1 = 0, // Right Hip 1 (Horizontal)
    R2 = 1, // Right Hip 2 (Horizontal)
    L1 = 2, // Left Hip 1  (Horizontal)
    L2 = 3, // Left Hip 2  (Horizontal)
    R4 = 4, // Right Vert 4 (Vertical - mounted with R2)
    R3 = 5, // Right Vert 3 (Vertical - mounted with R1)
    L3 = 6, // Left Vert 3  (Vertical - mounted with L1)
    L4 = 7  // Left Vert 4  (Vertical - mounted with L2)
} ServoName;

typedef enum {
    FACE_ANIM_LOOP = 0,
    FACE_ANIM_ONCE = 1,
    FACE_ANIM_BOOMERANG = 2
} FaceAnimMode;

// ===== EXTERNAL LINKAGE TO DRIVER CONFIGS =====
extern int frameDelay;
extern int walkCycles;
extern char currentCommand[32];

extern void set_servo_angle(int channel, float angle);
extern bool pressingCheck(const char* cmd, int ms);

// Stub placeholders replacing old LED screen functions
static inline void setFaceWithMode(const char* faceName, FaceAnimMode mode) { /* Placeholder */ }
static inline void delayWithFace(unsigned long ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
static inline void enterIdle(void) { /* Placeholder */ }

// ===== FORWARD DECLARATIONS =====
/*static inline void runStandPose(int face);*/

// =============================
// ========== POSES ============
// =============================

// Hard reset/calibration pose forcing every actuator channel to 0 degrees
static inline void runZeroPose(void) {
    printf("RIZZOTTO POSING: ZERO CALIBRATION\n");
    
    set_servo_angle(R1, 100); 
    set_servo_angle(R2, 45); 
    set_servo_angle(L1, 110); 
    set_servo_angle(L2, 85); 
    set_servo_angle(R4, 70);
    set_servo_angle(R3, 70); 
    set_servo_angle(L3, 45); 
    set_servo_angle(L4, 30);
}

static inline void runLegsExtremeUpPose(void) { 
    printf("RIZZOTTO POSING: EXTREME LEGS UP\n"); 
    
    // Hips stay at standard stand/body alignment
    set_servo_angle(R1, 90); 
    set_servo_angle(R2, 45); 
    set_servo_angle(L1, 110); 
    set_servo_angle(L2, 90); 
    
    // Legs pushed past the flat "Zero" limits to point upwards
    set_servo_angle(R4, 110);  // Stand: 0   -> Flat: 70  -> Up: 110
    set_servo_angle(R3, 30);   // Stand: 135 -> Flat: 70  -> Up: 30
    set_servo_angle(L3, 85);   // Stand: 0   -> Flat: 45  -> Up: 85
    set_servo_angle(L4, 0);    // Stand: 90  -> Flat: 30  -> Up: 0
}

static inline void runStandPose() { 
    printf("RIZZOTTO POSING: STAND\n"); 
 
    set_servo_angle(R1, 90); 
    set_servo_angle(R2, 90); 
    set_servo_angle(L1, 90); 
    set_servo_angle(L2, 45); 
    set_servo_angle(R4, 0); 
    set_servo_angle(R3, 140); 
    set_servo_angle(L3, 20); 
    set_servo_angle(L4, 100);
}

// =============================================================
// ============== CALIBRATED CREEP-GAIT WALK ENGINE ============
// =============================================================
// WHY THE OLD walkForwardOneStep DIDN'T WALK:
//   * it lifted one leg's knee while swinging a DIFFERENT leg's hip
//     (e.g. lifted FR knee R3 but swung BR hip R2), so legs fought;
//   * it never put the robot in the stand pose before stepping;
//   * it had no real power stroke to translate the body.
//
// This engine uses a CREEP gait: only one foot leaves the ground at a
// time, so 3 feet always support the body (balance is automatic). Then
// with all 4 feet planted it rotates every hip together -> body slides
// forward (the "power stroke").
//
// CRITICAL: this robot's servo horns are mounted at ASYMMETRIC offsets,
// so "90 = neutral" is FALSE. We therefore reuse the exact per-servo
// neutral angles from runStandPose(), and give each leg its own signed
// LIFT and FORWARD direction.

// LEG LAYOUT:  the L-servos are the FRONT legs, the R-servos are the BACK.
//        FRONT
//   L1+L3 ----- L2+L4
//      |   body    |
//   R1+R3 ----- R2+R4
//        BACK
// Leg index 0..3 = FL, FR, BL, BR.  Each leg = one hip + one knee servo.
// (Left/right of L1 vs L2 doesn't matter for a trot, as long as each
//  diagonal pair below is one front + one back leg.)
typedef enum { LEG_FL = 0, LEG_FR = 1, LEG_BL = 2, LEG_BR = 3 } LegId;

// Hip / knee servo of each leg.
static const int HIP_IDX[4]  = { L1, L2, R1, R2 };   // FL FR BL BR
static const int KNEE_IDX[4] = { L3, L4, R3, R4 };

// Planted (standing) angle of each leg's hip & knee  ==  runStandPose().
static const int HIP_STAND[4]  = { 110, 90, 90, 45 };   // L1 L2 R1 R2
static const int KNEE_STAND[4] = {   0, 90, 135, 0 };   // L3 L4 R3 R4

// HIP SWING SIGNS — two patterns, because this robot's four hips all pivot
// the SAME rotational sense:
//   STRAIGHT walk  -> front legs OPPOSITE back legs  => body translates
//   ROTATE in place-> all four the SAME              => body spins on the spot
// (Empirically {+1,+1,+1,+1} spun; flipping the FRONT pair gives straight.)
static const int HIP_FWD[4]  = { -1, -1, +1, +1 };   // FL FR BL BR  (STRAIGHT walk)
static const int HIP_ROT[4]  = { +1, +1, +1, +1 };   // FL FR BL BR  (ROTATE in place)

// Sign so +lift raises the foot OFF the ground (from runLegsExtremeUpPose data).
static const int KNEE_UP[4]  = { +1, -1, -1, +1 };   // FL FR BL BR

// Diagonal trot pairs: one pair swings forward while the OTHER strokes the
// body. G0 = front-left + back-right,  G1 = front-right + back-left.
static const int DIAG[2][2] = { { LEG_FL, LEG_BR }, { LEG_FR, LEG_BL } };

// ---- SPEED / SIZE TUNABLES ----
static int HIP_SWING = 26;   // stride length (deg from stand). Bigger = longer steps.
static int KNEE_LIFT = 36;   // foot clearance (deg). Just enough to not drag.
static int SWING_MS  = 110;  // time for the swing+stroke move.  SMALLER = FASTER.
static int LIFT_MS   = 45;   // time to lift / plant a foot.      SMALLER = FASTER.

// Which side each leg is on (used only for turning).  -1 = left, +1 = right.
// If rotateInPlace() turns the WRONG way, flip the dir argument in main.
static const int SIDE[4] = { -1, +1, -1, +1 };   // FL FR BL BR

// ---- ROUTINE TUNABLES (calibrate on the floor) ----
static int CYCLES_PER_METER = 14;  // trot strides to cover ~1 m. Count it, set it.
static int TURN_CYCLES      = 8;   // half-steps per rotateInPlace() call (~90 deg).

// ---- live state ----
static float gCur[8];     // current commanded angle of every servo (0..7)
static float gSwing[4];   // per-leg hip:  -1 back .. 0 stand .. +1 forward
static float gLift[4];    // per-leg knee:  0 planted .. 1 lifted

// The gait reads hip signs through this pointer: HIP_FWD to walk straight,
// HIP_ROT to spin in place. Each routine sets it before it runs.
static const int *gHipSign = HIP_FWD;

static inline float hipTarget(int leg)  { return HIP_STAND[leg]  + gHipSign[leg] * gSwing[leg] * HIP_SWING; }
static inline float kneeTarget(int leg) { return KNEE_STAND[leg] + KNEE_UP[leg] * gLift[leg]  * KNEE_LIFT; }

// Smoothly drive ALL 8 servos from their current angles to the targets
// implied by gSwing/gLift, in small steps over durMs. Moving everything
// together in tiny increments is what stops the robot throwing itself off
// balance.
static inline void applyTargets(int durMs) {
    const int STEP_MS = 15;
    int steps = durMs / STEP_MS;
    if (steps < 1) steps = 1;

    float startA[8], endA[8];
    for (int leg = 0; leg < 4; leg++) {
        endA[HIP_IDX[leg]]  = hipTarget(leg);
        endA[KNEE_IDX[leg]] = kneeTarget(leg);
    }
    for (int i = 0; i < 8; i++) startA[i] = gCur[i];

    for (int s = 1; s <= steps; s++) {
        float t = (float)s / steps;
        for (int i = 0; i < 8; i++) {
            gCur[i] = startA[i] + (endA[i] - startA[i]) * t;
            set_servo_angle(i, gCur[i]);
        }
        vTaskDelay(pdMS_TO_TICKS(STEP_MS));
    }
}

// Move to the calibrated stand and sync the smooth-move state. Call ONCE
// before walking so the gait starts from a known pose.
static inline void standAndSettle(void) {
    for (int leg = 0; leg < 4; leg++) {
        gSwing[leg] = 0;
        gLift[leg]  = 0;
        gCur[HIP_IDX[leg]]  = HIP_STAND[leg];
        gCur[KNEE_IDX[leg]] = KNEE_STAND[leg];
    }
    for (int i = 0; i < 8; i++) set_servo_angle(i, gCur[i]);
    vTaskDelay(pdMS_TO_TICKS(800));
}

// Smoothly drive all 8 servos to explicit per-servo angles (for static poses
// like sit). target[] is indexed by servo: target[R1], target[L4], etc.
static inline void moveAllTo(const int target[8], int durMs) {
    const int STEP_MS = 15;
    int steps = durMs / STEP_MS;
    if (steps < 1) steps = 1;
    float startA[8];
    for (int i = 0; i < 8; i++) startA[i] = gCur[i];
    for (int s = 1; s <= steps; s++) {
        float t = (float)s / steps;
        for (int i = 0; i < 8; i++) {
            gCur[i] = startA[i] + (target[i] - startA[i]) * t;
            set_servo_angle(i, gCur[i]);
        }
        vTaskDelay(pdMS_TO_TICKS(STEP_MS));
    }
}

// TROT half-step: lift diagonal pair g and fling it FORWARD while, at the
// same time, the planted pair strokes BACKWARD -> the body is driven forward.
// Two legs swing + two legs stroke together = fast and coordinated.
static inline void trotHalfStep(int g) {
    int a = DIAG[g][0],     b = DIAG[g][1];        // swing pair (this group)
    int c = DIAG[1 - g][0], d = DIAG[1 - g][1];    // planted pair (other group)

    gLift[a] = 1; gLift[b] = 1;                     // lift the swing pair
    applyTargets(LIFT_MS);

    gSwing[a] = +1; gSwing[b] = +1;                // swing pair -> forward
    gSwing[c] = -1; gSwing[d] = -1;                // planted pair -> stroke back
    applyTargets(SWING_MS);

    gLift[a] = 0; gLift[b] = 0;                     // plant the swing pair
    applyTargets(LIFT_MS);
}

// TURN half-step: same rhythm as the trot, but the planted pair strokes to
// YAW the body in place (left & right sides push opposite ways) instead of
// translating it. dir = +1 turns one way, -1 the other.
static inline void turnHalfStep(int g, int dir) {
    int a = DIAG[g][0],     b = DIAG[g][1];        // swing pair
    int c = DIAG[1 - g][0], d = DIAG[1 - g][1];    // planted (yaw) pair

    gLift[a] = 1; gLift[b] = 1;                     // lift the swing pair
    applyTargets(LIFT_MS);

    gSwing[a] = -SIDE[a] * dir; gSwing[b] = -SIDE[b] * dir;   // recover swing pair
    gSwing[c] =  SIDE[c] * dir; gSwing[d] =  SIDE[d] * dir;   // yaw with planted pair
    applyTargets(SWING_MS);

    gLift[a] = 0; gLift[b] = 0;                     // plant the swing pair
    applyTargets(LIFT_MS);
}

// Bring-up (MODE 1): exercise ONE leg so you can confirm the signs.
// Watch the foot:
//   "hip FORWARD" must move the foot toward the FRONT of the robot.
//   "knee UP"     must lift the foot off the ground.
// If either is backwards, flip that leg's sign in HIP_FWD[] / KNEE_UP[].
static inline void testLeg(int leg) {
    printf("\n--- LEG %d  (hip servo %d, knee servo %d) ---\n", leg, HIP_IDX[leg], KNEE_IDX[leg]);
    printf("hip FORWARD (foot should move toward FRONT)\n");
    gSwing[leg] = +1; applyTargets(150); vTaskDelay(pdMS_TO_TICKS(220));
    printf("hip BACK\n");
    gSwing[leg] = -1; applyTargets(180); vTaskDelay(pdMS_TO_TICKS(220));
    gSwing[leg] = 0;  applyTargets(150);
    printf("knee UP (foot should LIFT off the ground)\n");
    gLift[leg] = 1;   applyTargets(150); vTaskDelay(pdMS_TO_TICKS(220));
    printf("knee DOWN\n");
    gLift[leg] = 0;   applyTargets(150); vTaskDelay(pdMS_TO_TICKS(260));
}

// One full forward stride = both diagonal pairs take a turn. Call this in a
// tight loop (no extra delay) for a continuous, fast trot.
void walkForwardOneStep(void) {
    trotHalfStep(0);
    trotHalfStep(1);
}

// =============================================================
// ====== HIGH-LEVEL ROUTINES  (call these one-by-one in main) =
// =============================================================

// Walk forward ~`meters` metres as a fast continuous trot, then square up.
// Distance comes from CYCLES_PER_METER -- calibrate that once on the floor.
void walkForwardMeters(float meters) {
    gHipSign = HIP_FWD;                                // straight-walk hip signs
    standAndSettle();                                  // stand up / start clean
    int cycles = (int)(meters * CYCLES_PER_METER + 0.5f);
    for (int i = 0; i < cycles; i++) walkForwardOneStep();
    standAndSettle();                                  // stop squared up
}

// Rotate in place: the SAME trot, but with the all-same hip signs so the body
// spins on the spot instead of translating. Then square up for the next walk.
void rotateInPlace(int dir) {
    (void)dir;                                         // one direction for now
    gHipSign = HIP_ROT;                                // all-same signs -> spin
    standAndSettle();
    for (int i = 0; i < TURN_CYCLES; i++) trotHalfStep(i & 1);
    gHipSign = HIP_FWD;                                // restore for walking
    standAndSettle();
}

// Lower the rear and sit, then hold the pose.
// (Back knees fold so the rear drops; front legs stay planted. Tune angles.)
void sitDown(void) {
    standAndSettle();
    int sit[8];
    sit[R1] = 90;  sit[R2] = 45;  sit[L1] = 110; sit[L2] = 90;  // hips ~ stand
    sit[R3] = 70;  sit[R4] = 70;                                // back knees fold -> rear down
    sit[L3] = 0;   sit[L4] = 90;                                // front knees planted -> front up
    moveAllTo(sit, 500);
    vTaskDelay(pdMS_TO_TICKS(1500));                            // hold the sit
}

// Lay the BACK legs flat on the ground, then SEE-SAW the two front legs
// (one up while the other is down, alternating) FRONT_WAVES times, then
// stand back up so walking can continue.
static int FRONT_WAVES = 5;   // how many up/down cycles the front legs do
void frontPawWave(void) {
    standAndSettle(); // Start from a perfectly balanced standing pose

    // 1) Initialize our target pose array with current standing positions
    int pose[8];
    for (int i = 0; i < 8; i++) {
        pose[i] = gCur[i]; 
    }

    // 2) Drop the rear legs flat to the ground so the rear rests safely
    // Using KNEE_UP direction to safely flatten the back legs (BL=2, BR=3)
    pose[KNEE_IDX[LEG_BL]] = KNEE_STAND[LEG_BL] - (KNEE_UP[LEG_BL] * 65); // Fold flat
    pose[KNEE_IDX[LEG_BR]] = KNEE_STAND[LEG_BR] - (KNEE_UP[LEG_BR] * 65); // Fold flat
    moveAllTo(pose, 600);
    vTaskDelay(pdMS_TO_TICKS(200));

    // 3) See-saw the FRONT legs (LEG_FL and LEG_FR) using calibrated direction signs
    int fl_knee = KNEE_IDX[LEG_FL];
    int fr_knee = KNEE_IDX[LEG_FR];
    
    // Calculate distinct high (lifted) and low (planted) angles using KNEE_UP signs
    int fl_up   = KNEE_STAND[LEG_FL] + (KNEE_UP[LEG_FL] * 50); // 50 degrees upward lift
    int fl_down = KNEE_STAND[LEG_FL]; 
    
    int fr_up   = KNEE_STAND[LEG_FR] + (KNEE_UP[LEG_FR] * 50); // 50 degrees upward lift
    int fr_down = KNEE_STAND[LEG_FR];

    for (int i = 0; i < FRONT_WAVES; i++) {
        // Wave 1: FL Up, FR Down
        pose[fl_knee] = fl_up;
        pose[fr_knee] = fr_down;
        moveAllTo(pose, 250);
        
        // Wave 2: FL Down, FR Up
        pose[fl_knee] = fl_down;
        pose[fr_knee] = fr_up;
        moveAllTo(pose, 250);
    }

    // Put both front feet back down on the ground flat
    pose[fl_knee] = fl_down;
    pose[fr_knee] = fr_down;
    moveAllTo(pose, 250);

    // 4) Stand completely back up to reset the state machine
    standAndSettle();
}

static inline void runRestPose(void) { 
    printf("RIZZOTTO POSING: REST\n"); 
    setFaceWithMode("rest", FACE_ANIM_BOOMERANG); 
    for (int i = 0; i < 8; i++) set_servo_angle(i, 90); 
}


// ===================================
// ======= MOVEMENT ENGINES ==========
// ===================================

static inline void runWalkPose(void) {
    printf("RIZZOTTO MOVING: WALK FWD\n");
    setFaceWithMode("walk", FACE_ANIM_ONCE);
    
    // Initial starting pose
    /*set_servo_angle(R3, 135); set_servo_angle(L3, 45);
    set_servo_angle(R2, 100); set_servo_angle(L1, 25);
    vTaskDelay(pdMS_TO_TICKS(frameDelay)); // Give servos time to reach the starting pose*/
    
    // Loop through the walking cycles continuously
    for (int i = 0; i < walkCycles; i++) {
        
        set_servo_angle(R1, 120); // forward
        vTaskDelay(pdMS_TO_TICKS(frameDelay)); 
        set_servo_angle(R1, 60);  // drag backward
        vTaskDelay(pdMS_TO_TICKS(frameDelay));
        set_servo_angle(L2, 30);  // L2 supports
        vTaskDelay(pdMS_TO_TICKS(frameDelay));
        set_servo_angle(R3, 90);   // lift
        vTaskDelay(pdMS_TO_TICKS(frameDelay));  
        set_servo_angle(R1, 120);  // back to origin
        vTaskDelay(pdMS_TO_TICKS(frameDelay));

    }
}

