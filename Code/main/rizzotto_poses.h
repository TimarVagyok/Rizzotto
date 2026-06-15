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
static inline void runStandPose(int face);

// =============================
// ========== POSES ============
// =============================

static inline void runStandPose(int face) { 
    printf("RIZZOTTO POSING: STAND\n"); 
    if (face == 1) setFaceWithMode("stand", FACE_ANIM_ONCE); 
    set_servo_angle(R1, 135); 
    set_servo_angle(R2, 45); 
    set_servo_angle(L1, 45); 
    set_servo_angle(L2, 135); 
    set_servo_angle(R4, 0); 
    set_servo_angle(R3, 180); 
    set_servo_angle(L3, 0); 
    set_servo_angle(L4, 180); 
    if (face == 1) enterIdle();
}

static inline void runRestPose(void) { 
    printf("RIZZOTTO POSING: REST\n"); 
    setFaceWithMode("rest", FACE_ANIM_BOOMERANG); 
    for (int i = 0; i < 8; i++) set_servo_angle(i, 90); 
}

static inline void runWavePose(void) { 
    printf("RIZZOTTO POSING: WAVE\n"); 
    setFaceWithMode("wave", FACE_ANIM_ONCE); 
    runStandPose(0); 
    delayWithFace(200);
    set_servo_angle(R4, 80);  set_servo_angle(L3, 180); 
    set_servo_angle(L2, 90);  set_servo_angle(R1, 100); 
    delayWithFace(200);
    set_servo_angle(L3, 180); 
    delayWithFace(300); 
    for (int i = 0; i < 4; i++) { 
        set_servo_angle(L3, 180); delayWithFace(300); 
        set_servo_angle(L3, 100); delayWithFace(300); 
    } 
    runStandPose(1); 
    if (strcmp(currentCommand, "wave") == 0) currentCommand[0] = '\0';
}

static inline void runDancePose(void) { 
    printf("RIZZOTTO POSING: DANCE\n"); 
    setFaceWithMode("dance", FACE_ANIM_LOOP); 
    set_servo_angle(R1, 90);  set_servo_angle(R2, 90); 
    set_servo_angle(L1, 90);  set_servo_angle(L2, 90); 
    set_servo_angle(R4, 160); set_servo_angle(R3, 160); 
    set_servo_angle(L3, 10);  set_servo_angle(L4, 10); 
    delayWithFace(300); 
    for (int i = 0; i < 5; i++) { 
        set_servo_angle(R4, 115); set_servo_angle(R3, 115); 
        set_servo_angle(L3, 10);  set_servo_angle(L4, 10); 
        delayWithFace(300); 
        set_servo_angle(R4, 160); set_servo_angle(R3, 160); 
        set_servo_angle(L3, 65);  set_servo_angle(L4, 65); 
        delayWithFace(300); 
    } 
    runStandPose(1); 
    if (strcmp(currentCommand, "dance") == 0) currentCommand[0] = '\0';
}

static inline void runSwimPose(void) { 
    printf("RIZZOTTO POSING: SWIM\n"); 
    setFaceWithMode("swim", FACE_ANIM_ONCE); 
    for (int i = 0; i < 8; i++) set_servo_angle(i, 90); 
    for (int i = 0; i < 4; i++) { 
        set_servo_angle(R1, 135); set_servo_angle(R2, 45); 
        set_servo_angle(L1, 45);  set_servo_angle(L2, 135); 
        delayWithFace(400); 
        set_servo_angle(R1, 90);  set_servo_angle(R2, 90); 
        set_servo_angle(L1, 90);  set_servo_angle(L2, 90); 
        delayWithFace(400); 
    } 
    runStandPose(1); 
    if (strcmp(currentCommand, "swim") == 0) currentCommand[0] = '\0';
}

static inline void runPointPose(void) { 
    printf("RIZZOTTO POSING: POINT\n"); 
    setFaceWithMode("point", FACE_ANIM_BOOMERANG); 
    set_servo_angle(L2, 90);  set_servo_angle(R1, 135); 
    set_servo_angle(R2, 100); set_servo_angle(L4, 180); 
    set_servo_angle(L1, 25);  set_servo_angle(L3, 145);
    set_servo_angle(R4, 80);  set_servo_angle(R3, 170); 
    delayWithFace(2000); 
    runStandPose(1); 
    if (strcmp(currentCommand, "point") == 0) currentCommand[0] = '\0';
}

