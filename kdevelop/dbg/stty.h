/***************************************************************************
                          stty.h  -  description                              
                             -------------------                                         
    begin                : Mon Sep 13 1999                                           
    copyright            : (C) 1999 by John Birch
    email                : jb.nz@writeme.com

  This code was originally written by Judin Maxim, from the
	KDEStudio project.
	
  It was then updated with later code from konsole (KDE).

	It has also been enhanced with an idea from the code in kdbg
	written by Johannes Sixt<Johannes.Sixt@telecom.at>
	
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef STTY_H
#define STTY_H

class QSocketNotifier;

#include <qobject.h>
#include <qstring.h>

class STTY : public QObject
{
  Q_OBJECT

public:
  STTY(bool ext=false, const QString &termAppName=QString());
  ~STTY();

  QString getSlave()    { return ttySlave; };

private slots:
  void OutReceived(int);

signals:
  void OutOutput( const char* );
  void ErrOutput( const char* );

private:
  int findTTY();
  bool findExternalTTY(const QString &termApp);

private:
  int fout;
  int ferr;
  QSocketNotifier* out;
  QSocketNotifier* err;
  QString ttySlave;
  int pid_;

  char pty_master[50];  // "/dev/ptyxx" | "/dev/ptmx"
  char tty_slave[50];   // "/dev/ttyxx" | "/dev/pts/########..."
};

#endif
