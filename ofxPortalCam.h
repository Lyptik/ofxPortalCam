#pragma once
/*
 *  ofxPortalCam.h
 *  ofxPortalCam
 *
 *  Created by Albert Hwang on 5/25/12.
 *  Copyright 2012 Phedhex. All rights reserved.
 *
 */

#include "ofMain.h"
#include "ofxRay.h"
#include "ofxXmlSettings.h"

#define CALIBRATION_STEPS 12    // Must be multiple of 3
#define USER_TRACKED_TIMEOUT 6  // Number of frames before the user is considered not tracked anymore

class ofxPortalCam : public ofCamera {
public:

    enum Screen{
        WINDOW,
        CUSTOM
    };
    
	ofxPortalCam();
    ~ofxPortalCam();
	void begin();
	void end();
    void setup();

	void drawCalib();
	void createCalibRay();

	void loadCalib();
	void saveCalib();
	void resetCalib();
	bool needsCalib();
    void exitCalib();
	// transforms kinect space into screeen space
	ofVec3f screenify(ofVec3f kinectPoint);
    ofVec3f worldify(ofVec3f screenPoint);

	void tweakOrientation();
    
    void setScreen(Screen s);
    void setScreenSize(int w, int h);
    void setHeadPosition(ofPoint pos);
    void setHandPosition(ofPoint pos);
    ofPoint getHeadPosition(){ return headPos; };
    ofPoint getCalibratedHeadPosition(){ return screenify(headPos); };
    
    ofPoint getCenterPoint();

protected:
	void updateUserFromOSC();
    void mousePressed(ofMouseEventArgs& args);
    void setScreenSizeOnResize(ofResizeEventArgs& args);
    
    ofCamera myOfCamera;
    
    ofPoint headPos;
    ofPoint handPos;
    bool bIsUserTracked;
    int lastUpdateTimeout;
    
    Screen screen;
    int width;
    int height;

	bool calibDone;
    bool tweakDone;
	int calibStep;
	void calcCalib();
	ofRay calibRays[CALIBRATION_STEPS];
	ofxXmlSettings calibFile;
	
	ofVec3f displaceFactor;
	float scaleFactor;
	ofVec3f rotation1Perp;
	float rotation1;
	ofVec3f rotation2Perp;
	float rotation2;
	ofVec3f tweakPerp;
	float tweakAngle;

	ofVec3f screenNormal;
};