/**
 * @file RainEffect.cpp
 * @brief Implementation of the RainEffect class for LED matrix rain animation
 * 
 * This file contains the implementation of the RainEffect class, which creates
 * a realistic rain animation with falling raindrops and ground impact effects
 * on the LED matrix display.
 * 
 * @author zeevy
 * @version 1.0.0
 * @date 2025-09-06
 * @license MIT
 */

#include "RainEffect.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

/**
 * @brief Constructor for RainEffect
 * @param matrix Reference to the LED matrix display object
 * 
 * Initializes the rain effect system by setting up the member variables
 * and deactivating all raindrops and ground flashes.
 */
RainEffect::RainEffect(Max72xxPanel& matrix) 
  : ledMatrix(matrix), lastRaindropSpawnTime(0), isInitializedFlag(false) {
  // Initialize all raindrops as inactive
  for (int i = 0; i < MAX_RAINDROPS; i++) {
    raindropArray[i].isActive = false;
  }
  
  // Initialize all ground flashes as inactive
  for (int i = 0; i < MAX_GROUND_FLASHES; i++) {
    groundFlashArray[i].isActive = false;
  }
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

/**
 * @brief Initialize the rain effect system
 * 
 * Sets up the spawn timer and marks the system as initialized.
 * This method is safe to call multiple times and will not re-initialize
 * if already initialized.
 */
void RainEffect::initialize() {
  // Arrays are already initialized in constructor, no need to re-initialize
  lastRaindropSpawnTime = millis();
  isInitializedFlag = true;
}

/**
 * @brief Update the rain effect logic
 * 
 * This method handles the main rain effect logic including:
 * - Spawning new raindrops at regular intervals
 * - Updating falling raindrops
 * - Managing ground flash effects
 * 
 * Should be called every loop iteration for smooth animation.
 */
void RainEffect::update() {
  // Exit early if not initialized
  if (!isInitializedFlag) return;
  
  unsigned long currentTime = millis();

  // Spawn new raindrops at regular intervals
  if ((unsigned long)(currentTime - lastRaindropSpawnTime) >= RAIN_SPAWN_INTERVAL_MS) {
    spawnNewRaindrop();
    lastRaindropSpawnTime = currentTime;
  }
  
  // Update all active raindrops and ground flashes
  updateFallingRaindrops();
  updateGroundImpactFlashes();
}

/**
 * @brief Render the rain effect to the LED matrix
 * 
 * This method draws all active raindrops and ground flashes to the LED matrix.
 * It includes optimizations to only clear and redraw when there are active
 * elements, improving performance.
 */
void RainEffect::render() {
  // Exit early if not initialized
  if (!isInitializedFlag) return;
  
  // Optimization: Only clear screen if there are active elements to draw
  bool hasActiveElements = false;
  
  // Check if there are any active raindrops
  for (int raindropIndex = 0; raindropIndex < MAX_RAINDROPS; raindropIndex++) {
    if (raindropArray[raindropIndex].isActive) {
      hasActiveElements = true;
      break;
    }
  }
  
  // Check if there are any active ground flashes
  if (!hasActiveElements) {
    for (int flashIndex = 0; flashIndex < MAX_GROUND_FLASHES; flashIndex++) {
      if (groundFlashArray[flashIndex].isActive) {
        hasActiveElements = true;
        break;
      }
    }
  }
  
  // Only clear and redraw if there are active elements
  if (hasActiveElements) {
    // Clear the display
    ledMatrix.fillScreen(LOW);

    // Draw all active raindrops
    for (int raindropIndex = 0; raindropIndex < MAX_RAINDROPS; raindropIndex++) {
      if (raindropArray[raindropIndex].isActive) {
        ledMatrix.drawPixel(raindropArray[raindropIndex].positionX, 
                           raindropArray[raindropIndex].positionY, HIGH);
      }
    }

    // Draw all ground flashes with visual effects
    for (int flashIndex = 0; flashIndex < MAX_GROUND_FLASHES; flashIndex++) {
      if (groundFlashArray[flashIndex].isActive) {
        // Draw flash at ground level (bottom row)
        int groundYPosition = ledMatrix.height() - 1;

        // Draw main flash point
        ledMatrix.drawPixel(groundFlashArray[flashIndex].positionX, groundYPosition, HIGH);

        // Draw spread effect for more dramatic flash (high intensity)
        if (groundFlashArray[flashIndex].brightnessIntensity > 10) {
          // Left spread
          if (groundFlashArray[flashIndex].positionX > 0) {
            ledMatrix.drawPixel(groundFlashArray[flashIndex].positionX - 1, groundYPosition, HIGH);
          }
          // Right spread
          if (groundFlashArray[flashIndex].positionX < ledMatrix.width() - 1) {
            ledMatrix.drawPixel(groundFlashArray[flashIndex].positionX + 1, groundYPosition, HIGH);
          }
        }

        // Draw upward splash effect (medium intensity)
        if (groundFlashArray[flashIndex].brightnessIntensity > 5 && groundYPosition > 0) {
          ledMatrix.drawPixel(groundFlashArray[flashIndex].positionX, groundYPosition - 1, HIGH);
        }
      }
    }

    // Update the display
    ledMatrix.write();
  }
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

/**
 * @brief Spawn a new raindrop at the top of the display
 * 
 * Finds an available raindrop slot and initializes it with:
 * - Random X position across the display width
 * - Y position at the top (0)
 * - Random fall speed within the configured range
 * - Current timestamp for movement timing
 */
void RainEffect::spawnNewRaindrop() {
  // Find an inactive raindrop slot
  for (int raindropIndex = 0; raindropIndex < MAX_RAINDROPS; raindropIndex++) {
    if (!raindropArray[raindropIndex].isActive) {
      // Initialize raindrop properties
      raindropArray[raindropIndex].positionX = random(ledMatrix.width());
      raindropArray[raindropIndex].positionY = 0;  // Start at top of display
      raindropArray[raindropIndex].fallSpeedMs = random(RAIN_FALL_SPEED_MIN_MS, RAIN_FALL_SPEED_MAX_MS + 1);
      raindropArray[raindropIndex].isActive = true;
      raindropArray[raindropIndex].lastMoveTime = millis();
      break;
    }
  }
}

/**
 * @brief Update all falling raindrops
 * 
 * This method moves all active raindrops down the display based on their
 * individual fall speeds. When a raindrop reaches the bottom, it creates
 * a ground impact flash and deactivates the raindrop.
 */
void RainEffect::updateFallingRaindrops() {
  unsigned long currentTime = millis();

  // Update each active raindrop
  for (int raindropIndex = 0; raindropIndex < MAX_RAINDROPS; raindropIndex++) {
    if (raindropArray[raindropIndex].isActive) {
      // Check if it's time to move this raindrop based on its fall speed
      if ((unsigned long)(currentTime - raindropArray[raindropIndex].lastMoveTime) >= 
          (unsigned long)raindropArray[raindropIndex].fallSpeedMs) {
        
        // Move raindrop down one pixel
        raindropArray[raindropIndex].positionY++;
        raindropArray[raindropIndex].lastMoveTime = currentTime;

        // Check if raindrop hit the ground (bottom of display)
        if (raindropArray[raindropIndex].positionY >= ledMatrix.height()) {
          // Create ground impact flash effect
          createGroundImpactFlash(raindropArray[raindropIndex].positionX);

          // Deactivate this raindrop
          raindropArray[raindropIndex].isActive = false;
        }
      }
    }
  }
}

/**
 * @brief Create a ground impact flash at the specified position
 * @param xPosition X coordinate where the raindrop hit the ground
 * 
 * Creates a visual flash effect when a raindrop hits the ground.
 * The flash starts at maximum brightness and fades out over time.
 */
void RainEffect::createGroundImpactFlash(int xPosition) {
  // Find an inactive ground flash slot
  for (int flashIndex = 0; flashIndex < MAX_GROUND_FLASHES; flashIndex++) {
    if (!groundFlashArray[flashIndex].isActive) {
      // Initialize ground flash properties
      groundFlashArray[flashIndex].positionX = xPosition;
      groundFlashArray[flashIndex].brightnessIntensity = 15;  // Maximum brightness
      groundFlashArray[flashIndex].flashStartTime = millis();
      groundFlashArray[flashIndex].isActive = true;
      break;
    }
  }
}

/**
 * @brief Update all ground flash effects
 * 
 * This method updates the brightness of all active ground flashes
 * based on their age. Flashes fade out over time and are deactivated
 * when their duration expires.
 */
void RainEffect::updateGroundImpactFlashes() {
  unsigned long currentTime = millis();

  // Update each active ground flash
  for (int flashIndex = 0; flashIndex < MAX_GROUND_FLASHES; flashIndex++) {
    if (groundFlashArray[flashIndex].isActive) {
      unsigned long elapsedTime = (unsigned long)(currentTime - groundFlashArray[flashIndex].flashStartTime);

      // Check if flash duration has expired
      if (elapsedTime >= GROUND_FLASH_DURATION_MS) {
        // Flash duration expired, deactivate
        groundFlashArray[flashIndex].isActive = false;
      } else {
        // Calculate fade intensity (fade out over time)
        // Safety check to prevent division by zero
        if (GROUND_FLASH_DURATION_MS > 0) {
          float fadeRatio = 1.0 - (float)elapsedTime / GROUND_FLASH_DURATION_MS;
          groundFlashArray[flashIndex].brightnessIntensity = (int)(15 * fadeRatio);
        } else {
          // Fallback: set to maximum brightness if duration is invalid
          groundFlashArray[flashIndex].brightnessIntensity = 15;
        }
      }
    }
  }
}
