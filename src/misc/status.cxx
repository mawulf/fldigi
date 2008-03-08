#include <config.h>

#include <iostream>
#include <fstream>

#include "status.h"
#include "configuration.h"
#include "fl_digi.h"

#include "waterfall.h"

#include "modem.h"
#include "psk.h"
#include "cw.h"
#include "mfsk.h"
#include "rtty.h"
#include "olivia.h"
#include "dominoex.h"
#include "feld.h"
#include "throb.h"
#include "wwv.h"
#include "analysis.h"

#include "rigsupport.h"

extern void startup_modem(modem *m);
extern Fl_Double_Window *dlgViewer;
extern void openViewer();

status progStatus = {
	(int)MODE_BPSK31,	// trx_mode	lastmode;
	0,					// int mainX;
	0,					// int mainY;
	WNOM,				// int mainW;
	HNOM,				// int mainH;
	Hrcvtxt,			// int RxTextHeight;
	false,				// bool rigShown;
	0,					// int rigX;
	0,					// int rigY;
	1000,				// int carrier;
	1,					// int mag;
	NORMAL,				// WFdisp::WFspeed
	0,					// reflevel
	-60,				// ampspan
	40,					// uint	VIEWERnchars
	50,					// uint	VIEWERxpos
	50,					// uint	VIEWERypos
	false,				// bool VIEWERvisible
	false,				// bool LOGenabled
	30.0,				// double sldrSquelchValue
	true,				// bool afconoff
	true,				// bool sqlonoff
	1.0,				// double	RcvMixer;
	1.0,				// double	XmtMixer;
	false				// bool bLastStateRead;
};

	
void status::saveModeState(trx_mode m)
{
	lastmode = (int)m;
}

void status::saveLastState()
{
	mainX = fl_digi_main->x();
	mainY = fl_digi_main->y();
	mainW = fl_digi_main->w();
	mainH = fl_digi_main->h();
	RxTextHeight = ReceiveText->h();
	rigShown = false;
	rigX = 0;
	rigY = 0;
	carrier = wf->Carrier();
	mag = wf->Mag();
	speed = wf->Speed();
	reflevel = progdefaults.wfRefLevel;
	ampspan = progdefaults.wfAmpSpan;

	Fl_Menu_Item *mnulogging = getMenuItem("Log File");
	if (mnulogging)
		LOGenabled = mnulogging->value();
	else
		LOGenabled = false;
	
	if (dlgViewer) {
		if (dlgViewer->visible()) {
			VIEWERxpos = dlgViewer->x();
			VIEWERypos = dlgViewer->y();
			VIEWERvisible = true;
		} else
			VIEWERvisible = false;
	}
	
	if (rigcontrol)
		if (rigcontrol->visible()) {
			rigShown = rigcontrol->visible();
			rigX = rigcontrol->x();
			rigY = rigcontrol->y();
		}
	string deffname = HomeDir;
	deffname.append("fldigi.status");
	ofstream deffile(deffname.c_str(), ios::out);
	deffile << lastmode << endl;
	deffile << mainX << endl;
	deffile << mainY << endl;
	deffile << mainW << endl;
	deffile << mainH << endl;
	deffile << rigShown << endl;
	deffile << rigX << endl;
	deffile << rigY << endl;
	deffile << RxTextHeight << endl;
	deffile << carrier << endl;
	deffile << mag << endl;
	deffile << speed << endl;
	deffile << reflevel << endl;
	deffile << ampspan << endl;
	deffile << VIEWERnchars << endl;
	deffile << VIEWERxpos << endl;
	deffile << VIEWERypos << endl;
	deffile << VIEWERvisible << endl;
	deffile << LOGenabled << endl;
	deffile << sldrSquelchValue << endl;
	deffile << afconoff << endl;
	deffile << sqlonoff << endl;
	deffile << RcvMixer << endl;
	deffile << XmtMixer << endl;
	
	deffile.close();
}

void status::loadLastState()
{
	string deffname = HomeDir;
	deffname.append("fldigi.status");
	ifstream deffile(deffname.c_str(), ios::in);
	if (deffile) {
		deffile >> lastmode;
		deffile >> mainX;
		deffile >> mainY;
		deffile >> mainW;
		deffile >> mainH;
		deffile >> rigShown;
		deffile >> rigX;
		deffile >> rigY;
		deffile >> RxTextHeight;
		deffile >> carrier;
		deffile >> mag;
		deffile >> speed;
		deffile >> reflevel;
		deffile >> ampspan;
		deffile >> VIEWERnchars;
		deffile >> VIEWERxpos;
		deffile >> VIEWERypos;
		deffile >> VIEWERvisible;
		deffile >> LOGenabled;
		deffile >> sldrSquelchValue;
		deffile >> afconoff;
		deffile >> sqlonoff;
		deffile >> RcvMixer;
		deffile >> XmtMixer;
		deffile.close();
		progdefaults.wfRefLevel = reflevel;
		progdefaults.wfAmpSpan = ampspan;
		bLastStateRead = true;
	}
}

void status::initLastState()
{
	if (!bLastStateRead)
		loadLastState();

	init_modem((trx_mode)lastmode);

	while (!active_modem) MilliSleep(100);

 	wf->opmode();
	wf->Mag(mag);
	wf->Speed(speed);
	wf->setRefLevel();
	wf->setAmpSpan();
	wf->movetocenter();
	
	FL_LOCK_D();
	if (useCheckButtons) {
		chk_afconoff->value(afconoff);
		chk_sqlonoff->value(sqlonoff);
	} else {
		btn_afconoff->value(afconoff);
		btn_sqlonoff->value(sqlonoff);
	}
	sldrSquelch->value(sldrSquelchValue);
	valRcvMixer->value(RcvMixer);
	valXmtMixer->value(XmtMixer);
	FL_UNLOCK_D();

	if (rigShown == true) {
		if (!rigcontrol)
			createRigDialog();
		int rdW = rigcontrol->w();
		int rdH = rigcontrol->h();
		rigcontrol->resize(rigX, rigY, rdW, rdH);
		rigcontrol->show();
	}
	if (VIEWERvisible == true)
		openViewer();

	if (LOGenabled) {
		Fl_Menu_Item *mnulogging = getMenuItem("Log File");
		if (!mnulogging)
			return;
		mnulogging->set();
	}		
}
