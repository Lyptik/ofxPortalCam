/*
 *  ofxPortalCam.cpp
 *  ofxPortalCam
 *
 *  Created by Albert Hwang on 5/25/12.
 *  Copyright 2012 Phedhex. All rights reserved.
 *
 */

#include "ofxPortalCam.h"

ofxPortalCam::ofxPortalCam() {
    
}

ofxPortalCam::~ofxPortalCam() {
    ofRemoveListener(ofEvents().mousePressed, this, &ofxPortalCam::mousePressed);
}

void ofxPortalCam::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    
	calibDone = false;
    tweakDone = false;
	calibStep = 0;
	screenNormal.set(0, 0, 1);
	myOfCamera.setFarClip(20000);
    
    headPos = ofPoint(0,0,0);
    handPos = ofPoint(0,0,0);
    bIsUserTracked = false;
    lastUpdateTimeout = USER_TRACKED_TIMEOUT;
    
    screen = WINDOW;
    setScreen(screen);
    
	loadCalib();
    
    ofAddListener(ofEvents().mousePressed, this, &ofxPortalCam::mousePressed);
}

void ofxPortalCam::loadCalib() {
	if (calibFile.loadFile("calib.xml")) {
		calibFile.loadFile("calib.xml");
		string dxSetting = calibFile.getValue("calib:displaceFactor", "0, 0, 0");
		vector<string> dxSplit = ofSplitString(dxSetting, ", ", false, true);
		displaceFactor.set(ofToFloat(dxSplit[0]), ofToFloat(dxSplit[1]), ofToFloat(dxSplit[2]));

		scaleFactor = ofToFloat(calibFile.getValue("calib:scaleFactor", "0"));

		string r1pSetting = calibFile.getValue("calib:rotation1Perp", "0, 0, 0");
		vector<string> r1pSplit = ofSplitString(r1pSetting, ", ", false, true);
		rotation1Perp.set(ofToFloat(r1pSplit[0]), ofToFloat(r1pSplit[1]), ofToFloat(r1pSplit[2]));

		rotation1 = ofToFloat(calibFile.getValue("calib:rotation1", "0"));

		string r2pSetting = calibFile.getValue("calib:rotation2Perp", "0, 0, 0");
		vector<string> r2pSplit = ofSplitString(r2pSetting, ", ", false, true);
		rotation2Perp.set(ofToFloat(r2pSplit[0]), ofToFloat(r2pSplit[1]), ofToFloat(r2pSplit[2]));
		
		rotation2 = ofToFloat(calibFile.getValue("calib:rotation2", "0"));

		string tpSetting = calibFile.getValue("calib:tweakPerp", "0, 0, 0");
		vector<string> tpSplit = ofSplitString(tpSetting, ", ", false, true);
		tweakPerp.set(ofToFloat(tpSplit[0]), ofToFloat(tpSplit[1]), ofToFloat(tpSplit[2]));

		tweakAngle = ofToFloat(calibFile.getValue("calib:tweakAngle", "0"));

		calibDone = true;
        tweakDone = true;
        
        ofLogVerbose("ofxPortalCam") << "Calibration settings loaded";
	}
}

bool ofxPortalCam::needsCalib() {
	return !(calibDone && tweakDone);
}

void ofxPortalCam::saveCalib() {
	calibFile.loadFile("calib.xml");
	calibFile.setValue("calib:displaceFactor", ofToString(displaceFactor));
	calibFile.setValue("calib:scaleFactor", ofToString(scaleFactor));
	calibFile.setValue("calib:rotation1Perp", ofToString(rotation1Perp));
	calibFile.setValue("calib:rotation1", ofToString(rotation1));
	calibFile.setValue("calib:rotation2Perp", ofToString(rotation2Perp));
	calibFile.setValue("calib:rotation2", ofToString(rotation2));
	calibFile.saveFile("calib.xml");
	ofLogVerbose("ofxPortalCam") << "Calibration settings saved";
}

void ofxPortalCam::resetCalib() {
	calibStep = 0;
	calibDone = false;
    tweakDone = false;
    ofLogVerbose("ofxPortalCam") << "Reset calibration";
}

