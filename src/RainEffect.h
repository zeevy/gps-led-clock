/**
 * @file RainEffect.h
 * @brief Rain animation effect for LED matrix display
 * 
 * This file contains the RainEffect class and related structures for creating
 * a realistic rain animation on the LED matrix. The effect includes falling
 * raindrops and ground impact flashes when raindrops hit the bottom.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-06
 * @license MIT
 */

#ifndef RAIN_EFFECT_H
#define RAIN_EFFECT_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct RainDrop
 * @brief Represents a single raindrop in the rain animation
 * 
 * This structure holds all the necessary data for a single raindrop including
 * its position, movement speed, and state information.
 */
struct RainDrop {
  int positionX;           /**< X coordinate of the raindrop */
  int positionY;           /**< Y coordinate of the raindrop */
  int fallSpeedMs;         /**< Time in milliseconds between each fall movement */
  bool isActive;           /**< Whether this raindrop is currently active */
  unsigned long lastMoveTime; /**< Timestamp of the last movement */
};

/**
 * @struct GroundFlash
 * @brief Represents a ground flash effect when raindrop hits the ground
 * 
 * This structure manages the visual effect that occurs when a raindrop
 * impacts the ground, including position and brightness fade.
 */
struct GroundFlash {
  int positionX;           /**< X coordinate of the flash effect */
  int brightnessIntensity; /**< Current brightness intensity (0-15) */
  int initialIntensity;    /**< Initial brightness intensity for proper fading */
  unsigned long flashStartTime; /**< Timestamp when the flash started */
  bool isActive;           /**< Whether this flash is currently active */
};

// ============================================================================
// RAIN EFFECT CLASS
// ============================================================================

/**
 * @class RainEffect
 * @brief Manages the rain animation effect for the LED matrix display
 * 
 * This class creates a realistic rain animation with falling raindrops and
 * ground impact effects. It manages multiple raindrops simultaneously and
 * creates visual effects when they hit the ground.
 * 
 * Features:
 * - Multiple simultaneous raindrops with random speeds
 * - Ground impact flash effects
 * - Configurable spawn rates and fall speeds
 * - Optimized rendering to reduce screen clearing
 * 
 * @note This class is designed to be used when GPS signal is lost or time is not acquired
 */
class RainEffect {
public:
  // ========================================================================
  // CONSTRUCTOR
  // ========================================================================
  
  /**
   * @brief Constructor for RainEffect
   * @param matrix Reference to the LED matrix display object
   */
  RainEffect(Max72xxPanel& matrix);
  
  // ========================================================================
  // PUBLIC METHODS
  // ========================================================================
  
  /**
   * @brief Initialize the rain effect system
   * 
   * This method initializes the rain effect by setting up the spawn timer
   * and marking the system as initialized. It should be called before
   * using update() and render() methods.
   * 
   * @note Safe to call multiple times - will not re-initialize if already initialized
   */
  void initialize();
  
  /**
   * @brief Update the rain effect logic
   * 
   * This method should be called in the main loop to update the rain effect.
   * It handles spawning new raindrops, updating falling raindrops, and
   * managing ground flash effects.
   * 
   * @note Call this method every loop iteration for smooth animation
   */
  void update();
  
  /**
   * @brief Render the rain effect to the LED matrix
   * 
   * This method draws all active raindrops and ground flashes to the
   * LED matrix display. It includes optimizations to only clear and
   * redraw when necessary.
   * 
   * @note Call this method after update() to display the current frame
   */
  void render();
  
  /**
   * @brief Check if the rain effect is initialized
   * @return True if initialized, false otherwise
   */
  bool isInitialized() const { return isInitializedFlag; }
  
private:
  // ========================================================================
  // CONSTANTS
  // ========================================================================
  
  /** Maximum number of simultaneous raindrops */
  static const int MAX_RAINDROPS = 8;
  
  /** Maximum number of simultaneous ground flashes */
  static const int MAX_GROUND_FLASHES = 6;
  
  /** Time interval between spawning new raindrops (milliseconds) */
  static const int RAIN_SPAWN_INTERVAL_MS = 250;
  
  /** Minimum fall speed for raindrops (milliseconds between moves) */
  static const int RAIN_FALL_SPEED_MIN_MS = 80;
  
  /** Maximum fall speed for raindrops (milliseconds between moves) */
  static const int RAIN_FALL_SPEED_MAX_MS = 150;
  
  /** Duration of ground flash effect (milliseconds) */
  static const int GROUND_FLASH_DURATION_MS = 300;
  
  // ========================================================================
  // MEMBER VARIABLES
  // ========================================================================
  
  /** Reference to the LED matrix display object */
  Max72xxPanel& ledMatrix;
  
  /** Array of raindrop objects */
  RainDrop raindropArray[MAX_RAINDROPS];
  
  /** Array of ground flash objects */
  GroundFlash groundFlashArray[MAX_GROUND_FLASHES];
  
  /** Timestamp of the last raindrop spawn */
  unsigned long lastRaindropSpawnTime;
  
  /** Initialization flag */
  bool isInitializedFlag;
  
  // ========================================================================
  // PRIVATE METHODS
  // ========================================================================
  
  /**
   * @brief Spawn a new raindrop at the top of the display
   * 
   * This method finds an available raindrop slot and initializes it
   * with a random X position and fall speed.
   */
  void spawnNewRaindrop();
  
  /**
   * @brief Update all falling raindrops
   * 
   * This method moves all active raindrops down the display and handles
   * ground impact when they reach the bottom.
   */
  void updateFallingRaindrops();
  
  /**
   * @brief Create a ground impact flash at the specified position
   * @param xPosition X coordinate where the raindrop hit the ground
   */
  void createGroundImpactFlash(int xPosition);
  
  /**
   * @brief Update all ground flash effects
   * 
   * This method updates the brightness of all active ground flashes
   * and removes them when their duration expires.
   */
  void updateGroundImpactFlashes();
};

#endif // RAIN_EFFECT_H
