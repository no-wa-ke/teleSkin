#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxGui.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxSyphon.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void dragEvent(ofDragInfo dragInfo);
    void loadFace(string face);
    void crossFade();
    void loadLiveCam();
        void exit();
    
    void keyPressed(int key);
    
    ofxSyphonClient syphonMask;
    ofxSyphonClient syphonInputCam;
    ofxSyphonServer syphonOutput;
    
    ofxFaceTrackerThreaded camTracker;
    ofxFaceTracker srcTracker;
       ofxFaceTracker imgTracker;
    ofVideoGrabber cam;
    
    
//    ofxFaceTrackerThreaded srcTracker;
    ofImage src;
    vector<ofVec2f> srcPoints;
    vector<ofVec2f> inputSrcPoints;
    ofMesh srcMesh;
    
    bool cloneReady,camReady,rotate;
    Clone clone;
    ofFbo srcFbo, maskFbo;
    ofFbo largeFbo;
    ofFbo syphonMaskFbo;
    ofFbo LiveCamFbo;
    
    
    ofImage imageCopy;
    ofImage maskCopy;
    ofImage videoImage;
    ofPoint camSize;
    
    ofFbo  fboSyphonIn;
    ofPixels pix;
    ofPixels maskPix;
    
    
    ofDirectory faces;
    int currentFace;
    int xFade;
    
    ofxPanel gui;
    
    ofxIntSlider    texCoordX;//offset the mask on the mesh
    ofxIntSlider    texCoordY;
    ofxIntSlider fIteration;
    ofxIntSlider fAttempts;
    //ofxIntSlider    xFade;
    ofxFloatSlider    texCoordXScale;
    ofxFloatSlider    texCoordYScale;
    ofxFloatSlider    faceNoise;
    ofxFloatSlider    faceNoiseScale;
    ofxIntSlider    cloneStrength;
    ofxToggle        showMaskSource; //pick if syphon or the list of faces in the folder is a mask source
    ofxToggle        syphonMaskSource;
    
    
    bool drawGui;
    bool screenShot;
    bool fadeUp;
    bool fadeDown;
    int slot1, slot2;
    
    vector <ofImage> maskFile;
    
    ofImage screenshot;
    vector <ofImage> masks;
    
    float genericTime;
    float genericScreenCapTimer;
    ofxXmlSettings XML;
    int lastTagNumber;
    
    ofVideoPlayer movie; //if you want to play a movie on the face
};