void ofxPortalCam::drawCalib() {
    updateTimeout();
    
	bool canCalibrate = bIsUserTracked;
	ofPushStyle();
	ofSetColor(255, 255, 255);
	ofFill();
    // Draw calibration instructions and user head/hand
	if(bIsUserTracked){
        ofPushStyle();
        // Draw head and hand
        if(!calibDone){
            ofSetColor(0, 255, 0);
            ofCircle(headPos.x * width, headPos.y * height, headPos.z * 100, 20);
            ofCircle(handPos.x * width, handPos.y * height, handPos.z * 100, 10);
        }
        
        // Draw calibration information
        ofSetColor(56,128,255);
        ofDrawBitmapString("Calibration process : ", width/2 - 190, height - 50);
        ofNoFill();
        ofRect(width/2 + 180 - 190, height - 50 - 12, 200, 16);
        ofFill();
        ofRect(width/2 + 180 - 190, height - 50 - 12, 200 * ((float)calibStep/(float)CALIBRATION_STEPS), 16);
        
        ofSetColor(ofColor::white);
        if(calibStep < 3){
            ofDrawBitmapString( "  Notice that there's a red dot in the top left corner of the \n" \
                               	" screen. Grab your computer mouse and, standing at a distance,\n" \
                               	"extend your right arm forward so that your right hand obscures\n" \
                               	"  your vision of the dot. When you're there, click the mouse \n" \
                                "             button to set that calibration point.           \n" \
                                " Staying at the same position, repeat this operation for the \n" \
                                "                two other following red dots.                ",
                               width/2 - 250, height - 170);
        } else if(calibStep == CALIBRATION_STEPS){
            ofDrawBitmapString( "  To finalize the calibration, you will need to \"tweak\" the  \n" \
                                "  orientation. Put your head in line with the center of the  \n" \
                                "screen. It doesn't matter how far away you are from the screen\n" \
                                "as long as your head is in line with the center.  Once there,\n" \
                                "  click to tweak the orientation and ending the calibration.  ",
                               width/2 - 250, height - 170);
        } else {
            ofDrawBitmapString( " Great! You've just calibrated one set of points. Now we need \n" \
                               	" to calibrate "+ ofToString(CALIBRATION_STEPS/3-1) +" other sets of points ("+ ofToString(CALIBRATION_STEPS/3) +" sets in total). Stand \n" \
                               	" somewhere else in the room and repeat that action for those \n" \
                               	"    three dots. The further you can move your head around,   \n" \
                                "        the stronger your calibration will tend to be.        ",
                               width/2 - 250, height - 170);
        }
        
		ofPopStyle();
    } else {
        ofDrawBitmapString("No user detected. Can't calibrate.", width/2-135, height/2);
    }
    // Draw red circles
	if (canCalibrate && !calibDone) {
		int rad = 30;
		int circleX = 0;
		if ((calibStep % 3) == 1) {
			circleX = width;
		}
		int circleY = height * (calibStep % 3) / 2;
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofCircle(circleX, circleY, rad);
		ofPopStyle();
	}
    // Draw center of the screen
    else if(canCalibrate && calibDone && !tweakDone){
        ofPushStyle();
		ofSetColor(255, 0, 0);
		ofCircle(width/2.0f, height/2.0f, 30);
		ofPopStyle();
    }
	ofPopStyle();
}

void ofxPortalCam::exitCalib(){
    // If calibration not finished, load last settings
    if(needsCalib()){
        loadCalib();
    } else {
        ofLogNotice("ofxPortalCam") << "Calibration ended.";
        return;
    }
    // If cam still need calibration, that means no settings were loaded so calibration must be done
    if(needsCalib()){
        ofLogError("ofxPortalCam") << "Error : there is no correct calibration configuration saved. You must finish this calibration";
    } else {
        ofLogNotice("ofxPortalCam") << "Calibration stopped. Last configuration loaded";
    }
}

ofPoint ofxPortalCam::getCenterPoint(){
    return worldify(ofPoint(0, 0, width * 2.5));
}