static inline void runPushupPose(void) {
    printf("RIZZOTTO POSING: PUSHUP\n");
    setFaceWithMode("pushup", FACE_ANIM_ONCE);
    runStandPose(0); 
    delayWithFace(200);
    set_servo_angle(L1, 0);    set_servo_angle(R1, 180);
    set_servo_angle(L3, 90);   set_servo_angle(R3, 90);
    delayWithFace(500);
    for (int i = 0; i < 4; i++) {
        set_servo_angle(L3, 0);   set_servo_angle(R3, 180);
        delayWithFace(600);
        set_servo_angle(L3, 90);  set_servo_angle(R3, 90);
        delayWithFace(500);
    }
    runStandPose(1);
    if (strcmp(currentCommand, "pushup") == 0) currentCommand[0] = '\0';
}

static inline void runBowPose(void) {
    printf("RIZZOTTO POSING: BOW\n");
    setFaceWithMode("bow", FACE_ANIM_ONCE);
    runStandPose(0); 
    delayWithFace(200);
    set_servo_angle(L1, 0);    set_servo_angle(R1, 180);
    set_servo_angle(L3, 0);    set_servo_angle(R3, 180);
    set_servo_angle(L2, 180);  set_servo_angle(R2, 0);
    set_servo_angle(R4, 0);    set_servo_angle(L4, 180);
    delayWithFace(600);
    set_servo_angle(L3, 90);   set_servo_angle(R3, 90);
    delayWithFace(3000);
    runStandPose(1);
    if (strcmp(currentCommand, "bow") == 0) currentCommand[0] = '\0';
}

static inline void runCutePose(void) {
    printf("RIZZOTTO POSING: CUTE\n");
    setFaceWithMode("cute", FACE_ANIM_ONCE);
    runStandPose(0); 
    delayWithFace(200);
    set_servo_angle(L2, 160); set_servo_angle(R2, 20);
    set_servo_angle(R4, 180); set_servo_angle(L4, 0);
    set_servo_angle(L1, 0);   set_servo_angle(R1, 180);
    set_servo_angle(L3, 180); set_servo_angle(R3, 0);
    delayWithFace(200);
    for (int i = 0; i < 5; i++) {
        set_servo_angle(R4, 180); set_servo_angle(L4, 45);
        delayWithFace(300);
        set_servo_angle(R4, 135); set_servo_angle(L4, 0);
        delayWithFace(300);
    }
    runStandPose(1);
    if (strcmp(currentCommand, "cute") == 0) currentCommand[0] = '\0';
}

static inline void runWormPose(void) {
    printf("RIZZOTTO POSING: WORM\n");
    setFaceWithMode("worm", FACE_ANIM_ONCE);
    runStandPose(0);
    delayWithFace(200);
    set_servo_angle(R1, 180); set_servo_angle(R2, 0);   set_servo_angle(L1, 0);   set_servo_angle(L2, 180);
    set_servo_angle(R4, 90);  set_servo_angle(R3, 90);  set_servo_angle(L3, 90);  set_servo_angle(L4, 90);
    delayWithFace(200);
    for(int i=0; i<5; i++) {
        set_servo_angle(R3, 45);  set_servo_angle(L3, 135); set_servo_angle(R4, 45);  set_servo_angle(L4, 135);
        delayWithFace(300);
        set_servo_angle(R3, 135); set_servo_angle(L3, 45);  set_servo_angle(R4, 135); set_servo_angle(L4, 45);
        delayWithFace(300);
    }
    runStandPose(1);
    if (strcmp(currentCommand, "worm") == 0) currentCommand[0] = '\0';
}

static inline void runDeadPose(void) {
    printf("RIZZOTTO POSING: DEAD\n");
    runStandPose(0);
    setFaceWithMode("dead", FACE_ANIM_BOOMERANG);
    delayWithFace(200);
    set_servo_angle(R3, 90); set_servo_angle(R4, 90); set_servo_angle(L3, 90); set_servo_angle(L4, 90);
    if (strcmp(currentCommand, "dead") == 0) currentCommand[0] = '\0';
}

// ===================================
// ======= MOVEMENT ENGINES ==========
// ===================================

