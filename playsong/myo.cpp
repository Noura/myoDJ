// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

#include "myo.h"

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.

DataCollector::DataCollector()
: onArm(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose(), my_pitch(0)
{
}

// onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
void DataCollector::onUnpair(myo::Myo* myo, uint64_t timestamp)
{
    // We've lost a Myo.
    // Let's clean up some leftover state.
    roll_w = 0;
    pitch_w = 0;
    yaw_w = 0;
    my_pitch = 0;
    onArm = false;
}

// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
// as a unit quaternion.
void DataCollector::onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
{
    using std::atan2;
    using std::asin;
    using std::sqrt;
    using std::max;
    using std::min;

    // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
    float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                       1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
    float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
    float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                    1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));

    // Convert the floating point angles in radians to a scale from 0 to 18.
    roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
    pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
    yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
  
    my_pitch = 50 - static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 100);
    my_yaw = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 100) - 50;
    my_roll = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 100) - 50;
}

// onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
// making a fist, or not making a fist anymore.
void DataCollector::onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
{
    currentPose = pose;
    std::string poseString = currentPose.toString();
  
    std::cout << poseString << std::endl;

  if (pose == myo::Pose::fist) {
    system("afplay /Users/noura/myoDJ/media/buzzer_x.wav &");
    
  } else if (pose == myo::Pose::waveOut) {
    system("afplay /Users/noura/myoDJ/media/shotgun_clip.wav &");
  } else if (pose == myo::Pose::waveIn) {
    system("afplay /Users/noura/myoDJ/media/this_is_dj_simon.wav &");
  } else if (pose == myo::Pose::fingersSpread) {
    
  } else if (pose == myo::Pose::thumbToPinky) {
    
  }
    // Vibrate the Myo whenever we've detected that the user has made a fist.
    /* vibrations are annoying
    if (pose == myo::Pose::fist) {
      myo->vibrate(myo::Myo::vibrationMedium);
    }*/
}

// onArmRecognized() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
// arm. This lets Myo know which arm it's on and which way it's facing.
void DataCollector::onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
{
    onArm = true;
    whichArm = arm;
}

// onArmLost() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
// it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
// when Myo is moved around on the arm.
void DataCollector::onArmLost(myo::Myo* myo, uint64_t timestamp)
{
    onArm = false;
}

// There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
// For this example, the functions overridden above are sufficient.

// We define this function to print the current values that were updated by the on...() functions above.
void DataCollector::print()
{
    /*
    // Clear the current line
    std::cout << '\r';

    // Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
    std::cout << '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
              << '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
              << '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']';

    if (onArm) {
        // Print out the currently recognized pose and which arm Myo is being worn on.

        // Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
        // output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
        // that we can fill the rest of the field with spaces below, so we obtain it as a string using toString().
        std::string poseString = currentPose.toString();

        std::cout << '[' << (whichArm == myo::armLeft ? "L" : "R") << ']'
                  << '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
    } else {
        // Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
        std::cout << "[?]" << '[' << std::string(14, ' ') << ']';
    }

    std::cout << std::flush;
     */
  
    std::cout << "P " << my_pitch << " Y " << my_yaw << " R " << my_roll << std::endl;
}

