#include "ofApp.h"

using namespace ofxCv;
ofImage rotated;


void ofApp::setup() {
    
    rotate = false;
    
    //GUI--------------------
    gui.setup ("Panel");
    gui.add ( texCoordX.setup("txCoordX offset", 0, -250, 250));
    gui.add ( texCoordY.setup("txCoordY offset", 0, -250, 250));
    gui.add ( texCoordXScale.setup("txCoordX scale", 1, 0, 4));
    gui.add ( texCoordYScale.setup("txCoordY scale", 1, 0, 4));
    gui.add ( faceNoise.setup("Noise Speed", 0.00, 0, 0.2));
    gui.add ( faceNoiseScale.setup("Noise_scale", 40, 0, 300));
    gui.add ( cloneStrength.setup("Clone Strength", 16, 0, 30));
    gui.add ( showMaskSource.setup("Show Mask Source", false));
    gui.add ( syphonMaskSource.setup("Syphon Mask Source", false));
    
    
#ifdef TARGET_OSX
	//ofSetDataPathRoot(".../data/");
#endif
    ofSetWindowTitle("Syphon Face Substitution");
	ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    //ofSetFrameRate(30);
	cloneReady = false;
    
    //runs a little slower at 720p - you can drop this whole thing down to 640x360 for faster results
    camSize.x = 1280;
    camSize.y = 720;
	
	clone.setup(camSize.x, camSize.y);
	ofFbo::Settings settings;
	settings.width = camSize.x;
	settings.height = camSize.y;
   
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
    
    
//  cam.initGrabber(camSize.x, camSize.y); //Install
    cam.initGrabber(640, 480); //isight 1920,1080 for CCTV. otherwise it wont work.
//  cam.initGrabber(768, 432); // HD cam
//  cam.setDeviceID(2);
    
    largeFbo.allocate(camSize.x,camSize.y);
    
    
//precise configuration
    
    camTracker.setup();
    camTracker.setHaarMinSize(175);
    camTracker.setRescale(.25);
    camTracker.setIterations(3);
    camTracker.setTolerance(2);
    camTracker.setClamp(3);
    camTracker.setAttempts(4);
    
	srcTracker.setup();
	srcTracker.setIterations(3);
	srcTracker.setAttempts(4);
    
    
	//faces.allowExt("jpg");
	//faces.allowExt("png");
    
    //alternate version where we don't use syphon input as a mask texture, but use images instead
	faces.listDir("faces");
    cout<< "Loaded face count: " << faces.size() <<endl;
    for (int i=0; i<faces.size(); i++){
        masks.push_back(faces.getPath(i));
        cout<<i<< " " << faces.getPath(i)<<endl;;
    }
    
	currentFace = 0;
    
    
    
    syphonMaskFbo.begin();
    ofClear(0, 0, 0, 0);
    syphonMaskFbo.end();
    
    srcFbo.begin();
    ofClear(0, 0, 0, 0);
    srcFbo.end();
    
    //FBO
    fboSyphonIn.allocate(camSize.x, camSize.y, GL_RGB);
    syphonMaskFbo.allocate(camSize.x, camSize.y, GL_RGB);
    
    // fboSyphonOut.allocate(camSize.x, camSize.y, GL_RGB);
    
    pix.allocate(camSize.x, camSize.y, 3);
    maskPix.allocate(camSize.x, camSize.y, 3);

    
    genericTime = 0;
    
    xFade = 255;
    
    drawGui = false;
    
    
    //SYPHONNNN setup. make sure to use same name.
    syphonMask.setup();
    syphonMask.setApplicationName("VDMX5");
    syphonMask.setServerName("mask_layer");
    syphonInputCam.setup();
    syphonInputCam.setApplicationName("VDMX5");
    syphonInputCam.setServerName("camera_layer");
    syphonOutput.setName("FaceSubOutput");
    
    src.allocate(800, 800, OF_IMAGE_COLOR);
    
    
//  cam.getTextureReference().allocate(640, 480);
//  loadFace(faces.getPath(currentFace));
    
}
//------------------------------------------------------------
void ofApp::update() {
    
    imageCopy.setFromPixels(pix);
    

    //no mothods to frame sync yet
    camTracker.update(toCv(imageCopy));
    cloneReady = camTracker.getFound();//bool from source
    
    
    if(ofGetFrameNum()%30 == 0){
        
        loadLiveCam();
        
    }
    
    if(cloneReady) {
        
        ofMesh camMesh;
        camMesh = camTracker.getImageMesh();
        camMesh.clearTexCoords();
        for(int i=0; i< srcPoints.size(); i++){
            //add some offsets noise to the texture map
            camMesh.addTexCoord(ofVec2f(texCoordX +
                                        srcPoints[i].x*(texCoordXScale) +
                                        faceNoiseScale*ofSignedNoise((faceNoise*i)*ofGetElapsedTimef()),
                                        texCoordY +
                                        srcPoints[i].y*(texCoordYScale) +
                                        faceNoiseScale*ofSignedNoise((faceNoise*i)*ofGetElapsedTimef()))); //face points from droppedin image
            
            
        }
        
     

        
        maskFbo.begin();
        ofClear(0, 255);
        camMesh.draw();
        maskFbo.end();
        
        srcFbo.begin();
        ofClear(0, 255);
        
//        if(syphonMaskSource){ //bind mask
//            syphonMask.bind();
//            camMesh.draw();
//            syphonMask.unbind();
//           }else{
//        src.bind();
//        camMesh.draw();
//        src.unbind();
//        }
        
         //Live Cam
            cam.getTextureReference().bind();
            camMesh.draw();
            cam.getTextureReference().unbind();

        
        srcFbo.end();
        clone.setStrength(cloneStrength);
        clone.update(srcFbo.getTextureReference(), fboSyphonIn.getTextureReference(), maskFbo.getTextureReference());
    }
    
}
//------------------------------------------------------------
void ofApp::draw() {
    
    ofBackground(0, 0, 0);
	ofSetColor(255);
    ofEnableAlphaBlending();
    
    //Draw the syphon output image to an FBO.
    ofSetColor(255);
    fboSyphonIn.begin();
    ofClear(0,0,0,0);
    syphonInputCam.draw(0,0);
    fboSyphonIn.end();
    
    ofSetColor(255);
    syphonMaskFbo.begin();
    ofClear(0,0,0,0);
    syphonMask.draw(0,0);
    syphonMaskFbo.end();
    
    //Take that FBO reference image (ie the image input) and copy it to some pixels
    //We have to use pixels here because CV operations cannot be performed on textures
    fboSyphonIn.getTextureReference().readToPixels(pix);//these will be read in later in update
    syphonMaskFbo.getTextureReference().readToPixels(maskPix);
    ofSetColor(255);
    
    //Draw everything into FBO so you don't have to deal with scaling other pieces...ofScale would work as well
    largeFbo.begin();
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		syphonInputCam.draw(0, 0);
	}
    
    
    largeFbo.end();
    largeFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    if(srcTracker.getFound()){
        
    cam.draw(0,0,640/3,480/3); // show detected face
    ofDrawBitmapString("Face Found In Front Of the TV", 20,380);
        
    
    }
    
    if(showMaskSource){ //monitor source
        
        syphonMask.draw(0, 0, 640, 360);
        ofDrawBitmapString("Syphon Input", 20,20);
        src.draw(0,360, camSize.x/2, camSize.y/2);
        ofDrawBitmapString("Found Face/Saved Mask", 20,180);
    
    }
    
    
    //Show/hide gui
    if(drawGui){
        //screenshot.draw(200, ofGetHeight()-200, 260,200);
        gui.draw();
        if(!camTracker.getFound()) {
            drawHighlightString("Live Face Not Found", 10, 300);
        }
        if(!srcTracker.getFound()) {
            drawHighlightString("image face not found", 10,325);
        }
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(),2), 20, 360);
        ofDrawBitmapString("Send Camera from VDMX5 to layer named: \"camera_layer\"",20,380);
        ofDrawBitmapString("Send Mask from VDMX5 to layer named: \"mask_layer\"",20,400);
        ofDrawBitmapString("Show/Hide gui with 'd' key",20,420);
        ofDrawBitmapString("Reload mask layer with 'm'",20,440);
    }
    
        syphonOutput.publishTexture(&largeFbo.getTextureReference()); //putting this syphon bit before GUI draw seems to break GUI...
    
}

