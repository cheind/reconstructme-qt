/** @file
  * @copyright Copyright (c) 2012 PROFACTOR GmbH. All rights reserved. 
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are
  * met:
  *
  *     * Redistributions of source code must retain the above copyright
  * notice, this list of conditions and the following disclaimer.
  *     * Redistributions in binary form must reproduce the above
  * copyright notice, this list of conditions and the following disclaimer
  * in the documentation and/or other materials provided with the
  * distribution.
  *     * Neither the name of Google Inc. nor the names of its
  * contributors may be used to endorse or promote products derived from
  * this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  * @authors christoph.kopf@profactor.at
  *          florian.eckerstorfer@profactor.at
  */
  
#pragma once

namespace ReconstructMeGUI {

  // settings dialog strings
  const char* const file_changed_tag = "File changed";
  const char* const apply_changes_tag = "Do you want to reload and apply changes of\n";

  // scanner strings
  const char* const something_went_wrong_tag = "An error occured. please review the log output and the application settings.";
  const char* const invalid_license_tag = "Invalid license. Switching to non commercial mode.";
  const char* const license_applied_tag = "Successfully applied license file.";
  const char* const camera_track_found_tag = "Camera Track found.";
  const char* const camera_track_lost_tag = "Global Tracking.";
  const char* const camera_track_lost_license_tag = "Please wait, camera lost track due to the use of a non commercial version.";
  const char* const saved_file_to_tag = "Saved file to ";

  // scanner message box strings
  const char* const warning_tag = "Warning";
  const char* const information_tag = "Information";
  const char* const critical_tag = "Critical";
  const char* const question_tag = "Question";
  const char* const create_sensor_failed_no_context_tag = "Could not create sensor. No context available. Please review if your settings are correct.";
  const char* const run_failed_tag = "Could not start scanner. No sensor or no valid context detected. Please review your settings.";

  // reconstructme strings
  const char* const welcome_tag = "Welcome to ReconstructMe";
  const char* const create_views_tag = "Creating views";
  const char* const reload_settings_tag = "Reload settings";
  const char* const init_scanner_tag = "Intializing scanning environment";
  const char* const init_opencl_tag = "Initializing opencl";
  const char* const init_sensor_tag = "Initializing sensor";
  const char* const no_sensor_found_tag = "No sensor found.";
  const char* const no_sensor_found_msg_tag = "Press OK if you connect a sensor later and set its\nconfiguration file in the settings dialog manually.\nPress Retry if you want to plug in a sensor now and\nlet ReconstrutMe determine a sensor.";
  const char* const saving_to_tag = "Saving to ";
  const char* const mode_pause_tag = "Pause mode";
  const char* const mode_play_tag = "Play mode";
  const char* const volume_resetted_tag = "Volume resetted";
  const char* const loading_settings_tag = "Applying new settings, please wait...";
  const char* const open_url_tag = "Open URL ";
  const char* const application_about_tag = "This is a software for realtime \n3D surface reconstruction.";
  const char* const license_info_tag = "License Information";
  const char* const license_unspecified_tag = "Unspecified error when license was applied.\nSwitching to non commercial mode.";
  const char* const tool_tip_fps_color_label_tag = "Color indicates the quality of the reconstruction experience";
  const char* const tool_tip_fps_label_tag = "Frames per second";
 
  // urls
  const char* const url_install_tag = "http://reconstructme.net/installation/";
  const char* const url_device_matrix_tag = "https://docs.google.com/spreadsheet/ccc?key=0AjYhEvwkxrJOdHBGaTMyWVVBNVFjRHFzbU5RQU81TWc#gid=0" ;
  const char* const url_usage_tag = "http://reconstructme.net/usage/";
  const char* const url_faq_tag = "http://reconstructme.net/faq/";
  const char* const url_forum_tag = "https://groups.google.com/forum/?fromgroups=#!forum/reconstructme";
  const char* const url_sdk_doku_tag = "http://reconstructme.net/wp-content/uploads/ReconstructMe/1.0/doc/html/index.html";
  const char* const url_reconstructme_qt = "http://reconstructme.net/projects/reconstructmeqt/";
}