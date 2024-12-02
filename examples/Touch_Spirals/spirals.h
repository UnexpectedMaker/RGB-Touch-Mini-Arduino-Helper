/*
  ---------------------------------------------------------------------------
  Spiral Mapper - v1.0 Release - 02/12/2024.

  AUTHOR/LICENSE:
  Created by Seon Rozenblum - seon@unexpectedmaker.com
  Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html

  LINKS:
  Project home: https://rgbtouch.io
  Blog: https://unexpectedmaker.com

  PURPOSE:
  Spiral routines for generation and realtime playback of outward and inward
  spiral paths for the RBG Touch Mini
  ---------------------------------------------------------------------------
*/

#pragma once

#include <vector>
#include "arduino.h"

class SpiralMapper
{
public:
 void generate_spiral(uint8_t x, uint8_t y, int cycle_delay);
 uint16_t get_next_path_step();
 bool has_spiral();
 int current_spiral_index = 0;
 int cycle_update_delay = 100;
 unsigned long last_update_time = 0;

private:
 void reset_visited();
 uint16_t matrix_index(uint8_t x, uint8_t y);
 std::vector<uint16_t> path;
 bool initialized = false;
 bool visited[12][12] = {false}; // Persistent visited array for the matrix
 int cycle_update_delay_backup = 100;
};

// Is there a valid spiral path to follow?
bool SpiralMapper::has_spiral()
{
 return (path.size() > 0 && initialized);
}

// Reset the visited array so we can start a new spiral or wind a spiral back inwards
void SpiralMapper::reset_visited()
{
 for (int i = 0; i < 12; i++)
  for (int j = 0; j < 12; j++)
   visited[i][j] = false;
}

// Generate a spiral path starting at the given touch position
// The spiral is generated outwards from the touch position until it reaches a display boundary
// Then it is generated inwards from the boundary until it reaches the touch position again
// The cycle delay is the time between each step of the spiral in milliseconds when playing it back in realtime
void SpiralMapper::generate_spiral(uint8_t start_x, uint8_t start_y, int cycle_delay)
{
 Serial.printf("Start spiral @ (%d,%d)\n", start_x, start_y);

 // Clear previous path
 path.clear();
 reset_visited();

 // Initialize position and bounds
 int x = start_x, y = start_y;
 int min_x = start_x, max_x = start_x;
 int min_y = start_y, max_y = start_y;

 // Direction vectors (right, down, left, up)
 const int dx[] = {1, 0, -1, 0};
 const int dy[] = {0, 1, 0, -1};
 int direction = 0; // Start moving right

 // Add starting point
 path.push_back(matrix_index(x, y));
 visited[x][y] = true;

 // Outward Spiral
 while (true)
 {
  x += dx[direction];
  y += dy[direction];

  // Check bounds
  if (x < min_x || x > max_x || y < min_y || y > max_y)
  {
   // Expand bounds and ensure corners are handled
   if (direction == 0)
    max_x++;
   else if (direction == 1)
    max_y++;
   else if (direction == 2)
    min_x--;
   else if (direction == 3)
    min_y--;

   // Add the corner that is about to be turned into the path
   if (!visited[x][y] && x >= 0 && x <= 11 && y >= 0 && y <= 11)
   {
    path.push_back(matrix_index(x, y));
    visited[x][y] = true;
    // Serial.printf("Added corner index: %d (x=%d, y=%d)\n", matrix_index(x, y), x, y);
   }

   // Change direction
   direction = (direction + 1) % 4;
   x = std::clamp(x, min_x, max_x);
   y = std::clamp(y, min_y, max_y);

   // Check if bounds exceed matrix size
   if (min_x < 0 || max_x >= 12 || min_y < 0 || max_y >= 12)
    break;

   continue;
  }

  // If already visited, stop outward spiral
  if (visited[x][y])
   break;

  // Mark as visited and add to the path
  path.push_back(matrix_index(x, y));
  visited[x][y] = true;
  // Serial.printf("Added index: %d (x=%d, y=%d)\n", matrix_index(x, y), x, y);
 }

 // Reset visited for inward spiral
 reset_visited();

 min_x = std::clamp(min_x, 0, 11);
 min_y = std::clamp(min_y, 0, 11);
 max_x = std::clamp(max_x, 0, 11);
 max_y = std::clamp(max_y, 0, 11);

 x = std::clamp(x, min_x, max_x);
 y = std::clamp(y, min_y, max_y);

 Serial.printf("Inward spiral starting with (%d,%d) - min_x: %d, min_y: %d, max_x: %d, max_y: %d\n", x, y, min_x, min_y, max_x, max_y);

 // Inward Spiral
 while (min_x <= max_x && min_y <= max_y)
 {
  x += dx[direction];
  y += dy[direction];

  // Check bounds
  if (x < min_x || x > max_x || y < min_y || y > max_y)
  {
   // Shrink bounds
   if (direction == 0)
    min_x++;
   else if (direction == 1)
    min_y++;
   else if (direction == 2)
    max_x--;
   else if (direction == 3)
    max_y--;

   // Add the corner being turned into the path
   if (x >= min_x && x <= max_x && y >= min_y && y <= max_y && x >= 0 && x <= 11 && y >= 0 && y <= 11 && !visited[x][y])
   {
    path.push_back(matrix_index(x, y));
    visited[x][y] = true;
    // Serial.printf("Added corner index: %d (x=%d, y=%d)\n", matrix_index(x, y), x, y);
   }

   // Change direction
   direction = (direction + 1) % 4;
   x = std::clamp(x, min_x, max_x);
   y = std::clamp(y, min_y, max_y);

   continue;
  }

  // Add inward position to the path
  if (!visited[x][y])
  {
   visited[x][y] = true;
   path.push_back(matrix_index(x, y));
   // Serial.printf("Added index: %d (x=%d, y=%d)\n", matrix_index(x, y), x, y);
  }
 }

 // Finalize
 current_spiral_index = 0;
 cycle_update_delay = cycle_delay;
 cycle_update_delay_backup = cycle_delay;
 last_update_time = millis();
 initialized = true;

 Serial.println("Spiral completed.\n");
}

// Calculate the index of a position in the 12x12 matrix
uint16_t SpiralMapper::matrix_index(uint8_t x, uint8_t y)
{
 // Progressive rows: row 0 starts at 0, row 1 starts at 12, row 2 starts at 24, etc.
 return y * 12 + x;
}

// Get the next step in the spiral path, or 0 if there are no more steps
// The cycle delay is the time between each step of the spiral in milliseconds when playing it back in realtime
uint16_t SpiralMapper::get_next_path_step()
{
 if (current_spiral_index == path.size())
 {
  current_spiral_index = 0;
  // If we've made it to the end of the spiral path (back to the start position)
  // then we pause and allow for the spiral to *mostly* finish before going again
  cycle_update_delay = 250;
 }
 else
 {
  // reset the spiral update delay beack to the initial setting
  cycle_update_delay = cycle_update_delay_backup;
 }

 return path[current_spiral_index++];
}

// Create the instance of the SpiralMapper class
SpiralMapper spirals;