//------------------------------------------------------------
void ofApp::loadFace(string face){
    
    //NOT THREADED - SLOWS DOWN APP
    
    cout<<"Loading:"<<face<<endl;
    
	src.loadImage(face);
    
    cout<<"Is src allocated: " << src.bAllocated()<<endl;
	if(src.getWidth() > 0 && src.isAllocated()) {
		srcTracker.update(toCv(src)); //convert source image to opencv acceptable format
        if(srcTracker.getFound()){
            srcPoints = srcTracker.getImagePoints(); //load the vector of points that are returned from the tracker as the proper face points
            //srcMesh = srcTracker.getImageMesh();
            cout<<"Face Found"<<endl;
        }
        else{
            srcPoints = inputSrcPoints;
            cout<<"Face NOT Found"<<endl;
        }
	}
}

//------------------------------------------------------------
void ofApp::loadLiveCam(){ // Live cam feed
    
    
    cam.update();

    
     cout<<"Is src allocated: " << cam.getTextureReference().bAllocated()<<endl;
    
    
    if(cam.getWidth() > 0 && cam.isFrameNew()) {
        
        ofPixels& pixels = cam.getPixelsRef(); //buffer pixels
        ofxCv::rotate90(pixels, rotated, rotate ? 270 : 0);
//      ofxCv:flip(rotated, rotated, 1); // suppose to align, but its actually off-setting
        Mat rotatedMat = toCv(rotated);
        
        srcTracker.update(rotatedMat);
        
        if(srcTracker.getFound()) {
                srcPoints = srcTracker.getImagePoints(); // strictly pass down points after detection.
                cout<<"Face Found"<<endl;
            
            rotated.saveImage(ofToString("face")+ofToString(".jpg"));
        }
            else {
                srcPoints = inputSrcPoints;
                cout<<"Face NOT Found"<<endl;
            }
            
        }

    

    
    
}
//------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

	loadFace(dragInfo.files[0]); //grab the first file

}

void ofApp::keyPressed(int key){
	switch(key){
        case OF_KEY_UP:
            currentFace++;
            currentFace = currentFace%faces.size();
            if(faces.size()!=0){
                loadFace(faces.getPath(currentFace));
                
            }
            
            break;
        case OF_KEY_DOWN:
            currentFace--;
            currentFace = currentFace%faces.size();
            if(faces.size()!=0){
                loadFace(faces.getPath(currentFace));
                
            }
            break;
        
        case 'd':
            drawGui = !drawGui;
            break;
        
        case 'f' :
            ofToggleFullscreen();
            break;
            
        case 'm':
            maskCopy.setFromPixels(maskPix);
            src = maskCopy;
            if(src.getWidth() > 0 && src.isAllocated()) {
                srcTracker.update(toCv(src)); //convert source image to opencv acceptable format
                if(srcTracker.getFound()){
                    srcPoints = srcTracker.getImagePoints(); //load the vector of points that are returned from the tracker as the proper face points
                    //srcMesh = srcTracker.getImageMesh();
                    cout<<"Face Found"<<endl;
                }
                else{
                    srcPoints = inputSrcPoints;
                    cout<<"Face NOT Found"<<endl;
                }
            }
            
            break;
   
    }
    
    
}