void ofxPortalCam::begin() {
	if (bIsUserTracked && headPos != ofVec3f(0,0,0)) {
		ofVec3f screenHead = screenify(headPos);
		myOfCamera.setPosition(screenHead);
	} else {
		myOfCamera.setPosition(0, 0, width * 2.5);
	}

	ofVec3f topLeft, bottomLeft, bottomRight;
	topLeft.set(-width/2, height/2, 0);
	bottomLeft.set(-width/2, -height/2, 0);
	bottomRight.set(width/2, -height/2, 0);
	myOfCamera.setupOffAxisViewPortal(topLeft, bottomLeft, bottomRight);

	myOfCamera.begin();
}

void ofxPortalCam::end() {
	myOfCamera.end();
    updateTimeout();
}

void ofxPortalCam::createCalibRay() {
	if ((!calibDone) && bIsUserTracked && headPos!=ofVec3f(0,0,0) && handPos!=ofVec3f(0,0,0)) {
		ofVec3f headPosition, handPosition;
		headPosition = headPos;
		// for some reason "Left" is from the kinect's perspective, so I changed it to the human perspective
		handPosition = handPos;
		calibRays[calibStep] = ofRay(headPosition, handPosition - headPosition);
		calibStep = calibStep + 1;
		ofLog() << headPosition;
		ofLog() << handPosition;
		if (calibStep == CALIBRATION_STEPS) {
			calcCalib();
		}
	}
}


//--------------------------------------------------------------
void ofxPortalCam::calcCalib(){
	ofVec3f planeDefinition[3];

	for (int i = 0; i < 3; i++) {
		ofVec3f averagePoint;
		int numOfIntersections = CALIBRATION_STEPS / 3; // num of intersections per plane definition point
		ofVec3f* individualPoints = new ofVec3f[numOfIntersections];
		for (int j = 0; j < numOfIntersections; j++) {
			int thisInd = (i +  j    * 3);
			int nextInd = (i + (j+1) * 3) % CALIBRATION_STEPS;
			ofRay intersectionSegment;
			intersectionSegment = calibRays[thisInd].intersect(calibRays[nextInd]);
			ofVec3f intersectionMidpoint;
			intersectionMidpoint.set(intersectionSegment.getMidpoint());
			individualPoints[j] = intersectionMidpoint;
		}
		averagePoint.average(individualPoints, numOfIntersections);
		planeDefinition[i] = averagePoint;
        delete individualPoints;
	}

	// next, i need to find out how to transform kinect space into screen space.

	// find the scalar difference between planeDef[0]-planeDef[2] & window height
	float kinectSpaceWindowHeight = planeDefinition[0].distance(planeDefinition[2]);
	scaleFactor = height / kinectSpaceWindowHeight;
	for (int i = 0; i < 3; i++) {
		planeDefinition[i] = planeDefinition[i] * scaleFactor;
	}

	// find the vector difference between planeDef[0] and (- window width / 2, -window height / 2, 0)
	displaceFactor = planeDefinition[0];

	// find the rotation required to get planeDef[1] & planeDef[2] in place
	ofVec3f rotation1Init = planeDefinition[2] - planeDefinition[0];
	ofVec3f rotation1End;
	rotation1End.set(0, height, 0);

	rotation1Perp = rotation1Init.getPerpendicular(rotation1End);
	rotation1 = rotation1Init.angle(rotation1End);

	ofVec3f	rotation2PreInit, rotation2Init, rotation2InitDebugYSquash, rotation2End, rotation2EndSquash;
	rotation2PreInit = planeDefinition[1] - planeDefinition[0];
	rotation2Init = rotation2PreInit;
	rotation2Init.rotate(rotation1, rotation1Perp);
	rotation2InitDebugYSquash = rotation2Init;
	rotation2InitDebugYSquash.y = 0; // Force this whole thing into a 2d rotation
	rotation2End;
	rotation2End.set(width, height/2, 0);
	rotation2EndSquash = rotation2End;
	rotation2EndSquash.y = 0;
	rotation2Perp = rotation1End;
	rotation2 = rotation2InitDebugYSquash.angle(rotation2EndSquash);
	float rotation2CandidateA = rotation2InitDebugYSquash.angle(rotation2EndSquash);
	float rotation2CandidateB = -rotation2CandidateA;
	ofVec3f rotation2CandidateAPoint = rotation2Init.rotate(rotation2CandidateA, rotation2Perp);
	ofVec3f rotation2CandidateBPoint = rotation2Init.rotate(rotation2CandidateB, rotation2Perp);
	if ((rotation2CandidateAPoint - rotation2End).length() < (rotation2CandidateBPoint - rotation2End).length()) {
		rotation2 = rotation2CandidateA;
	} else {
		rotation2 = rotation2CandidateB;
	}
	calibDone = true;
	saveCalib();
}