static inline void runWalkPose(void) {
    printf("RIZZOTTO MOVING: WALK FWD\n");
    setFaceWithMode("walk", FACE_ANIM_ONCE);
    set_servo_angle(R3, 135); set_servo_angle(L3, 45);
    set_servo_angle(R2, 100); set_servo_angle(L1, 25);
    if (!pressingCheck("forward", frameDelay)) return;
    
    for (int i = 0; i < walkCycles; i++) {
        set_servo_angle(R3, 135); set_servo_angle(L3, 0);
        if (!pressingCheck("forward", frameDelay)) return;
        set_servo_angle(L4, 135); set_servo_angle(L2, 90);
        set_servo_angle(R4, 0);   set_servo_angle(R1, 180);
        if (!pressingCheck("forward", frameDelay)) return;    
        set_servo_angle(R2, 45);  set_servo_angle(L1, 90);
        if (!pressingCheck("forward", frameDelay)) return;
        set_servo_angle(R4, 45);  set_servo_angle(L4, 180);
        if (!pressingCheck("forward", frameDelay)) return;
        set_servo_angle(R3, 180); set_servo_angle(L3, 45);
        set_servo_angle(R2, 90);  set_servo_angle(L1, 0);
        if (!pressingCheck("forward", frameDelay)) return;  
        set_servo_angle(L2, 135); set_servo_angle(R1, 90);
        if (!pressingCheck("forward", frameDelay)) return;
    }
    runStandPose(1);
}

static inline void runWalkBackward(void) {
    printf("RIZZOTTO MOVING: WALK BACK\n");
    setFaceWithMode("walk", FACE_ANIM_ONCE);
    if (!pressingCheck("backward", frameDelay)) return;
    
    for (int i = 0; i < walkCycles; i++) {
        set_servo_angle(R3, 135); set_servo_angle(L3, 0);
        if (!pressingCheck("backward", frameDelay)) return;
        set_servo_angle(L4, 135); set_servo_angle(L2, 135);
        set_servo_angle(R4, 0);   set_servo_angle(R1, 90);
        if (!pressingCheck("backward", frameDelay)) return;    
        set_servo_angle(R2, 90);  set_servo_angle(L1, 0);
        if (!pressingCheck("backward", frameDelay)) return;
        set_servo_angle(R4, 45);  set_servo_angle(L4, 180);
        if (!pressingCheck("backward", frameDelay)) return;
        set_servo_angle(R3, 180); set_servo_angle(L3, 45);
        set_servo_angle(R2, 45);  set_servo_angle(L1, 90);
        if (!pressingCheck("backward", frameDelay)) return;  
        set_servo_angle(L2, 90);  set_servo_angle(R1, 180);
        if (!pressingCheck("backward", frameDelay)) return;
    }
    runStandPose(1);
}

static inline void runTurnLeft(void) {
    printf("RIZZOTTO MOVING: TURN LEFT\n");
    setFaceWithMode("walk", FACE_ANIM_ONCE);
    for (int i = 0; i < walkCycles; i++) {
        set_servo_angle(R3, 135); set_servo_angle(L4, 135); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R1, 180); set_servo_angle(L2, 180); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R3, 180); set_servo_angle(L4, 180); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R1, 135); set_servo_angle(L2, 135);
        if (!pressingCheck("left", frameDelay)) return;
        
        set_servo_angle(R4, 45);  set_servo_angle(L3, 45); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R2, 90);  set_servo_angle(L1, 90); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R4, 0);   set_servo_angle(L3, 0); 
        if (!pressingCheck("left", frameDelay)) return;
        set_servo_angle(R2, 45);  set_servo_angle(L1, 45);
        if (!pressingCheck("left", frameDelay)) return;  
    }
    runStandPose(1);
}

static inline void runTurnRight(void) {
    printf("RIZZOTTO MOVING: TURN RIGHT\n");
    setFaceWithMode("walk", FACE_ANIM_ONCE);
    for (int i = 0; i < walkCycles; i++) {
        set_servo_angle(R4, 45);  set_servo_angle(L3, 45); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R2, 0);   set_servo_angle(L1, 0); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R4, 0);   set_servo_angle(L3, 0); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R2, 45);  set_servo_angle(L1, 45);
        if (!pressingCheck("right", frameDelay)) return;  
        
        set_servo_angle(R3, 135); set_servo_angle(L4, 135); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R1, 90);  set_servo_angle(L2, 90); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R3, 180); set_servo_angle(L4, 180); 
        if (!pressingCheck("right", frameDelay)) return;
        set_servo_angle(R1, 135); set_servo_angle(L2, 135);
        if (!pressingCheck("right", frameDelay)) return;
    }
    runStandPose(1);
}