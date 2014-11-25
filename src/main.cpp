#include "ofApp.h"
#include "ofAppGlutWindow.h"

int main() {
//	ofAppGlutWindow window;
//	ofSetupOpenGL(&window, 1280, 720, OF_WINDOW);
    ofSetupOpenGL(ofPtr<ofAppBaseWindow>(new ofAppGlutWindow()), 1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}
