/** @file
  * @copyright Copyright (c) 2013 PROFACTOR GmbH. All rights reserved. 
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

#include <QApplication>
#include <QFile>
#include <QSplashScreen>

#include "settings.h"
#include "strings.h"
#include "reconstructme.h"
#include "defines.h"

#define SPLASH_MSG_ALIGNMENT Qt::AlignBottom | Qt::AlignLeft

#if _WIN32 && !RECONSTRUCTMEQT_ENABLE_CONSOLE
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) 
{
    int argc = 0;
    char **argv = 0;
#else
  int main(int argc, char *argv[]) 
{
#endif

  using namespace ReconstructMeGUI;

  QApplication app(argc, argv);

  // Splashscreen
  QPixmap splashPix(":/images/splash_screen.png");
  QSplashScreen *sc = new QSplashScreen(splashPix);
  sc->setAutoFillBackground(false);
  sc->showMessage(welcome_tag, SPLASH_MSG_ALIGNMENT);
  sc->show();

  // global style sheet
  QFile style_file(style_sheet_file_tag);
  if(style_file.open(QFile::ReadOnly)) {
    app.setStyle("plastique");
    app.setStyleSheet(style_file.readAll());
  }

  // MainWindow
  reconstructme reme;
  sc->finish(&reme);
  reme.show();

  return app.exec();
}
