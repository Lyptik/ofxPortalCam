# ofxPortalCam

An OpenFrameworks addon that provides you an anamorphic camera that can be used to transform your monitor (or projected image or whatever) to a viewport through which you can view a digital 3d environment.  This digital 3d environment behaves as if it's actually in front of your face.

This repository is a fork from [the addon by Albert Hwang](https://github.com/Albert/ofxPortalCam).
The following changes were made : 

- Compatibility up to OF 0.8.4
- Removed ofxOpenNI dependency
- Kinect tracking is replaced by OSC, so you can use any tracking device to set your camera position

## Dev env setup

### System

OSX 10.9

### Libs

OpenFrameworks 0.8.4 

#### Addons

Already in OpenFrameworks

- ofxOsc
- ofxXmlSettings

External Addons :

- [ofxRay](https://github.com/elliotwoods/ofxRay)

## Dev env install

### OSX & Linux

Please refer to the external addons section above and use the links to get all the addons.

If you encounter a compilation error with ofxRay, try the following changes : 

- in the file `addons/ofxRay/src/ofxRay/Plane_fitToPoints.cpp`, replace all `UINT` occurrences by `UInt`
- in the file `addons/ofxRay/src/ofxRay/Projector.cpp`, at line 14, replace `os << (ofVec4f&)node.getOrientationQuat();` by `os << node.getOrientationQuat().asVec4();`
