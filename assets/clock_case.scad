/*
 * Wall Clock Case for LED Matrix Display
 *
 * A 3D printable case designed for a 4x MAX7219 LED matrix display
 * with integrated GPS module mounting and ventilation system.
 *
 * Features:
 * - Customizable dimensions for different LED matrix sizes
 * - Integrated ventilation system for heat dissipation (optional cross-flow)
 * - GPS module mounting (Neo-6M compatible)
 * - M3 screw mounting system
 * - Rounded corners
 * - Modular design for easy customization
 *
 * Author: zeevy
 * License: MIT
 * Version: 1.1
 *
 * Dependencies:
 * - OpenSCAD 2019.05 or later
 *
 * Usage:
 * - Toggle the render flags in the Assembly section to choose what to export.
 * - Adjust parameters in the Configuration section as needed.
 * - Use $preview = true for fast preview, false for final render.
 */

// ============================================================================
// CONFIGURATION PARAMETERS
// ============================================================================

// Rendering quality
$fn = $preview ? 30 : 96;

// Render epsilon. Keeps boolean cuts from leaving zero-thickness faces.
// Tuned separately from physical assembly clearance (see back_panel_fit_tolerance).
eps = 0.05;

// Material and manufacturing parameters
wall_thickness        = 3.5;    // Thickness of case walls
screen_thickness      = 0.64;   // Thickness of screen/display area

// LED Matrix Display dimensions (MAX7219 4x configuration)
// Module is 128 x 33 x 15 mm (L x W x H) per the Handson Technology DSP-1172
// datasheet. Each dimension below is the module size plus a print clearance;
// the base numbers must stay equal to the datasheet, not to the 32 mm the
// matrix area alone measures, or the pocket ends up a press fit.
led_matrix_clearance_xy = 1.0;   // Clearance added to length and width
led_matrix_clearance_z  = 0.5;   // Clearance added to height
led_matrix_length     = 128 + led_matrix_clearance_xy;  // Module length + clearance
led_matrix_width      = 33  + led_matrix_clearance_xy;  // Module width + clearance
led_matrix_height     = 15  + led_matrix_clearance_z;   // Module height + clearance

// Main case dimensions
case_length           = 150;    // Total length of the case
case_width            = 46;     // Total width of the case
case_height           = 32;     // Total height of the case
case_bottom_thickness = 5;      // Thickness of bottom panel that holds the LED matrix
case_corner_radius    = 2;      // Corner radius for case and panel geometry

// Back panel fit. Total radial clearance between panel and its recess in the case.
back_panel_fit_tolerance = 0.3;

// Ventilation
ventilation_hole_diameter   = 1.26;
ventilation_slot_length     = 16;
ventilation_hole_spacing    = 3.8;
ventilation_hole_count      = 8;
ventilation_enable_back_wall = true;  // Mirror vents to opposite long wall for cross-flow

// GPS antenna (Neo-6M is square ~25mm)
gps_antenna_size            = 25.25;
gps_module_cutout_inset_x   = 40;   // Distance from case end (back panel antenna cutout)
gps_module_bracket_inset_x  = 42;   // Distance from case end (back panel bracket)
gps_post_origin_x           = 96;   // X offset for Neo-6M PCB mounting posts (back panel frame)
gps_post_spacing_x          = 31;   // PCB hole spacing in X (Neo-6M board)
gps_post_spacing_y          = 20.6; // PCB hole spacing in Y (Neo-6M board)

// Hardware
m3_screw_tap_hole_diameter         = 2.8;   // Self-tap hole in mounting feet
m3_screw_shaft_clearance_diameter  = 3.25;  // Through-hole in back panel
m3_screw_distance_from_edge        = 5.5;   // Distance from case edge to mounting feet/holes
m3_screw_head_recess_diameter      = 6;     // Counterbore diameter for screw head
// Counterbore depth into the back panel. With 2.0mm depth in a 3.5mm panel, a
// standard 3mm cap head will sit ~1mm proud. Increase toward ~3.0mm if you use
// flat/countersunk M3 screws (panel still needs material below the recess).
m3_screw_head_recess_depth         = 2;

// Cable management
cable_pass_through_diameter        = 4.5;
cable_pass_through_inset_x         = 12;  // Distance from each end of the back panel
cable_pass_through_y               = 4.6;

// Mounting bars on the underside of the case
foot_bar_inset_x                   = 15;

// Display ring
display_ring_thickness             = 2.4;  // Z thickness of the ring
display_ring_border                = 4.2;  // XY width of the ring border

// Assembly render toggles
render_case        = false;
render_back_panel  = false;
render_display_ring = true;

// ============================================================================
// SANITY ASSERTIONS
// ============================================================================

assert(case_length > led_matrix_length + 2 * wall_thickness,
       "case_length must accommodate led_matrix_length plus side walls");
assert(case_width > led_matrix_width + 2 * wall_thickness,
       "case_width must accommodate led_matrix_width plus side walls");
assert(case_height > led_matrix_height + case_bottom_thickness + wall_thickness,
       "case_height must accommodate led_matrix_height plus floor and back panel");