//--------------------------------------------------------------
void ofxPortalCam::tweakOrientation(){
	if (calibDone) {
		tweakAngle = 0;
		ofVec3f screenHead = screenify(headPos);
		tweakPerp = screenHead.getPerpendicular(screenNormal);
		tweakAngle = screenHead.angle(screenNormal);
		calibFile.loadFile("calib.xml");
		calibFile.setValue("calib:tweakPerp", ofToString(tweakPerp));
		calibFile.setValue("calib:tweakAngle", ofToString(tweakAngle));
		calibFile.saveFile("calib.xml");
        tweakDone = true;
	}
}

//--------------------------------------------------------------
ofVec3f ofxPortalCam::screenify(ofVec3f kinectPoint){
	ofVec3f newPoint;
	newPoint = kinectPoint;
	newPoint = newPoint * scaleFactor;
	newPoint = newPoint - displaceFactor;
	newPoint.rotate(rotation1, rotation1Perp);
	newPoint.rotate(rotation2, rotation2Perp);
	newPoint.x = newPoint.x - width/2;
	newPoint.x = -newPoint.x;
	newPoint.y = newPoint.y - height/2;
	newPoint.rotate(180, screenNormal);
	newPoint.rotate(tweakAngle, tweakPerp);
	return newPoint;
}

//--------------------------------------------------------------
ofVec3f ofxPortalCam::worldify(ofVec3f screenPoint){
	ofVec3f newPoint;
	newPoint = screenPoint;
	newPoint.rotate(-tweakAngle, tweakPerp);
    newPoint.rotate(-180, screenNormal);
    newPoint.y = newPoint.y + height/2;
    newPoint.x = -newPoint.x;
    newPoint.x = newPoint.x + width/2;
    newPoint.rotate(-rotation2, rotation2Perp);
    newPoint.rotate(-rotation1, rotation1Perp);
    newPoint = newPoint + displaceFactor;
    newPoint = newPoint / scaleFactor;
	return newPoint;
}

//--------------------------------------------------------------
void ofxPortalCam::mousePressed(ofMouseEventArgs& args){
    if(!calibDone){
        createCalibRay();
    } else if(calibDone && !tweakDone){
        tweakOrientation();
    }
}

//--------------------------------------------------------------
void ofxPortalCam::setHeadPosition(ofPoint pos){
    headPos = pos;
    bIsUserTracked = true;
    lastUpdateTimeout = USER_TRACKED_TIMEOUT;
}

//--------------------------------------------------------------
void ofxPortalCam::setHandPosition(ofPoint pos){
    handPos = pos;
    bIsUserTracked = true;
    lastUpdateTimeout = USER_TRACKED_TIMEOUT;
}

//--------------------------------------------------------------
void ofxPortalCam::setScreen(Screen s){
    screen = s;
    
    switch (s) {
        case WINDOW:
            ofAddListener(ofEvents().windowResized, this, &ofxPortalCam::setScreenSizeOnResize);
            width = ofGetWidth();
            height = ofGetHeight();
            ofLogNotice("ofxPortalCam") << "Camera screen set to window resolution";
            break;
            
        case CUSTOM:
            ofRemoveListener(ofEvents().windowResized, this, &ofxPortalCam::setScreenSizeOnResize);
            ofLogNotice("ofxPortalCam") << "Camera screen set to custom resolution";
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofxPortalCam::setScreenSize(int w, int h){
    if(screen != CUSTOM){
        setScreen(CUSTOM);
    }
    width = w;
    height = h;
};

//--------------------------------------------------------------
void ofxPortalCam::setScreenSizeOnResize(ofResizeEventArgs& args){
    width = args.width;
    height = args.height;
};

//--------------------------------------------------------------
void ofxPortalCam::updateTimeout(){
    lastUpdateTimeout--;
    if(lastUpdateTimeout == 0){
        bIsUserTracked = false;
    }
};