assert(m3_screw_head_recess_depth < wall_thickness,
       "m3_screw_head_recess_depth must leave material below; reduce depth or thicken wall");

// ============================================================================
// MAIN ASSEMBLY
// ============================================================================

color("lightgray") {
  if (render_case) {
    led_case();
  }

  if (render_back_panel) {
    translate([
      wall_thickness/2,
      wall_thickness/2,
      case_height - wall_thickness + display_ring_thickness/2
    ]) led_case_back_panel(include_holes = true, gap = 0);
  }

  if (render_display_ring) {
    translate([0, 0, display_ring_thickness/2]) display_ring();
  }
}

// ============================================================================
// CORE MODULES
// ============================================================================

/**
 * Main LED case. Primary housing for the LED matrix display with integrated
 * ventilation, mounting points, and cable management.
 */
module led_case() {
  union() {
    difference() {
      // Outer case shell
      rounded_box(case_length, case_width, case_height, case_corner_radius);

      // Interior cavity
      translate([wall_thickness, wall_thickness, case_bottom_thickness]) {
        cube([case_length - wall_thickness * 2,
              case_width - wall_thickness * 2,
              case_height + wall_thickness]);
      }

      // LED matrix cutout
      translate([(case_length - led_matrix_length)/2,
                 (case_width - led_matrix_width)/2,
                 screen_thickness]) {
        led_matrix_cutout();
      }

      // Back panel mounting recess. Subtract ONLY the plate shape so the GPS
      // bracket and mounting posts (added by led_case_back_panel) do not also
      // carve features into the case body.
      translate([wall_thickness/2,
                 wall_thickness/2,
                 case_height - wall_thickness + eps]) {
        back_panel_plate(include_holes = false, gap = back_panel_fit_tolerance);
      }

      // Ventilation on the y=0 long wall
      create_ventilation_slots(start_x = 20);
      create_ventilation_slots(
        start_x = case_length - (20 + ventilation_hole_spacing * ventilation_hole_count + ventilation_hole_diameter * 2)
      );

      // Optional mirrored vents on the y=case_width wall for cross-flow
      if (ventilation_enable_back_wall) {
        translate([0, case_width, 0]) mirror([0, 1, 0]) {
          create_ventilation_slots(start_x = 20);
          create_ventilation_slots(
            start_x = case_length - (20 + ventilation_hole_spacing * ventilation_hole_count + ventilation_hole_diameter * 2)
          );
        }
      }
    }

    create_mounting_feet();
    create_bottom_feet();
  }
}

/**
 * Back panel assembly: plate plus GPS bracket plus GPS PCB mounting posts.
 * @param include_holes Whether to include screw, GPS, and cable cutouts.
 * @param gap Additional clearance applied to the plate footprint.
 */
module led_case_back_panel(include_holes = true, gap = 0) {
  union() {
    back_panel_plate(include_holes, gap);
    create_gps_mounting_bracket();
    translate([gps_post_origin_x, case_width/4, eps * 2]) {
      rotate([0, 180, 0]) neo_6m_mounting_posts();
    }
  }
}

/**
 * Back panel plate only (rounded box plus optional cutouts). Used standalone
 * for the printed plate and as the recess shape carved into the case body.
 */
module back_panel_plate(include_holes = true, gap = 0) {
  difference() {
    rounded_box(
      case_length - wall_thickness + gap,
      case_width  - wall_thickness + gap,
      wall_thickness,
      case_corner_radius
    );
    if (include_holes) {
      create_m3_mounting_holes();
      create_gps_module_cutout();
      create_cable_pass_throughs();
    }
  }
}

/**
 * Decorative ring around the display area.
 */
module display_ring() {
  difference() {
    rounded_box(case_length, case_width, display_ring_thickness, case_corner_radius);
    translate([display_ring_border/2, display_ring_border/2, -eps]) {
      rounded_box(
        case_length - display_ring_border,
        case_width  - display_ring_border,
        display_ring_thickness + eps * 2,
        2
      );
    }
  }
}

// ============================================================================
// COMPONENT MODULES
// ============================================================================

module led_matrix_cutout() {
  cube([led_matrix_length, led_matrix_width, led_matrix_height]);
}

/**
 * Row of capsule-shaped ventilation slots cutting through the y=0 long wall.
 * @param start_x X position where the row begins.
 */
module create_ventilation_slots(start_x) {
  for (i = [1 : ventilation_hole_count]) {
    translate([start_x, 0, (case_height/2 - ventilation_slot_length/2)]) {
      create_ventilation_slot(i);
    }
  }
}

module create_ventilation_slot(index) {
  hull() {
    translate([(ventilation_hole_spacing * index), 0, 0]) {
      rotate([-90, 0, 0])
        cylinder(d = ventilation_hole_diameter, h = wall_thickness + eps * 2);
    }
    translate([(ventilation_hole_spacing * index), 0, ventilation_slot_length]) {
      rotate([-90, 0, 0])
        cylinder(d = ventilation_hole_diameter, h = wall_thickness + eps * 2);
    }
  }
}

/**
 * Two mounting posts inside the case (back panel screws thread into these).
 */
module create_mounting_feet() {
  foot_height = case_height - case_bottom_thickness - wall_thickness + (eps * 2);

  translate([m3_screw_distance_from_edge, case_width/2, case_bottom_thickness - eps]) {
    mounting_foot(7.5, m3_screw_tap_hole_diameter, foot_height);
  }

  translate([case_length - m3_screw_distance_from_edge, case_width/2, case_bottom_thickness - eps]) {
    mounting_foot(7.5, m3_screw_tap_hole_diameter, foot_height);
  }
}

/**
 * Cable-management bars on the underside of the case.
 */
module create_bottom_feet() {
  footbar_width  = 2.5;
  footbar_height = 1.5;

  // Left bar
  hull() {
    translate([foot_bar_inset_x, -(footbar_height - eps), footbar_width]) {
      cube([footbar_width, footbar_height, (case_height - footbar_width * 2)]);
    }
    translate([foot_bar_inset_x, 0, 0]) {
      cube([footbar_width, eps, eps]);
    }
  }

  // Right bar
  hull() {
    translate([case_length - foot_bar_inset_x, -(footbar_height - eps), footbar_width]) {
      cube([footbar_width, footbar_height, (case_height - footbar_width * 2)]);
    }
    translate([case_length - foot_bar_inset_x, 0, 0]) {
      cube([footbar_width, eps, eps]);
    }
  }
}

/**
 * Shaft clearance hole plus counterbore for the M3 screw head, both ends.
 */
module create_m3_mounting_holes() {
  y_pos = (case_width - wall_thickness) / 2;
  x_positions = [
    m3_screw_distance_from_edge - wall_thickness/2,
    case_length - wall_thickness/2 - m3_screw_distance_from_edge
  ];

  for (x_pos = x_positions) {
    translate([x_pos, y_pos, -eps]) {
      cylinder(d = m3_screw_shaft_clearance_diameter,
               h = wall_thickness + (eps * 2));
    }
    translate([x_pos, y_pos, wall_thickness - m3_screw_head_recess_depth]) {
      cylinder(d = m3_screw_head_recess_diameter,
               h = m3_screw_head_recess_depth + eps * 2);
    }
  }
}

module create_gps_module_cutout() {
  translate([case_length - gps_module_cutout_inset_x,
             (case_width - gps_antenna_size - wall_thickness) / 2,
             -wall_thickness/2]) {
    rounded_box(gps_antenna_size, gps_antenna_size, wall_thickness + (eps * 2), 1.8);
  }
}

module create_cable_pass_throughs() {
  translate([(case_length - wall_thickness + eps) - cable_pass_through_inset_x,
             cable_pass_through_y,
             -eps])
    cylinder(d = cable_pass_through_diameter, h = wall_thickness + eps);
  translate([cable_pass_through_inset_x, cable_pass_through_y, -eps])
    cylinder(d = cable_pass_through_diameter, h = wall_thickness + eps);
}

module create_gps_mounting_bracket() {
  translate([case_length - gps_module_bracket_inset_x,
             (case_width - gps_antenna_size - wall_thickness - 4) / 2,
             -wall_thickness + eps]) {
    difference() {
      rounded_box(gps_antenna_size + 4, gps_antenna_size + 4, 5 + (eps * 2), 1.8);
      translate([2, 2, -eps])
        rounded_box(gps_antenna_size, gps_antenna_size, 5 + (eps * 4), 1.8);
    }
  }
}

/**
 * Neo-6M GPS PCB mounting posts. Hole spacing is gps_post_spacing_x by
 * gps_post_spacing_y (configurable above).
 */
module neo_6m_mounting_posts() {
  for (x = [0, gps_post_spacing_x], y = [0, gps_post_spacing_y]) {
    translate([x, y, 0]) mounting_foot(8, m3_screw_tap_hole_diameter, 3.5);
  }
}

// ============================================================================
// UTILITY MODULES
// ============================================================================

/**
 * Rounded rectangular box. Final extents are [0..length] x [0..width] x [0..height].
 */
module rounded_box(length, width, height, radius) {
  translate([radius, radius, 0]) {
    linear_extrude(height = height) {
      offset(r = radius) {
        square([length - radius * 2, width - radius * 2]);
      }
    }
  }
}

/**
 * Cylindrical mounting post with a flared base for strength.
 * @param diameter      Post diameter.
 * @param hole_diameter Tap-hole diameter through the post.
 * @param height        Total post height.
 * @param fillet_radius Radius of the base flare (default 1.5).
 */
module mounting_foot(diameter, hole_diameter, height, fillet_radius = 1.5) {
  difference() {
    difference() {
      cylinder(d = diameter + fillet_radius, h = height);
      rotate_extrude() {
        translate([(diameter + fillet_radius * 2) / 2, fillet_radius, 0]) {
          minkowski() {
            square(height);
            circle(fillet_radius);
          }
        }
      }
    }
    cylinder(d = hole_diameter, h = height + 1);
  }
}
